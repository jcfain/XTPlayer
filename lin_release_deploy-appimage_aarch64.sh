#!/bin/bash

home=~/
target='/media/NAS/main/STK/Dev/XTP/XTP-Release/'
architecture="aarch64"
xtplayerSource="${home}"git/XTPlayer
xtengineSource="${home}"git/XTEngine
xtplayerBuildDirectory="${xtplayerSource}"/build-release
xtengineBuildDirectory="${xtengineSource}"/build-release
xtplayerLocation="${xtplayerBuildDirectory}"/release/
xtengineLocation="${xtengineBuildDirectory}"/release/
deployDirectory="${xtplayerSource}"/Deploy/
qtDirectory="${home}"Qt/6.9.1/gcc_arm64/bin
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

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
export SCRIPT_DIR

${SCRIPT_DIR}/lin_release_deploy-appimage.sh
