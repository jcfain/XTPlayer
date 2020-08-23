windeployqt .\build-XTPlayer-Desktop_Qt_5_15_0_MinGW_64_bit-Release\release\XTPlayer.exe -dir .\bin\release\
xcopy .\build-XTPlayer-Desktop_Qt_5_15_0_MinGW_64_bit-Release\release\XTPlayer.exe .\bin\release\
xcopy .\build-QtAV-Desktop_Qt_5_15_0_MinGW_64_bit-Release\bin\*.dll .\bin\release\
xcopy .\libav-x86_64-w64-mingw32-11.7\usr\bin\*.dll  .\bin\release\
pause