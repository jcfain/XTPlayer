#!/bin/bash

echo "Enter version: (ex: 0.423b)"  
read versionInput
export version=v${versionInput}

home=~/
targetDirectory=./Deploy/Binary/XTPlayer-${version}_Linux_x86_64
xteTargetDirectory=./Deploy/Binary/XTEngine-${version}_Linux_x86_64
binDirectory=./build-XTPlayer-Desktop_Qt_5_15_2_GCC_64bit-Release/XTPlayer
xteBinDirectory=../XTEngine/build-XTEngine-Desktop-Release/release
librDirectory=/home/jay/Qt/5.15.2/gcc_64/lib
qmakeDirectory=/home/jay/Qt/5.15.2/gcc_64/bin/qmake
httpserverLocation="${home}"git/HttpServer/src/build/release/
qtCompressDir=../build-qtcompress-Desktop_Qt_5_15_2_GCC_64bit-Release/lib
zlibDir=../build-zlib-Desktop_Qt_5_15_2_GCC_64bit-Release

#export PATH="${qtDirectory}":$PATH
#export LD_LIBRARY_PATH="${binDirectory}lib;${xteBinDirectory};${httpserverLocation}"


rm -rf "${targetDirectory}"

cqtdeployer -bin $xteBinDirectory clear -libDir $librDirectory -qmake $qmakeDirectory -targetDir $xteTargetDirectory
cqtdeployer -bin $binDirectory clear -libDir $librDirectory -qmake $qmakeDirectory -targetDir $targetDirectory
mkdir $targetDirectory/bin/themes
cp -R src/themes/ $targetDirectory/bin/
mkdir $targetDirectory/bin/www
cp $xteBinDirectory/www/*-min.html $targetDirectory/bin/www/
cp $xteBinDirectory/www/*-min.js $targetDirectory/bin/www/
cp $xteBinDirectory/www/*-min.css $targetDirectory/bin/www/
cp $xteBinDirectory/www/favicon.ico $targetDirectory/bin/www/

cp -R $xteTargetDirectory/bin/*.so* $targetDirectory/lib
cp -R $xteTargetDirectory/lib/*.so* $targetDirectory/lib
cp -R $xteTargetDirectory/plugins $targetDirectory/plugins
cp -R $xteTargetDirectory/translations $targetDirectory/translations
cp -R $httpserverLocation/*.so* $targetDirectory/lib
cp -R $qtCompressDir/*.so* $targetDirectory/lib
cp -R $zlibDir/*.so* $targetDirectory/lib
#cp -R Deploy/lib/* $targetDirectory/lib
#cp -R $xteTargetDirectory/lib/* $targetDirectory/lib
cp ../XTEngine/src/rename_me_for_local_settings.ini $targetDirectory/bin
#cp $xteBinDirectory/*.* $targetDirectory/lib
rm -r $xteTargetDirectory
cp "XTP_and_VR_guide.pdf" $targetDirectory/bin/"XTP_and_VR_guide.pdf"
read -p "Press enter to resume ..."


