

mkdir build
cd build

set gitWorkSpace=%userprofile%\git

C:\Qt\5.15.2\mingw81_64\bin\qmake.exe %gitWorkSpace%\qtcompress\qtcompress.pro
pause
IF %ERRORLEVEL% NEQ 0 (EXIT /B %ERRORLEVEL%)
C:\Qt\Tools\mingw810_64\bin\mingw32-make.exe -j4
pause
IF %ERRORLEVEL% NEQ 0 (EXIT /B %ERRORLEVEL%)
C:\Qt\Tools\mingw810_64\bin\mingw32-make.exe check
pause
IF %ERRORLEVEL% NEQ 0 (EXIT /B %ERRORLEVEL%)
C:\Qt\Tools\mingw810_64\bin\mingw32-make.exe install
pause
IF %ERRORLEVEL% NEQ 0 (EXIT /B %ERRORLEVEL%)

EXIT /B 0

REM mkdir build
REM cd build
REM C:\Qt\5.15.2\mingw81_64\bin\qmake.exe %~dp0..\..\qtcompress\qtcompress.pro
REM pause
REM C:\Qt\Tools\mingw810_64\bin\mingw32-make.exe -j4
REM pause
REM C:\Qt\Tools\mingw810_64\bin\mingw32-make.exe check
REM pause
REM C:\Qt\Tools\mingw810_64\bin\mingw32-make.exe install
REM pause