#!/bin/bash

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

rm -rf "${appDir}"

mkdir -p "${appDir}"
mkdir -p "${appDir}"/usr/bin/www
mkdir -p "${appDir}"/usr/bin/themes
cp -r "${xtplayerLocation}"www/*-min.*  "${appDir}"/usr/bin/www/
cp -r "${xtplayerLocation}"themes/*.*  "${appDir}"/usr/bin/themes/
cp "${xtplayerSource}"/XTPlayer.desktop "${xtplayerLocation}"/XTPlayer.desktop

cp "${xtplayerSource}"/src/images/icons/XTP-icon.png "${xtplayerLocation}"/XTPlayer.png

#export GSTREAMER_INCLUDE_BAD_PLUGINS="1"
#--plugin gstreamer 
"${linuxdeployBinary}" --appdir "${appDir}" --executable "${xtplayerLocation}"/XTPlayer --plugin qt --output appimage --icon-file "${xtplayerLocation}"/XTPlayer.png --desktop-file "${xtplayerLocation}"/XTPlayer.desktop

mv "${xtplayerSource}"/XTPlayer-"${architecture}".AppImage "${deployDirectory}"XTPlayer-"${version}"-Linux-"${architecture}".AppImage
cp "${deployDirectory}"XTPlayer-"${version}"-Linux-"${architecture}".AppImage "${target}"

cp "${deployDirectory}"XTPlayer-"${version}"-Linux-"${architecture}".AppImage "${deployDirectory}"XTPlayer-Latest.AppImage
