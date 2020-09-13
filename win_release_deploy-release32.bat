windeployqt .\build-XTPlayer-Desktop_Qt_5_15_0_MinGW_32_bit-Release\release\XTPlayer.exe -dir .\bin32\release\
xcopy .\build-XTPlayer-Desktop_Qt_5_15_0_MinGW_32_bit-Release\release\XTPlayer.exe .\bin32\release\
xcopy C:\QtAV\bin\*.dll .\bin\release\
xcopy .\libav-x86_64-w64-mingw32-11.7\usr\bin\*.dll  .\bin32\release\
pause