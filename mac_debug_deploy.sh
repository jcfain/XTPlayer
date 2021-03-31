cd build-XTPlayer-Desktop_Qt_5_15_2_clang_64bit-Debug
~/Qt/5.15.2/clang_64/bin/macdeployqt XTPlayer.app -dmg -verbose=3
cp -R ../src/themes XTPlayer.app/Contents/MacOS/
cp -R ../src/images XTPlayer.app/Contents/MacOS/