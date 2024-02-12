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
    * Control the app and TCode devices with a gamepad
    * Sync with DeoVR/HereSphere and Whirligig VR meadia players
    * Built in web server for viewing media collection with a variaty of devices.
    * And much more


## Building on Windows
     1. Download and install the following apps
          *  QT Creator Community Open source edition: https://www.qt.io/download 
             DUring installation, make sure to get Qt 5.15.2. ONLY Qt 5.15.2 is supported at this time.
          *  GNU make: https://gnuwin32.sourceforge.net/packages/make.htm
          *  Strawberry Perl: https://www.perl.org/get.html#win32
     2. Required libraries needed:
          * https://github.com/nezticle/qtcompress
               * Follow the instructions on the github
          * https://github.com/addisonElliott/HttpServer
               * Follow the instructions on the github
               * you dont need OpenSSL unless you do.
               * Youll need to build zlib. I usually open the CmakeLists.txt with Qt creator and build from there.
      If you have trouble building these I added a couple files in the "dependencies" directory of this repo for reminders.
      3. Clone/download XTPlayer https://github.com/jcfain/XTPlayer/ and XTEngine https://github.com/jcfain/XTEngine/ source 
      4. Open the XTEngine and XTPlayer projects in Qt Creator
      5. Select XTPlayer click Projects on the rightside then Dependencies and select XTEngine and check sync.
      6. Build Release or debug
          
 ## Deploying
      1. In the root directory of this repository there is a file named "win_release_deploy-release.bat"
      2. Double click this file to run.
      3. Make sure all commands say at least "1 files copied".
      4. The newly deployed app should be in a newly created dir ".\bin\release"
      5. You should be able to ship this directory to any windows 64 pc.
    


