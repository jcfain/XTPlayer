#!/bin/bash

version=v0.444b
targetDirectory=./Deploy/XTPlayer-${version}_Linux_x86_64
xteTargetDirectory=./Deploy/XTEngine-${version}_Linux_x86_64
binDirectory=./build-XTPlayer-Desktop_Qt_5_15_2_GCC_64bit-Release/XTPlayer
xteBinDirectory=../XTEngine/build-XTEngine-Desktop-Release/release
librDirectory=/home/jay/Qt/5.15.2/gcc_64/lib
qmakeDirectory=/home/jay/Qt/5.15.2/gcc_64/bin/qmake
cqtdeployer -bin $xteBinDirectory clear -libDir $librDirectory -qmake $qmakeDirectory -targetDir $xteTargetDirectory
cqtdeployer -bin $binDirectory clear -libDir $librDirectory -qmake $qmakeDirectory -targetDir $targetDirectory
mkdir $targetDirectory/bin/themes
cp -R src/themes/ $targetDirectory/bin/
mkdir $targetDirectory/bin/www
cp $xteBinDirectory/www/*-min.html $targetDirectory/bin/www/
cp $xteBinDirectory/www/*-min.js $targetDirectory/bin/www/
cp $xteBinDirectory/www/*-min.css $targetDirectory/bin/www/
cp $xteBinDirectory/www/favicon.ico $targetDirectory/bin/www/
cp -R Deploy/lib/* $targetDirectory/lib
cp -R $xteTargetDirectory/lib/* $targetDirectory/lib
cp ../XTEngine/src/rename_me_for_local_settings.ini $targetDirectory/bin
cp $xteBinDirectory/*.* $targetDirectory/lib
rm -r $xteTargetDirectory
cp "XTP_and_VR_guide.pdf" $targetDirectory/bin/"XTP_and_VR_guide.pdf"
read -p "Press enter to resume ..."


