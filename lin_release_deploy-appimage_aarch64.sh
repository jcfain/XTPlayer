#!/bin/bash

home=~/
target='/media/NAS/main/STK/Dev/XTP/XTP-Release/'
architecture="aarch64"
xtplayerSource="${home}"git/XTPlayer
xtengineSource="${home}"git/XTEngine
xtplayerLocation="${xtplayerSource}"/build-release/release/
xtengineLocation="${xtengineSource}"/build-release/release/
deployDirectory="${xtplayerSource}"/Deploy/
qtDirectory="${home}"Qt/6.9.1/gcc_arm64/bin

export home
export target
export architecture
export xtplayerSource
export xtengineSource
export xtplayerLocation
export xtengineLocation
export deployDirectory
export qtDirectory

./lin_release_deploy-appimage.sh
