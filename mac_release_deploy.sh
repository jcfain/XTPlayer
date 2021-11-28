cd build-XTPlayer-Desktop_Qt_5_15_2_clang_64bit-Release
~/Qt/5.15.2/clang_64/bin/macdeployqt XTPlayer.app
cp -R ../src/themes XTPlayer.app/Contents/MacOS/
cp -R ../src/images XTPlayer.app/Contents/MacOS/
mkDir XTPlayer.app/Contents/MacOS/www
cp ../src/www/index-min.html XTPlayer.app/Contents/MacOS/www/index.html
cp ../src/www/index-min.js XTPlayer.app/Contents/MacOS/www/index.js
cp ../src/www/styles-min.css XTPlayer.app/Contents/MacOS/www/styles.css
cp ../src/www/favicon.ico XTPlayer.app/Contents/MacOS/www/favicon.ico
cp ../../HttpServer/src/build/release/libhttpServer.1.0.0.dylib XTPlayer.app/Contents/Frameworks/
cp ../../HttpServer/src/build/release/libhttpServer.1.0.dylib XTPlayer.app/Contents/Frameworks/
cp ../../HttpServer/src/build/release/libhttpServer.1.dylib XTPlayer.app/Contents/Frameworks/
cp ../../HttpServer/src/build/release/libhttpServer.dylib XTPlayer.app/Contents/Frameworks/
cp ~/Qt/5.15.2/clang_64/lib/libav* XTPlayer.app/Contents/Frameworks/
cp ~/Qt/5.15.2/clang_64/lib/libswresample.3.dylib XTPlayer.app/Contents/Frameworks/
cp ~/Qt/5.15.2/clang_64/lib/libswscale.5.dylib XTPlayer.app/Contents/Frameworks/
install_name_tool -change libhttpServer.1.dylib @executable_path/../Frameworks/libhttpServer.1.dylib XTPlayer.app/Contents/MacOS/XTPlayer
install_name_tool -change ~/Qt/5.15.2/clang_64/lib/QtAV.framework/Versions/1/QtAV @executable_path/../Frameworks/QtAV.framework/Versions/1/QtAV XTPlayer.app/Contents/MacOS/XTPlayer
install_name_tool -change ~/Qt/5.15.2/clang_64/lib/QtAVWidgets.framework/Versions/1/QtAVWidgets @executable_path/../Frameworks/QtAVWidgets.framework/Versions/1/QtAVWidgets XTPlayer.app/Contents/MacOS/XTPlayer
install_name_tool -change ~/Qt/5.15.2/clang_64/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui XTPlayer.app/Contents/Frameworks/QtAV.framework/Versions/1/QtAV 
install_name_tool -change ~/Qt/5.15.2/clang_64/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore XTPlayer.app/Contents/Frameworks/QtAV.framework/Versions/1/QtAV 
install_name_tool -change ~/Qt/5.15.2/clang_64/lib/QtAV.framework/Versions/1/QtAV  @executable_path/../Frameworks/QtAV.framework/Versions/1/QtAV XTPlayer.app/Contents/Frameworks/QtAVWidgets.framework/Versions/1/QtAVWidgets
install_name_tool -change ~/Qt/5.15.2/clang_64/lib/QtOpenGL.framework/Versions/5/QtOpenGL @executable_path/../Frameworks/QtOpenGL.framework/Versions/5/QtOpenGL XTPlayer.app/Contents/Frameworks/QtAVWidgets.framework/Versions/1/QtAVWidgets
install_name_tool -change ~/Qt/5.15.2/clang_64/lib/QtWidgets.framework/Versions/5/QtWidgets  @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets XTPlayer.app/Contents/Frameworks/QtAVWidgets.framework/Versions/1/QtAVWidgets
install_name_tool -change ~/Qt/5.15.2/clang_64/lib/QtGui.framework/Versions/5/QtGui  @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui XTPlayer.app/Contents/Frameworks/QtAVWidgets.framework/Versions/1/QtAVWidgets
install_name_tool -change ~/Qt/5.15.2/clang_64/lib/QtCore.framework/Versions/5/QtCore  @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore XTPlayer.app/Contents/Frameworks/QtAVWidgets.framework/Versions/1/QtAVWidgets
mkdir XTPlayer-v0.27b_MacOS_x86_64
cp -R XTPlayer.app XTPlayer-v0.27b_MacOS_x86_64/
hdiutil create /tmp/tmp.dmg -ov -volname "XTPlayer-v0.27_MacOS_x86_64" -fs HFS+ -srcfolder "XTPlayer-v0.27b_MacOS_x86_64"
hdiutil convert /tmp/tmp.dmg -format UDZO -o XTPlayer-v0.27b_MacOS_x86_64.dmg

