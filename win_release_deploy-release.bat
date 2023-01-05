@echo off

SET QtDir=C:\Qt\5.15.2\mingw81_64\bin\
SET buildDir=.\build-XTPlayer-Desktop_Qt_5_15_2_MinGW_64_bit-Release\
SET engineBuildDir=..\XTEngine\build-XTEngine-Desktop_Qt_5_15_2_MinGW_64_bit-Release\
SET deployDir=.\bin\release\
SET deployZipDir=.\bin\
SET stagingDirectory=.\bin\latest
SET sevenZipLocation="C:\Program Files\7-Zip\7z.exe"

SET /P version=Enter a version (example: 0.423b):
IF NOT DEFINED version SET "version=UNKNOWN"

%QtDir%windeployqt %buildDir%release\XTPlayer.exe -dir %deployDir%
%QtDir%windeployqt %engineBuildDir%release\XTEngine.dll -dir %deployDir%
xcopy %buildDir%release\XTPlayer.exe %deployDir% /s /i  /K /D /H /Y
xcopy %engineBuildDir%release\XTEngine.dll %deployDir%/s /i  /K /D /H /Y
xcopy %buildDir%release\themes %deployDir%themes /s /i /K /D /H /Y
REM xcopy %QtDir%QtAV1.dll %deployDir%
REM xcopy %QtDir%QtAVWidgets1.dll %deployDir%
REM xcopy %QtDir%avcodec-57.dll %deployDir%
REM xcopy %QtDir%avdevice-57.dll %deployDir%
REM xcopy %QtDir%avfilter-6.dll %deployDir%
REM xcopy %QtDir%avformat-57.dll %deployDir%
REM xcopy %QtDir%avresample-3.dll %deployDir%
REM xcopy %QtDir%swresample-2.dll %deployDir%
REM xcopy %QtDir%avutil-55.dll %deployDir%
REM xcopy %QtDir%swscale-4.dll %deployDir%
REM xcopy %QtDir%libvo-aacenc-0.dll %deployDir%
REM xcopy %QtDir%libmp3lame-0.dll %deployDir%
REM xcopy %QtDir%libvorbis-0.dll %deployDir%
REM xcopy %QtDir%libopus-0.dll %deployDir%
REM xcopy %QtDir%libx264-142.dll %deployDir%
REM xcopy %QtDir%libvorbisenc-2.dll %deployDir%
REM xcopy %QtDir%libogg-0.dll %deployDir%
xcopy %QtDir%zlib1.dll %deployDir% /s /i /K /D /H /Y
xcopy %QtDir%Qt5OpenGL.dll %deployDir% /s /i /K /D /H /Y
xcopy %QtDir%Qt5Compress.dll %deployDir% /s /i /K /D /H /Y
xcopy ..\build-HttpServer-Desktop_Qt_5_15_2_MinGW_64_bit-Release\src\release\httpServer.dll %deployDir% /s /i /K /D /H /Y
xcopy ..\XTEngine\src\rename_me_for_local_settings.ini %deployDir% /s /i /K /D /H /Y
xcopy "XTP_and_VR_guide.pdf" %deployDir% /s /i /K /D /H /Y
xcopy .\src\XTPlayerDebug.bat %deployDir% /s /i /K /D /H /Y
xcopy .\src\XTPlayerVerbose.bat %deployDir% /s /i /K /D /H /Y
xcopy .\src\README.txt %deployDir% /s /i /K /D /H /Y
xcopy ..\XTEngine\src\www\*-min.html %deployDir%www\ /s /i /K /D /H /Y
xcopy ..\XTEngine\src\www\*-min.js %deployDir%www\ /s /i /K /D /H /Y
xcopy ..\XTEngine\src\www\*-min.css %deployDir%www\ /s /i /K /D /H /Y
xcopy ..\XTEngine\src\www\favicon.ico %deployDir%www\ /s /i /K /D /H /Y

xcopy %deployDir%*.* %stagingDirectory% /s /i /K /D /H /Y

%sevenZipLocation% a -tzip %deployZipDir%XTPlayer-v%version%_Win_x86_64.zip %deployDir%

pause