@echo off

SET currentPath=%cd%
SET QTDIR=C:\Qt\6.9.1\mingw_64
SET mingwDir=C:\Qt\Tools\mingw1310_64\bin
SET QtDirBin=%QTDIR%\bin\
SET xtplayerSource="%UserProfile%\git\XTPlayer"
SET xtengineSource="%UserProfile%\git\XTEngine"
SET xtplayerBuildDirectory="%xtplayerSource%\build-release"
SET xtengineBuildDirectory="%xtengineSource%\build-release"
SET buildDir=%xtplayerBuildDirectory%\release
SET engineBuildDir=%xtengineBuildDirectory%\release
SET deployDir=%xtplayerSource%\bin\release\
SET deployZipDir=%xtplayerSource%\bin\
SET stagingDirectory=%xtplayerSource%\bin\latest
SET sevenZipLocation="C:/Program Files/7-Zip/7z.exe"
SET mingw=%mingwDir%\mingw32-make.exe
rem this is where the zip file will be copied for backup
SET releaseDirectory="\\192.168.0.218\main\STK\Dev\XTP\XTP-Release\"
rem SET httpServerDll=..\HttpServer\build\release\httpServer.dll
rem SET zlibDll=..\zlib-1.3.1\build\Desktop_Qt_5_15_2_MinGW_64_bit-Release\libzlib.dll

SET PATH=%PATH%;%QtDirBin%;%mingwDir%

SET /P version=Enter a version (example: 0.5b):
IF NOT DEFINED version SET "version=UNKNOWN"

cd %xtplayerSource%
git pull || echo "Error: Pull XTP source" && cd %currentPath% && exit /b %errorlevel%
cd %xtengineSource%
git pull || echo "Error: Pull XTE source" && cd %currentPath% && exit /b %errorlevel%

if exist %xtplayerBuildDirectory% (
	rd /s /q %xtplayerBuildDirectory% || echo "Error: Clean XTP build" && cd %currentPath% && exit /b %errorlevel%
)
if exist %xtengineBuildDirectory%  (
	rd /s /q %xtengineBuildDirectory% || echo "Error: Clean XTE build" && cd %currentPath% && exit /b %errorlevel%
)

rem C:/Qt/Tools/mingw1310_64/bin/mingw32-make -f Makefile.Release

mkdir %xtengineBuildDirectory% || echo "Error: Make XTE build directory" && cd %currentPath% && exit /b %errorlevel%
cd %xtengineBuildDirectory%
qmake %xtengineSource%\src\XTEngine.pro CONFIG+=release || echo "Error: Qmake XTE" && cd %currentPath% && exit /b %errorlevel%
%mingw% -f Makefile qmake_all || echo "Error: Make XTE Makefile" && cd %currentPath% && exit /b %errorlevel%
%mingw% -j8 --silent || echo "Error: Make XTE" && cd %currentPath% && exit /b %errorlevel%

mkdir %xtplayerBuildDirectory% || echo "Error: Make XTP build directory" && cd %currentPath% && exit /b %errorlevel%
cd %xtplayerBuildDirectory%
qmake %xtplayerSource%\src\XTPlayer.pro CONFIG+=release || echo "Error: Qmake XTP" && cd %currentPath% && exit /b %errorlevel%
%mingw% -f Makefile qmake_all || echo Error: "Make XTP Makefile" && cd %currentPath% && exit /b %errorlevel%
%mingw% -j8 --silent || echo "Error: Make XTP" && cd %currentPath% && exit /b %errorlevel%

cd %xtplayerSource%

%QtDirBin%windeployqt %buildDir%\XTPlayer.exe -dir %deployDir% || echo "Error: windeployqt XTP" && cd %currentPath% && exit /b %errorlevel%
%QtDirBin%windeployqt %engineBuildDir%\XTEngine.dll -dir %deployDir% || echo "Error: windeployqt XTE" && cd %currentPath% && exit /b %errorlevel%
xcopy %buildDir%\XTPlayer.exe %deployDir% /s /i  /K /D /H /Y
xcopy %engineBuildDir%\XTEngine.dll %deployDir%/s /i  /K /D /H /Y
xcopy %buildDir%\themes %deployDir%themes /s /i /K /D /H /Y
rem xcopy %zlibDll% %deployDir% /s /i /K /D /H /Y
rem xcopy %httpServerDll% %deployDir% /s /i /K /D /H /Y
rem xcopy %QtDirBin%Qt5OpenGL.dll %deployDir% /s /i /K /D /H /Y
rem xcopy %QtDirBin%Qt5Compress.dll %deployDir% /s /i /K /D /H /Y
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
cd %currentPath%
pause