#!/bin/bash

targetDirectory=./XTPlayer-v0.27b_Linux_x86_64
binDirectory=./build-XTPlayer-Desktop_Qt_5_15_0_GCC_64bit-Release/XTPlayer
librDirectory=/home/jay/Qt/5.15.0/gcc_64/lib
qmakeDirectory=/home/jay/Qt/5.15.0/gcc_64/bin/qmake

cqtdeployer -bin $binDirectory -clear true -deploySystem false -extractPlugins false -noCheckRPATH false -qif false -libDir $librDirectory -qmlDir . -qmake $qmakeDirectory -targetDir $targetDirectory
mkdir $targetDirectory/bin/themes
cp -R src/themes/ $targetDirectory/bin/
cp -R lib/* $targetDirectory/lib
cp src/rename_me_for_local_settings.ini $targetDirectory/bin
cp "XTP and DeoVR guide.pdf" $targetDirectory
read -p "Press enter to resume ..."


