#!/bin/bash

home=~/
target='/media/NAS/main/STK/Dev/XTP/XTP-Release/'
architecture="x86_64"
xtplayerSource="${home}"git/XTPlayer
xtengineSource="${home}"git/XTEngine
xtplayerBuildDirectory="${xtplayerSource}"/build-release
xtengineBuildDirectory="${xtengineSource}"/build-release
xtplayerLocation="${xtplayerBuildDirectory}"/release/
xtengineLocation="${xtengineBuildDirectory}"/release/
deployDirectory="${xtplayerSource}"/Deploy/
qtDirectory="${home}"Qt/6.9.1/gcc_64/bin

export home
export target
export architecture
export xtplayerSource
export xtengineSource
export xtplayerBuildDirectory
export xtengineBuildDirectory
export xtplayerLocation
export xtengineLocation
export deployDirectory
export qtDirectory

./lin_release_deploy-appimage.sh
