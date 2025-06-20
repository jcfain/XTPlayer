cd build-release
appLocation="release/XTPlayer.app"
~/Qt/6.9.1/macos/bin/macdeployqt ${appLocation}
cp -R ../src/themes ${appLocation}/Contents/MacOS/
cp -R ../src/images ${appLocation}/Contents/MacOS/
mkDir ${appLocation}/Contents/MacOS/www
cp ../../XTEngine/src/www/*-min.html ${appLocation}/Contents/MacOS/www/
cp ../../XTEngine/src/www/*-min.js ${appLocation}/Contents/MacOS/www/
cp ../../XTEngine/src/www/*-min.css ${appLocation}/Contents/MacOS/www/
cp ../../XTEngine/src/www/favicon.ico ${appLocation}/Contents/MacOS/www/favicon.ico
cp ../../XTEngine/build-release/release/lib* ${appLocation}/Contents/Frameworks/
install_name_tool -change libxtengine.1.0.0.dylib @executable_path/../Frameworks/libxtengine.1.0.0.dylib ${appLocation}/Contents/MacOS/XTPlayer
version="v0.5b"
mkdir XTPlayer-${version}_MacOS_x86_64
cp -R ${appLocation} XTPlayer-${version}_MacOS_x86_64/
hdiutil create /tmp/tmp.dmg -ov -volname "XTPlayer-${version}_MacOS_x86_64" -fs HFS+ -srcfolder "XTPlayer-${version}_MacOS_x86_64"
hdiutil convert /tmp/tmp.dmg -format UDZO -o XTPlayer-${version}_MacOS_x86_64.dmg