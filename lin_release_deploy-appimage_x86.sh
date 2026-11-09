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
qtRootDirectory="${home}"Qt
qtVersion="6.12.0"
qtDirectory="${qtRootDirectory}/${qtVersion}"
qtGccDirectory="${qtDirectory}"/gcc_64
qtGccBinDirectory="${qtGccDirectory}"/bin
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
export qtRootDirectory
export qtVersion
export qtDirectory
export qtGccDirectory
export qtGccBinDirectory
export SCRIPT_DIR

${SCRIPT_DIR}/lin_release_deploy-appimage.sh
