#!/bin/bash

echo "Enter version: (ex: 0.423b)"  
read versionInput
export VERSION=v${versionInput}

home=~/
target='/media/NAS/STK/Hardware/my software/'
xtplayerSource="${home}"git/XTPlayer
xtplayerLocation="${home}"git/XTPlayer/build-XTPlayer-Desktop_Qt_5_15_2_GCC_64bit-Release/
xtengineLocation="${home}"git/XTEngine/build-XTEngine-Desktop-Release/release/
httpserverLocation="${home}"git/HttpServer/src/build/release/
deployDirectory="${home}"git/XTPlayer/Deploy/
appDir=""${deployDirectory}"XTPlayer-"${VERSION}"-Linux-x86_64"
linuxdeployBinary="${home}git/linuxdeploy/linuxdeploy-x86_64.AppImage"
qtDirectory="${home}"Qt/5.15.2/gcc_64/bin
linuxdeployPlugins="${home}git/linuxdeploy-plugin"

#Export directory with qmake to path.
export PATH="${linuxdeployPlugins}:${qtDirectory}":$PATH
export LD_LIBRARY_PATH="${deployDirectory}lib"

#echo LD_LIBRARY_PATH: ${LD_LIBRARY_PATH}
#echo PATH: ${PATH}
#echo Home: ${home}
#echo Target: ${target}

mkdir -p "${appDir}"
mkdir -p "${deployDirectory}lib"
find "${xtengineLocation}" \( -name "moc_*" -or -name "*.o" -or -name "qrc_*" -or -name "Makefile*" -or -name "*.a" \) -exec rm {} \;
cp -r "${xtengineLocation}"* "${deployDirectory}lib"
cp -r "${httpserverLocation}"* "${deployDirectory}lib"
cp -rf /usr/lib/x86_64-linux-gnu/gstreamer-1.0 "${deployDirectory}lib"
find "${xtplayerLocation}" \( -name "moc_*" -or -name "*.o" -or -name "qrc_*" -or -name "Makefile*" -or -name "*.a" \) -exec rm {} \;
cp "${xtplayerSource}"/XTPlayer.desktop "${xtplayerLocation}"/XTPlayer.desktop

cp "${xtplayerSource}"/src/images/icons/XTP-icon.png ${xtplayerLocation}/XTPlayer.png

GSTREAMER_INCLUDE_BAD_PLUGINS="1"
"${linuxdeployBinary}" --appdir ${appDir} --executable ${xtplayerLocation}/XTPlayer --plugin qt --plugin gstreamer --output appimage --icon-file ${xtplayerLocation}/XTPlayer.png --desktop-file "${xtplayerLocation}"/XTPlayer.desktop

#"${linuxdeployBinary}" ${xtplayerLocation}/XTPlayer -appimage -always-overwrite -extra-plugins=iconengines,platformthemes/libqgtk3.so,mediaservice,gamepads

#,libgstreamer1.0-0,libgstreamer-plugins-base1.0-0,libgstreamer-plugins-bad1.0-0,gstreamer1.0-x,gstreamer1.0-alsa,gstreamer1.0-plugins-base,gstreamer1.0-plugins-good,gstreamer1.0-plugins-bad,gstreamer1.0-plugins-ugly,gstreamer1.0-pulseaudio,gstreamer1.0-libav
#,platformthemes/libqgtk3.so
#mv ${home}git/XTPlayer.appimage  ${home}git/XTPlayer-v$(version).appimage
#-unsupported-allow-new-glibc 
mv "${xtplayerSource}"/XTPlayer-"${VERSION}"-x86_64.AppImage "${deployDirectory}"XTPlayer-"${VERSION}"-Linux-x86_64.AppImage
cp "${deployDirectory}"XTPlayer-"${VERSION}"-Linux-x86_64.AppImage "${target}"XTPlayer-"${VERSION}"-Linux-x86_64.AppImage
