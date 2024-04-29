cd build-XTPlayer-Desktop_Qt_5_15_2_clang_64bit-Release
~/Qt/5.15.2/clang_64/bin/macdeployqt XTPlayer.app
cp -R ../src/themes XTPlayer.app/Contents/MacOS/
cp -R ../src/images XTPlayer.app/Contents/MacOS/
mkDir XTPlayer.app/Contents/MacOS/www
cp ../../XTEngine/src/www/*-min.html XTPlayer.app/Contents/MacOS/www/
cp ../../XTEngine/src/www/*-min.js XTPlayer.app/Contents/MacOS/www/
cp ../../XTEngine/src/www/*-min.css XTPlayer.app/Contents/MacOS/www/
cp ../../XTEngine/src/www/favicon.ico XTPlayer.app/Contents/MacOS/www/favicon.ico
cp ../../HttpServer/src/build/release/libhttpServer.1.0.0.dylib XTPlayer.app/Contents/Frameworks/
cp ../../HttpServer/src/build/release/libhttpServer.1.0.dylib XTPlayer.app/Contents/Frameworks/
cp ../../HttpServer/src/build/release/libhttpServer.1.dylib XTPlayer.app/Contents/Frameworks/
cp ../../HttpServer/src/build/release/libhttpServer.dylib XTPlayer.app/Contents/Frameworks/
cp ../../XTEngine/build-XTEngine-Desktop_Qt_5_15_2_clang_64bit-Release/release/lib* XTPlayer.app/Contents/Frameworks/
install_name_tool -change libhttpServer.1.dylib @executable_path/../Frameworks/libhttpServer.1.dylib XTPlayer.app/Contents/MacOS/XTPlayer
install_name_tool -change libhttpServer.1.dylib @executable_path/../Frameworks/libhttpServer.1.dylib XTPlayer.app/Contents/Frameworks/libxtengine.1.0.0.dylib
install_name_tool -change libxtengine.1.0.0.dylib @executable_path/../Frameworks/libxtengine.1.0.0.dylib XTPlayer.app/Contents/MacOS/XTPlayer
version="v0.45b"
mkdir XTPlayer-${version}_MacOS_x86_64
cp -R XTPlayer.app XTPlayer-${version}_MacOS_x86_64/
hdiutil create /tmp/tmp.dmg -ov -volname "XTPlayer-${version}_MacOS_x86_64" -fs HFS+ -srcfolder "XTPlayer-${version}_MacOS_x86_64"
hdiutil convert /tmp/tmp.dmg -format UDZO -o XTPlayer-${version}_MacOS_x86_64.dmg