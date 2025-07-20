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
version="v0.54b"
outname="XTPlayer-${version}_MacOS_Universal"
mkdir ${outname}
cp -R ${appLocation} ${outname}/
hdiutil create /tmp/tmp.dmg -ov -volname ${outname} -fs HFS+ -srcfolder ${outname}
hdiutil convert /tmp/tmp.dmg -format UDZO -o ${outname}.dmg