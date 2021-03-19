# XTPlayer
XTPlayer is a cross platform TCode media player. 
![Main window](/main.jpg)

## Features: 
    * Load a folder with videos for easy access.
    * Built in funscript to TCode conversion
    * Load any funscript with any video
    * Customize the player looks via an "easy" CSS syntax.
    * Add multipliers to TCode's other axis' via the current funscript pos
    * Control TCode devices with a gamepad
    * Sync with DeoVR and Whirligig VR meadi players
    * And much more


## Building on Windows
     1. Download and install QT Creator
          * https://www.qt.io/download
     2. Required libraries needed:
          ###Qtav
          * https://github.com/wang-bin/QtAV
               * As the time of this writing the project has not had any commits for awhile
               * So its safe to get the latest master branch.
            * https://libav.org/download/ (requaired to build QtAV)
               * I on windows I used prebuilts I found here: 
                    * http://builds.libav.org/windows/release-gpl/libav-x86_64-w64-mingw32-11.7.7z
                    * Add the extracted folder path to the lib and include to the QTAV projects Build environment (Release/Debug)
                        * CPATH path/To/Extracted/include
                        * LD_LIBRARY_PATH path/to/extracted/lib
                        * LIBRARY_PATH path/to/extracted/lib
                        * Copy the contents of libav-x86_64-w64-mingw32-11.7\usr\bin to C:\Qt\5.15.2\mingw81_64\bin
          * If the compile completes with out error, execute the generated sdk_install.bat in build-QtAV-Desktop_Qt_5_15_2_MinGW_64_bit-Debug
          ###Other
          * https://github.com/nezticle/qtcompress
               * Build from QT Creator
               * Install make and do make install from the output build dir.
                  Note: see "QMake install qtcompress.bat" for details
      3. Clone/download XTP source
      4. Open the project and build
          
    
    


