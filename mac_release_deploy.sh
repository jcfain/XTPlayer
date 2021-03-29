cd build-XTPlayer-Desktop_Qt_5_15_2_clang_64bit-Release
~/Qt/5.15.2/clang_64/bin/macdeployqt XTPlayer.app 
cp -R ../src/themes XTPlayer.app/Contents/MacOS/
cp -R ../src/images XTPlayer.app/Contents/MacOS/