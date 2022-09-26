SET QtDir=C:\Qt\5.15.2\mingw81_64\bin\
SET buildDir=.\build-XTPlayer-Desktop_Qt_5_15_2_MinGW_64_bit-Release\
SET engineBuildDir=..\XTEngine\build-XTEngine-Desktop_Qt_5_15_2_MinGW_64_bit-Release\
SET deployDir=.\bin\release\

%QtDir%windeployqt %buildDir%release\XTPlayer.exe -dir %deployDir%
%QtDir%windeployqt %engineBuildDir%release\XTEngine.dll -dir %deployDir%
xcopy %buildDir%release\XTPlayer.exe %deployDir%
xcopy %engineBuildDir%release\XTEngine.dll %deployDir%
xcopy %buildDir%release\themes %deployDir%themes /s /i
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
xcopy %QtDir%zlib1.dll %deployDir%
xcopy %QtDir%Qt5OpenGL.dll %deployDir%
xcopy %QtDir%Qt5Compress.dll %deployDir%
xcopy ..\build-HttpServer-Desktop_Qt_5_15_2_MinGW_64_bit-Release\src\release\httpServer.dll %deployDir%
xcopy ..\XTEngine\src\rename_me_for_local_settings.ini %deployDir%
xcopy "XTP_and_DeoVR-HereSphere_guide.pdf" %deployDir%
xcopy .\src\XTPlayerDebug.bat %deployDir%
xcopy .\src\XTPlayerVerbose.bat %deployDir%
xcopy .\src\README.txt %deployDir%
xcopy ..\XTEngine\src\www\*-min.html %deployDir%www\
xcopy ..\XTEngine\src\www\*-min.js %deployDir%www\
xcopy ..\XTEngine\src\www\*-min.css %deployDir%www\
xcopy ..\XTEngine\src\www\favicon.ico %deployDir%www\

pause