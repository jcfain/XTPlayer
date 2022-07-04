#!/bin/bash

targetDirectory=./Deploy/XTPlayer-v0.3a_Linux_x86_64
binDirectory=./build-XTPlayer-Desktop_Qt_5_15_2_GCC_64bit-Release/XTPlayer
librDirectory=/home/jay/Qt/5.15.2/gcc_64/lib
qmakeDirectory=/home/jay/Qt/5.15.2/gcc_64/bin/qmake
cqtdeployer -bin $binDirectory clear -libDir $librDirectory -qmake $qmakeDirectory -targetDir $targetDirectory
mkdir $targetDirectory/bin/themes
cp -R src/themes/ $targetDirectory/bin/
mkdir $targetDirectory/bin/www
cp src/www/*-min.html $targetDirectory/bin/www/
cp src/www/*-min.js $targetDirectory/bin/www/
cp src/www/*-min.css $targetDirectory/bin/www/
cp src/www/favicon.ico $targetDirectory/bin/www/
cp -R Deploy/lib/* $targetDirectory/lib
cp src/rename_me_for_local_settings.ini $targetDirectory/bin
cp "XTP and DeoVR guide.pdf" $targetDirectory/"XTP and DeoVR guide.pdf"
read -p "Press enter to resume ..."


