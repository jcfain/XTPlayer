INCLUDEPATH += $$PWD/../build-zlib-Desktop_Qt_5_15_2_MinGW_64_bit-Release/release
LIBS += -L$$PWD/../build-zlib-Desktop_Qt_5_15_2_MinGW_64_bit-Release/release -lz
include($$PWD/3rdparty/qtpromise/qtpromise.pri)
