#!/bin/bash
cqtdeployer -bin ./build-XTPlayer-Desktop_Qt_5_15_0_GCC_64bit-Release/ -targetDir ./XTPlayer-release 
#-deployVersion 0.13b -releaseDate $(date) +'%m/%d/%Y' -icon ./src/images/icons/XTP-icon.png
cp -R src/themes ./XTPlayer-release/bin
cp -R lib/* ./XTPlayer-release/lib
read -p "Press enter to resume ..."


