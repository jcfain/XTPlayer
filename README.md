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
    
## Installing from [release package](https://github.com/jcfain/XTPlayer/releases/latest)
      1. Grab the latest release package for your OS from the release section on this page. https://github.com/jcfain/XTPlayer/releases/latest.
      2. On windows, extract the zip to a place where your user has permissions to.
      3. Run the executable.
      4. Choose at least one path to the media/funscripts to add.
      5. The application will scan the directory and all subdirectories for media and their funscripts then start generating thumbnails. This can take sometime on first startup depending on how many files you have.

## Building from source
     1. Download and install the following apps
          *  QT Creator Community Open source edition: https://www.qt.io/download 
             During installation, make sure to get Qt 6.9.1. with the aditional libraries: HTTP Server, Serial port, Speech, Connectivity, MultiMedia, Wayland(Linux) and WebSockets.
     2. Required libraries needed:
          * https://github.com/pumphaus/qtgamepadlegacy
               * I usually open the CMakeLists.txt in Qt creator and build them copy the folders ./bin, ./include etc to the Qt directory of the same names. (There is probably a better way to do this)
               * MACOS Only: If you want to build a universal (arm/x86) binary, open the Projects tab on the left side of Qt Creator and select Build. Scroll down to CMake and find the configuration "CMAKE_OSX_ARCHITECTURES" Enter "x86_64;x86_64h;arm64" as the value.
      You may need to check the XTEngine.pro and XTPlayer.pro build files for specific library directories. See the win32 section for windows.
      If you have trouble building these I added a couple files in the "dependencies" directory of this repo for reminders.
      3. Clone/download XTPlayer https://github.com/jcfain/XTPlayer/ and XTEngine https://github.com/jcfain/XTEngine/ source into the same directory.
      4. Open the XTEngine and XTPlayer projects in Qt Creator
      5. Select XTPlayer click Projects on the rightside then Dependencies and select XTEngine and check sync.
      6. Select the "Build" item under "Desktop Qt 6.9.1" and change the Build directory to "path to"/git/XTPlayer/build-release 
      7. Do the same for XTEngine but set it to "path to"git/XTEngine/build-release.
      8. Do the same for the debug build if you want to build debug.
      9. You should be able to build now.
          
 ## Deploying
      1. In the root directory of this repository there is a file named "win_release_deploy-release.bat"
         You may want to edit the file and make sure your directories are the same first.
      2. Double click this file to run.
      3. Make sure all commands say at least "1 files copied".
      4. The newly deployed app should be in a newly created dir ".\bin\release"
      5. You should be able to ship this directory to any windows 64 pc.
    


