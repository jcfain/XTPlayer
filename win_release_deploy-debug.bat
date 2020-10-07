C:\Qt\5.15.0\mingw81_64\bin\windeployqt .\build-XTPlayer-Desktop_Qt_5_15_0_MinGW_64_bit-Debug\debug\XTPlayer.exe -dir .\bin\debug\
xcopy .\build-XTPlayer-Desktop_Qt_5_15_0_MinGW_64_bit-Debug\debug\XTPlayer.exe .\bin\debug\
xcopy .\build-XTPlayer-Desktop_Qt_5_15_0_MinGW_64_bit-Debug\debug\themes .\bin\debug\themes /s /i
xcopy .\build-QtAV-Desktop_Qt_5_15_0_MinGW_64_bit-Release\bin\*.dll .\bin\debug\
xcopy .\libav-x86_64-w64-mingw32-11.7\usr\bin\*.dll .\bin\debug\
xcopy C:\Qt\5.15.0\mingw81_64\bin\libgcc_s_seh-1.dll .\bin\debug\
xcopy C:\Qt\5.15.0\mingw81_64\bin\libstdc++-6.dll .\bin\debug\
xcopy C:\Qt\5.15.0\mingw81_64\bin\libwinpthread-1.dll .\bin\debug\
xcopy C:\Qt\5.15.0\mingw81_64\bin\Qt5OpenGL.dll .\bin\debug\
xcopy .\src\rename_me_for_local_settings.ini .\bin\debug\
pause