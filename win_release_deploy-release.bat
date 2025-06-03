@echo off

SET QtDir=C:\Qt\6.9.1\mingw_64\bin\
SET buildDir=.\build\
SET engineBuildDir=..\XTEngine\build\
SET deployDir=.\bin\release\
SET deployZipDir=.\bin\
SET stagingDirectory=.\bin\latest
SET sevenZipLocation="C:\Program Files\7-Zip\7z.exe"
rem this is where the zip file will be copied for backup
SET releaseDirectory="\\192.168.0.218\main\STK\Dev\XTP\XTP-Release\"
rem SET httpServerDll=..\HttpServer\build\release\httpServer.dll
rem SET zlibDll=..\zlib-1.3.1\build\Desktop_Qt_5_15_2_MinGW_64_bit-Release\libzlib.dll

SET /P version=Enter a version (example: 0.5b):
IF NOT DEFINED version SET "version=UNKNOWN"

%QtDir%windeployqt %buildDir%release\XTPlayer.exe -dir %deployDir%
%QtDir%windeployqt %engineBuildDir%release\XTEngine.dll -dir %deployDir%
xcopy %buildDir%release\XTPlayer.exe %deployDir% /s /i  /K /D /H /Y
xcopy %engineBuildDir%release\XTEngine.dll %deployDir%/s /i  /K /D /H /Y
xcopy %buildDir%release\themes %deployDir%themes /s /i /K /D /H /Y
rem xcopy %zlibDll% %deployDir% /s /i /K /D /H /Y
rem xcopy %httpServerDll% %deployDir% /s /i /K /D /H /Y
rem xcopy %QtDir%Qt5OpenGL.dll %deployDir% /s /i /K /D /H /Y
rem xcopy %QtDir%Qt5Compress.dll %deployDir% /s /i /K /D /H /Y
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

xcopy %deployZipDir%XTPlayer-v%version%_Win_x86_64.zip %releaseDirectory% /s /i /K /D /H /Y

pause