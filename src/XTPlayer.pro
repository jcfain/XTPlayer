QT       += core gui serialport network

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets avwidgets
} else {
  CONFIG += avwidgets
}
#rpath for apple
mac {
  RPATHDIR *= @loader_path/../Frameworks
  isEmpty(QMAKE_LFLAGS_RPATH): QMAKE_LFLAGS_RPATH=-Wl,-rpath,
  for(R,RPATHDIR) {
    QMAKE_LFLAGS *= \'$${QMAKE_LFLAGS_RPATH}$$R\'
  }
}
CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CustomControls/RangeSlider.cpp \
    lib/handler/funscripthandler.cpp \
    lib/handler/loghandler.cpp \
    lib/handler/serialhandler.cpp \
    lib/handler/settingshandler.cpp \
    lib/handler/tcodehandler.cpp \
    lib/handler/udphandler.cpp \
    lib/handler/videohandler.cpp \
    lib/tool/xmath.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    CustomControls/RangeSlider.h \
    lib/handler/videohandler.h \
    lib/lookup/enum.h \
    lib/handler/funscripthandler.h \
    lib/handler/loghandler.h \
    lib/handler/loghandler.h \
    lib/handler/serialhandler.h \
    lib/handler/settingshandler.h \
    lib/handler/tcodehandler.h \
    lib/handler/udphandler.h \
    lib/lookup/enum.h \
    lib/struct/ConnectionChangedSignal.h \
    lib/struct/Funscript.h \
    lib/struct/LibraryListItem.h \
    lib/struct/NetworkAddress.h \
    lib/struct/SerialComboboxItem.h \
    lib/tool/xmath.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# LIBS       += -lVLCQtCore -lVLCQtWidgets
unix:!mac {
    LIBS += -L$$QT.core.libs -lQtAV
}
unix:mac {
    INCLUDEPATH += $$QT.core.libs/QtAV.framework/Versions/1/Headers
    QMAKE_LFLAGS += -F$$QT.core.libs
    LIBS += -framework QtAV
}
win32{
    LIBS += -L$$QT.core.libs -lQtAV1
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
