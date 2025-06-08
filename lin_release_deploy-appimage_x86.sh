#!/bin/bash

home=~/
target='/media/NAS/main/STK/Dev/XTP/XTP-Release/'
architecture="x86_64"
xtplayerSource="${home}"git/XTPlayer
xtplayerLocation="${home}"git/XTPlayer/build-release/release/
xtengineLocation="${home}"git/XTEngine/build-release/release/
deployDirectory="${home}"git/XTPlayer/Deploy/
qtDirectory="${home}"Qt/6.9.1/gcc_64/bin

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
