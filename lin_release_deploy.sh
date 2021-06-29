#!/bin/bash
cqtdeployer -confFile cqt-deb.conf 
#-bin ./build-XTPlayer-Desktop_Qt_5_15_0_GCC_64bit-Release -icon ./src/images/icons/XTP-icon.png
#-libDir /home/jay/Qt/5.15.0/gcc_64/lib
#-targetDir ./XTPlayer-release noOverwrite deploySystem extractPlugins
#-deployVersion 0.13b -releaseDate $(date) +'%m/%d/%Y' -icon ./src/images/icons/XTP-icon.png
cp -R src/themes ./XTPlayer-v0.258b_Linux_x86_64/bin
cp -R lib/* ./XTPlayer-v0.258b_Linux_x86_64/lib
cp src/rename_me_for_local_settings.ini ./XTPlayer-v0.258b_Linux_x86_64/bin
cp "XTP and DeoVR guide.pdf" ./XTPlayer-v0.258b_Linux_x86_64
read -p "Press enter to resume ..."


