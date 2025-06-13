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

export EXTRA_PLATFORM_PLUGINS=libqwayland-generic.so
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
export PATH="${qtDirectory}":$PATH
export LD_LIBRARY_PATH="${deployDirectory}lib;${xtengineLocation}"

echo LD_LIBRARY_PATH: ${LD_LIBRARY_PATH}
echo PATH: ${PATH}
echo Home: ${home}
echo Target: ${target}
echo appDir: ${appDir}

cd ${xtplayerSource}
if needToPull; then
	git pull || { echo 'git pull XTPlayer failed' ; exit 1; }
fi
cd ${xtengineSource}
if needToPull; then
	git pull || { echo 'git pull XTEngine failed' ; exit 1; };
fi

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
cp -r "${xtplayerLocation}"www/*-min.*  "${appDir}"/usr/bin/www/ || { echo 'Copy www failed' ; exit 1; }
cp -r "${xtplayerLocation}"themes/*.*  "${appDir}"/usr/bin/themes/ || { echo 'Copy themes failed' ; exit 1; }
cp "${xtplayerSource}"/XTPlayer.desktop "${xtplayerLocation}"/XTPlayer.desktop || { echo 'Copy desktop file failed' ; exit 1; }

cp "${xtplayerSource}"/src/images/icons/XTP-icon.png "${xtplayerLocation}"/XTPlayer.png || { echo 'Copy icon failed' ; exit 1; }

#export GSTREAMER_INCLUDE_BAD_PLUGINS="1"
#--plugin gstreamer 
"${linuxdeployBinary}" --appdir "${appDir}" --executable "${xtplayerLocation}"/XTPlayer --plugin qt --output appimage --icon-file "${xtplayerLocation}"/XTPlayer.png --desktop-file "${xtplayerLocation}"/XTPlayer.desktop || { echo 'Deploy failed' ; exit 1; }

mv "${xtplayerSource}"/XTPlayer-"${architecture}".AppImage "${deployDirectory}"XTPlayer-"${version}"-Linux-"${architecture}".AppImage
cp "${deployDirectory}"XTPlayer-"${version}"-Linux-"${architecture}".AppImage "${target}"

cp "${deployDirectory}"XTPlayer-"${version}"-Linux-"${architecture}".AppImage "${deployDirectory}"XTPlayer-Latest.AppImage
