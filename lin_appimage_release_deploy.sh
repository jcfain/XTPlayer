#!/bin/bash

targetDirectory=./Deploy/XTPlayer-v0.32a_Linux_x86_64
binDirectory=./build-XTPlayer-Desktop_Qt_5_15_2_GCC_64bit-Release
librDirectory=/home/jay/Qt/5.15.2/gcc_64/lib
qmakeDirectory=/home/jay/Qt/5.15.2/gcc_64/bin/qmake

./appimage-builder-1.0.2-x86_64.AppImage --appdir $binDirectory
#mkdir $targetDirectory/bin/themes
#cp -R src/themes/ $targetDirectory/bin/
#mkdir $targetDirectory/bin/www
#cp src/www/index-min.html $targetDirectory/bin/www/
#cp src/www/index-min.js $targetDirectory/bin/www/
#cp src/www/styles-min.css $targetDirectory/bin/www/
#cp src/www/favicon.ico $targetDirectory/bin/www/
#cp -R lib/* $targetDirectory/lib
#cp src/rename_me_for_local_settings.ini $targetDirectory/bin
#cp "XTP and DeoVR guide.pdf" $targetDirectory
read -p "Press enter to resume ..."


