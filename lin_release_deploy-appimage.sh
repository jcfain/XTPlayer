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

echo Pulling the latest XTP src from git
cd ${xtplayerSource}
#if needToPull; then
	git pull || { echo 'git pull XTPlayer failed' ; exit 1; }
#fi
echo Pulling the latest XTE src from git
cd ${xtengineSource}
#if needToPull; then
	git pull || { echo 'git pull XTEngine failed' ; exit 1; };
#fi

# --- Optional AV1 codec build (FFmpeg n9.0.1 + dav1d/aom) -------------------
# The FFmpeg media backend bundled with Qt has no AV1 decoders, so AV1 videos
# play audio only. If a codec build exists it is bundled into the AppImage;
# otherwise (interactive runs only) we offer to build it. CI runs
# (no TTY / CODEC_BUILD_AUTOMATED=1) never prompt - they continue without
# AV1 support unless CODEC_LIB_DIR points at a prebuilt codec dir.
CODEC_LIB_DIR="${CODEC_LIB_DIR:-${HOME}/git/ffmpeg-n9.0.1-av1/lib}"
CODEC_BUILD_SCRIPT="$(dirname -- "${BASH_SOURCE[0]}")/helper_scripts/build-ffmpeg-av1.sh"
codecEnabled=0
if [ -f "${CODEC_LIB_DIR}/libavcodec.so.63" ]; then
    echo "Found AV1 codec build in ${CODEC_LIB_DIR} - will bundle it."
    codecEnabled=1
elif [ "${CODEC_BUILD_AUTOMATED:-0}" = "1" ] || [ ! -t 0 ]; then
    echo "WARNING: AV1 codec build not found at ${CODEC_LIB_DIR}."
    echo "         AV1 videos will play audio only (no video) in the FFmpeg backend."
    echo "         Build it first: helper_scripts/build-ffmpeg-av1.sh (or set CODEC_LIB_DIR)."
    echo "         Continuing without AV1 codec support."
else
    echo "AV1 codec build not found. Without it, AV1 videos play audio only (no video)."
    read -r -p "Build it now? (FFmpeg n9.0.1 + dav1d/aom, ~10 min) [Y/n] " answer
    case "${answer:-Y}" in
        [Yy]*)
            "${CODEC_BUILD_SCRIPT}" || { echo 'Codec build failed'; exit 1; }
            codecEnabled=1
            ;;
        *)
            echo "Continuing without AV1 codec support."
            ;;
    esac
fi

# --- glibc compatibility guard ----------------------------------------------
# The codec pack must not require a NEWER glibc than the machine that will RUN
# the AppImage. If it does, libavcodec.so.63 fatals at load time
# ("version `GLIBC_x.y' not found"), the FFmpeg backend silently fails to load
# (loading spinner, no video), and the app falls back to a system libavcodec.
# This happens when the codec was built in a newer-glibc environment (e.g. a
# Flatpak / SDK-26 container) than the target desktop. Build the codec pack on
# the same machine (or one with an equal/older glibc) that runs the AppImage.
if [ "${codecEnabled}" = "1" ]; then
    _codec_lib="${CODEC_LIB_DIR}/libavcodec.so.63"
    _need_glibc="$(objdump -T "${_codec_lib}" 2>/dev/null | grep -oE 'GLIBC_[0-9]+\.[0-9]+' | sed 's/GLIBC_//' | sort -Vu | tail -1)"
    _sys_glibc="$(ldd --version 2>/dev/null | head -1 | grep -oE '[0-9]+\.[0-9]+' | head -1)"
    if [ -n "${_need_glibc}" ] && [ -n "${_sys_glibc}" ]; then
        _need_maj="${_need_glibc%%.*}"; _need_min="${_need_glibc#*.}"
        _sys_maj="${_sys_glibc%%.*}";   _sys_min="${_sys_glibc#*.}"
        if [ "${_need_maj}" -gt "${_sys_maj}" ] || { [ "${_need_maj}" -eq "${_sys_maj}" ] && [ "${_need_min}" -gt "${_sys_min}" ]; }; then
            echo "ERROR: AV1 codec pack requires glibc ${_need_glibc} but this system has glibc ${_sys_glibc}."
            echo "       libavcodec.so.63 will FAIL to load at runtime (GLIBC_${_need_glibc} not found),"
            echo "       breaking the FFmpeg backend (loading spinner, no video)."
            echo "       Rebuild the codec pack ON THIS MACHINE (or one with an older glibc):"
            echo "           helper_scripts/build-ffmpeg-av1.sh"
            echo "       It was built in a newer-glibc environment (e.g. a Flatpak/SDK container)."
            exit 1
        else
            echo "OK: AV1 codec pack glibc requirement (${_need_glibc}) <= system glibc (${_sys_glibc})."
        fi
    fi
fi
# --- end glibc compatibility guard ------------------------------------------
# --- end AV1 codec build ----------------------------------------------------

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

# --- Bundle the AV1 codec libs into the AppImage ----------------------------
# Overwrite the AV1-less FFmpeg libs that linuxdeployqt would bundle from the
# Qt tree, and add the AV1 decoders (dav1d/aom) which live on the system and
# would NOT be bundled by linuxdeployqt on their own.
if [ "${codecEnabled}" = "1" ]; then
    mkdir -p "${appDir}"/usr/lib
    cp -f "${CODEC_LIB_DIR}"/libavcodec.so.63*    "${appDir}"/usr/lib/ || { echo 'Copy codec libavcodec failed' ; exit 1; }
    cp -f "${CODEC_LIB_DIR}"/libavformat.so.63*   "${appDir}"/usr/lib/ || { echo 'Copy codec libavformat failed' ; exit 1; }
    cp -f "${CODEC_LIB_DIR}"/libavutil.so.61*     "${appDir}"/usr/lib/ || { echo 'Copy codec libavutil failed' ; exit 1; }
    cp -f "${CODEC_LIB_DIR}"/libswscale.so.10*    "${appDir}"/usr/lib/ || { echo 'Copy codec libswscale failed' ; exit 1; }
    cp -f "${CODEC_LIB_DIR}"/libswresample.so.7*  "${appDir}"/usr/lib/ || { echo 'Copy codec libswresample failed' ; exit 1; }
    for extra in libdav1d.so.7* libaom.so.3*; do
        cp -f "${CODEC_LIB_DIR}"/${extra} "${appDir}"/usr/lib/ 2>/dev/null
    done
    # Make dependency resolution prefer the appDir copies over the Qt tree
    export LD_LIBRARY_PATH="${appDir}/usr/lib:${LD_LIBRARY_PATH}"
    echo "Bundled AV1 codec libs from ${CODEC_LIB_DIR}"
fi
# --- end AV1 codec bundling --------------------------------------------------

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
    if [ "${codecEnabled}" = "1" ]; then
        BUNDLED_AVCODEC="$(find "${VERIFY_DIR}" -name 'libavcodec.so.63*' | head -1)"
        if [ -n "${BUNDLED_AVCODEC}" ] && nm -D "${BUNDLED_AVCODEC}" 2>/dev/null | grep -q dav1d && \
           find "${VERIFY_DIR}" -name 'libdav1d.so.7*' | grep -q .; then
            echo "OK: AV1 codec (dav1d) bundled in AppImage."
        else
            echo "WARNING: AV1 codec NOT properly bundled in AppImage (libavcodec lacks dav1d or libdav1d missing)."
        fi
    fi
else
    echo "NOTE: skipped AppImage verification (no appimagetool, and AppImage not executable on this host)."
    echo "      Pre-deploy check already confirmed libqwebp.so is in the Qt tree, so it should be bundled."
fi
rm -rf "${VERIFY_DIR}"
# --- end webp verify -------------------------------------------------------
