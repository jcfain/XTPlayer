echo "Enter version: (ex: 0.423b)"  
read versionInput
VERSION=v${versionInput}
export VERSION

mkdir -p ~/git/XTPlayer/Deploy/lib 
cp -r ~/git/XTEngine/build-XTEngine-Desktop_Qt_5_15_2_GCC_64bit-Release/release/* ~/git/XTPlayer/Deploy/lib
cp -r ~/git/HttpServer/src/build/release/* ~/git/XTPlayer/Deploy/lib
#RPATH=$(pwd)/XTPlayer/Deploy/lib:${RPATH}
#export RPATH
LD_LIBRARY_PATH=$(pwd)/Deploy/lib
export LD_LIBRARY_PATH
echo $LD_LIBRARY_PATH
find ~/git/XTPlayer/build-XTPlayer-Desktop-Release/XTPlayer \( -name "moc_*" -or -name "*.o" -or -name "qrc_*" -or -name "Makefile*" -or -name "*.a" \) -exec rm {} \;
cp ~/git/XTPlayer/XTPlayer.desktop ~/git/XTPlayer/build-XTPlayer-Desktop-Release/XTPlayer.desktop

cp ~/git/XTPlayer/src/images/icons/XTP-icon.png ~/git/XTPlayer/build-XTPlayer-Desktop-Release/XTPlayer.png
~/git/linuxdeployqt/bin/linuxdeployqt ~/git/XTPlayer/build-XTPlayer-Desktop-Release/XTPlayer -appimage -unsupported-allow-new-glibc -always-overwrite -extra-plugins=iconengines
#,platformthemes/libqgtk3.so
#mv ~/git/XTPlayer.appimage  ~/git/XTPlayer-v$(version).appimage
cp ~/git/XTPlayer/XTPlayer-${VERSION}-aarch64.AppImage /media/NAS/STK/Hardware/my\ software/XTPlayer-${VERSION}-aarch64.AppImage
