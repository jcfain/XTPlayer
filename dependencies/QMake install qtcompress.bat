mkdir build
cd build
C:\Qt\5.15.0\mingw81_64\bin\qmake.exe ..\qtcompress-master\qtcompress.pro
pause
C:\Qt\Tools\mingw810_64\bin\mingw32-make.exe -j4
pause
C:\Qt\Tools\mingw810_64\bin\mingw32-make.exe check
pause
C:\Qt\Tools\mingw810_64\bin\mingw32-make.exe install
pause