#!/bin/bash

home=~/
target='/media/NAS/main/STK/Dev/XTP/XTP-Release/'
architecture="aarch64"
xtplayerSource="${home}"git/XTPlayer
xtplayerLocation="${home}"git/XTPlayer/build-release/release/
xtengineLocation="${home}"git/XTEngine/build-release/release/
deployDirectory="${home}"git/XTPlayer/Deploy/
qtDirectory=/usr/lib/qt6.9.1/

export home
export target
export architecture
export xtplayerSource
export xtplayerLocation
export xtengineLocation
export deployDirectory
export qtDirectory

./lin_release_deploy-appimage.sh
