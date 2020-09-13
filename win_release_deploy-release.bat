C:\Qt\5.15.0\mingw81_64\bin\windeployqt .\build-XTPlayer-Desktop_Qt_5_15_0_MinGW_64_bit-Release\release\XTPlayer.exe -dir .\bin\release\
xcopy .\build-XTPlayer-Desktop_Qt_5_15_0_MinGW_64_bit-Release\release\XTPlayer.exe .\bin\release\
xcopy .\build-XTPlayer-Desktop_Qt_5_15_0_MinGW_64_bit-Release\release\themes .\bin\release\themes /s /i
xcopy .\build-QtAV-Desktop_Qt_5_15_0_MinGW_64_bit-Release\bin\*.dll .\bin\release\
xcopy .\libav-x86_64-w64-mingw32-11.7\usr\bin\*.dll .\bin\release\
xcopy C:\Qt\5.15.0\mingw81_64\bin\libgcc_s_seh-1.dll .\bin\release\
xcopy C:\Qt\5.15.0\mingw81_64\bin\libstdc++-6.dll .\bin\release\
xcopy C:\Qt\5.15.0\mingw81_64\bin\libwinpthread-1.dll .\bin\release\
xcopy C:\Qt\5.15.0\mingw81_64\bin\Qt5OpenGL.dll .\bin\release\
pause