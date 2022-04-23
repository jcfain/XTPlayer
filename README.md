# XTPlayer
XTPlayer is a cross platform TCode media player. 
![Main window](/main.jpg)

## Features: 
    * Sync VR/2d/audio/stand alone funscripts with any robot that runs under the TCode standard V2 or V3 
    * Load a folder with videos for easy access.
    * Built in funscript to TCode conversion
    * Load any funscript with any video
    * Customize the player looks via an "easy" CSS like syntax.
    * Add psuedo random motion to TCode's other axis' via the current funscript pos
    * Control TCode devices with a gamepad
    * Sync with DeoVR/HereSphere and Whirligig VR meadi players
    * Built in web server for viewing media collection with a variaty of devices.
    * And much more


## Building on Windows
     1. Download and install QT Creator
          * https://www.qt.io/download
     2. Required libraries needed:
          ### QtAV
          * https://github.com/wang-bin/QtAV
               * As the time of this writing the project has not had any commits for awhile
               * So its safe to get the latest master branch.
            * https://libav.org/download/ (requaired to build QtAV)
               * I on windows I used ffmpeg prebuilts of v4.3 I found here: 
                    * https://sourceforge.net/projects/avbuild/files/windows-desktop/ ffmpeg-4.3-windows-desktop-clang-default.tar.xz
                        (In you use a different version you may need to modify the deploy bat)
                    * Add the extracted folder path to the lib and include to the QTAV projects Build environment (Release/Debug)
                        * CPATH path/To/Extracted/include
                        * LD_LIBRARY_PATH path/to/extracted/lib
                        * LIBRARY_PATH path/to/extracted/lib
                        * Copy the contents of libav-x86_64-w64-mingw32-11.7\usr\bin to C:\Qt\5.15.2\mingw81_64\bin
          * If the compile completes with out error, execute the generated sdk_install.bat in build-QtAV-Desktop_Qt_5_15_2_MinGW_64_bit-Debug
          ### Other
          * https://github.com/nezticle/qtcompress
               * Build from QT Creator
               * Install make and do make install from the output build dir.
                  Note: see "QMake install qtcompress.bat" for details
          * https://github.com/addisonElliott/HttpServer
      3. Clone/download XTP source
      4. Open the project and build
          
 ## Deploying
      1. In the root directory of this repository there is a file named "win_release_deploy-release.bat"
      2. Double click this file to run.
      3. Make sure all commands say at least "1 files copied".
      4. The newly deployed app should be in a newly created dir ".\bin\release"
      5. You should be able to ship this directory to any windows 64 pc.
    


