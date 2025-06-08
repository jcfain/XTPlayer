#!/bin/bash

cd ~/git/
git clone https://github.com/FFmpeg/nv-codec-headers.git
cd nv-codec-headers/
mkdir build
cd build
sudo make -j install
pause

#build ffmpeg for Qt 6.9.1
mkdir ~/git
cd ~/git
git clone --branch n7.1 https://git.ffmpeg.org/ffmpeg.git ffmpeg
sudo apt install yasm libva-dev

mkdir ~/git/ffmpeg/build
cd ~/git/ffmpeg/build
../configure --prefix=/usr/local/ffmpeg --disable-doc --enable-network --enable-shared --enable-libaom

pause