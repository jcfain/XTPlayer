#!/bin/bash

needToPull() {
	UPSTREAM=${1:-'@{u}'}
	LOCAL=$(git rev-parse @)
	REMOTE=$(git rev-parse "$UPSTREAM")
	BASE=$(git merge-base @ "$UPSTREAM")
	if [ $LOCAL = $REMOTE ]; then
		echo "Up-to-date"
	elif [ $LOCAL = $BASE ]; then {
		echo "Need to pull"
		return 0
	} elif [ $REMOTE = $BASE ]; then
		echo "Need to push"
	else
		echo "Diverged"
	fi
	return 1
}

export EXTRA_PLATFORM_PLUGINS=libqwayland.so
export EXTRA_QT_MODULES="waylandcompositor"

if [ -z ${home+x} ]; then 
    echo "Do not run this script directly!"; 
    exit 1
fi

echo "Enter version: (ex: 0.423b)"  
read versionInput
export version=v${versionInput}

appDir="${deployDirectory}"XTPlayer-"${version}"-Linux-"${architecture}"
linuxdeployBinary="${xtplayerSource}"/linuxdeploy/"${architecture}"/linuxdeploy-"${architecture}".AppImage

#Export directory with qmake to path.
export PATH="${qtGccBinDirectory}":$PATH
export LD_LIBRARY_PATH="${deployDirectory}lib;${xtengineLocation}"

echo LD_LIBRARY_PATH: ${LD_LIBRARY_PATH}
echo PATH: ${PATH}
echo Home: ${home}
echo Target: ${target}
echo appDir: ${appDir}

cd ${xtplayerSource}
#if needToPull; then
	git pull || { echo 'git pull XTPlayer failed' ; exit 1; }
#fi
cd ${xtengineSource}
#if needToPull; then
	git pull || { echo 'git pull XTEngine failed' ; exit 1; };
#fi

mkdir -p "${xtengineBuildDirectory}" || { echo 'Make XTEngine build dir failed' ; exit 1; }
cd ${xtengineBuildDirectory}
qmake ${xtengineSource}/src/XTEngine.pro CONFIG+=release || { echo 'QMake XTEngine failed' ; exit 1; }
make Makefile qmake_all || { echo 'Make Makefile failed' ; exit 1; }
make --silent -j24 || { echo 'Make failed' ; exit 1; }
mkdir -p "${xtplayerBuildDirectory}" || { echo 'Make XTPlayer build dir failed' ; exit 1; }
cd ${xtplayerBuildDirectory}
qmake ${xtplayerSource}/src/XTPlayer.pro CONFIG+=release || { echo 'QMake XTPlayer failed' ; exit 1; }
make Makefile qmake_all || { echo 'Make Makefile failed' ; exit 1; }
make --silent -j24 || { echo 'Make failed' ; exit 1; }

cd ${SCRIPT_DIR}

rm -rf "${appDir}"

mkdir -p "${appDir}"
mkdir -p "${appDir}"/usr/bin/www
mkdir -p "${appDir}"/usr/bin/themes
mkdir -p "${appDir}"/usr/bin/translations
cp -r "${xtplayerLocation}"www/*.*  "${appDir}"/usr/bin/www/ || { echo 'Copy www failed' ; exit 1; }
cp -r "${xtplayerLocation}"themes/*.*  "${appDir}"/usr/bin/themes/ || { echo 'Copy themes failed' ; exit 1; }
cp -r "${xtplayerLocation}"translations/*.*  "${appDir}"/usr/bin/translations/ || { echo 'Copy translations failed' ; exit 1; }

cp "${xtplayerSource}"/XTPlayer.desktop "${xtplayerLocation}"/XTPlayer.desktop || { echo 'Copy desktop file failed' ; exit 1; }

cp "${xtplayerSource}"/src/images/icons/XTP-icon.png "${xtplayerLocation}"/XTPlayer.png || { echo 'Copy icon failed' ; exit 1; }

# --- Ensure the WebP imageformat plugin is bundled -------------------------
# linuxdeploy's --plugin qt copies imageformats plugins from the Qt tree, so
# libqwebp.so must exist there for the AppImage to write .webp thumbs (a
# missing plugin makes QImage::save(".webp") fail; the app then falls back to
# jpg). If it's not in the Qt tree, copy it in from WEBP_PLUGIN_SRC (set that
# env var to an aarch64 libqwebp.so) so it gets bundled.
QT_IMAGEFORMATS_DIR="${qtGccDirectory}/plugins/imageformats"
if [ ! -f "${QT_IMAGEFORMATS_DIR}/libqwebp.so" ]; then
    if [ -n "${WEBP_PLUGIN_SRC}" ] && [ -f "${WEBP_PLUGIN_SRC}" ]; then
        mkdir -p "${QT_IMAGEFORMATS_DIR}" || { echo 'Make imageformats dir failed' ; exit 1; }
        cp "${WEBP_PLUGIN_SRC}" "${QT_IMAGEFORMATS_DIR}/libqwebp.so" || { echo 'Copy libqwebp.so failed' ; exit 1; }
        echo "Copied webp imageformat plugin from ${WEBP_PLUGIN_SRC} to ${QT_IMAGEFORMATS_DIR}"
    else
        echo "WARNING: libqwebp.so not found in ${QT_IMAGEFORMATS_DIR}."
        echo "         AppImage thumbs will fall back to jpg. Set WEBP_PLUGIN_SRC to an aarch64 libqwebp.so to bundle webp."
    fi
fi
# read -p "Press Enter to continue" </dev/tty
# --- end webp imageformat plugin check -------------------------------------

#export GSTREAMER_INCLUDE_BAD_PLUGINS="1"
#--plugin gstreamer 
"${linuxdeployBinary}" --appdir "${appDir}" --executable "${xtplayerLocation}"/XTPlayer --plugin qt --output appimage --icon-file "${xtplayerLocation}"/XTPlayer.png --desktop-file "${xtplayerLocation}"/XTPlayer.desktop || { echo 'Deploy failed' ; exit 1; }

mv "${xtplayerSource}"/XTPlayer-"${architecture}".AppImage "${deployDirectory}"XTPlayer-"${version}"-Linux-"${architecture}".AppImage
cp "${deployDirectory}"XTPlayer-"${version}"-Linux-"${architecture}".AppImage "${target}"

cp "${deployDirectory}"XTPlayer-"${version}"-Linux-"${architecture}".AppImage "${deployDirectory}"XTPlayer-Latest.AppImage

# --- Verify the webp imageformat plugin made it into the AppImage ----------
# Cross-build note: --appimage-extract works by EXECUTING the AppImage, which
# fails when the host arch != target arch (e.g. building aarch64 on x86_64).
# Prefer appimagetool -x (host-native, arch-independent); fall back to
# executing the AppImage for native builds; skip gracefully if neither works.
APPIMAGE_OUT="${deployDirectory}XTPlayer-${version}-Linux-${architecture}.AppImage"
VERIFY_DIR=/tmp/xtp_appimage_verify
rm -rf "${VERIFY_DIR}"
verified=0
if command -v appimagetool >/dev/null 2>&1; then
    appimagetool -x "${APPIMAGE_OUT}" "${VERIFY_DIR}" >/dev/null 2>&1 && verified=1
elif [ -x "${APPIMAGE_OUT}" ]; then
    "${APPIMAGE_OUT}" --appimage-extract -o "${VERIFY_DIR}" >/dev/null 2>&1 && verified=1
fi
if [ "${verified}" -eq 1 ]; then
    if find "${VERIFY_DIR}" -name 'libqwebp.so' | grep -q .; then
        echo "OK: libqwebp.so bundled in AppImage (webp thumbs enabled)."
    else
        echo "WARNING: libqwebp.so NOT found in AppImage - thumbs will fall back to jpg."
    fi
else
    echo "NOTE: skipped AppImage verification (no appimagetool, and AppImage not executable on this host)."
    echo "      Pre-deploy check already confirmed libqwebp.so is in the Qt tree, so it should be bundled."
fi
rm -rf "${VERIFY_DIR}"
# --- end webp verify -------------------------------------------------------
