#!/bin/bash
cqtdeployer -confFile cqt-deb.conf 
#-bin ./build-XTPlayer-Desktop_Qt_5_15_0_GCC_64bit-Release -icon ./src/images/icons/XTP-icon.png
#-libDir /home/jay/Qt/5.15.0/gcc_64/lib
#-targetDir ./XTPlayer-release noOverwrite deploySystem extractPlugins
#-deployVersion 0.13b -releaseDate $(date) +'%m/%d/%Y' -icon ./src/images/icons/XTP-icon.png
cp -R src/themes ./DeploymentPackageDeb/bin
cp -R lib/* ./DeploymentPackageDeb/lib
cp src/rename_me_for_local_settings.ini ./DeploymentPackageDeb/bin
read -p "Press enter to resume ..."


