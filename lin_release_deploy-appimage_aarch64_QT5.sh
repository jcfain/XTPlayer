#!/bin/bash

home=~/
target='/media/NAS/main/STK/Dev/XTP/XTP-Release/'
architecture="aarch64"
xtplayerSource="${home}"git/XTPlayer
xtplayerLocation="${home}"git/XTPlayer/build-XTPlayer-Desktop-Release/
xtengineLocation="${home}"git/XTEngine/build-XTEngine-Desktop-Release/release/
httpserverLocation="${home}"git/HttpServer/src/build/release/
deployDirectory="${home}"git/XTPlayer/Deploy/
qtDirectory=/usr/lib/qt5/

export home
export target
export architecture
export xtplayerSource
export xtplayerLocation
export xtengineLocation
export httpserverLocation
export deployDirectory
export qtDirectory

./lin_release_deploy-appimage.sh
