QT += core gui serialport network gamepad texttospeech compress websockets multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
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
    CustomControls/rangeslider.cpp \
    addchanneldialog.cpp \
    addplaylistdialog.cpp \
    dlnascriptlinks.cpp \
    lib/handler/httphandler.cpp \
    lib/handler/deohandler.cpp \
    lib/handler/devicehandler.cpp \
    lib/handler/funscripthandler.cpp \
    lib/handler/gamepadhandler.cpp \
    lib/handler/loghandler.cpp \
    lib/handler/medialibraryhandler.cpp \
    lib/handler/serialhandler.cpp \
    lib/handler/settingshandler.cpp \
    lib/handler/synchandler.cpp \
    lib/handler/tcodehandler.cpp \
    lib/handler/udphandler.cpp \
    lib/handler/videohandler.cpp \
    lib/handler/vrdevicehandler.cpp \
    lib/handler/websockethandler.cpp \
    lib/handler/whirligighandler.cpp \
    lib/handler/xtpwebhandler.cpp \
    lib/handler/xvideopreview.cpp \
    lib/handler/xvideopreviewwidget.cpp \
    lib/handler/xvideosurface.cpp \
    lib/handler/xvideowidget.cpp \
    lib/lookup/Constant.cpp \
    lib/lookup/tcodechannellookup.cpp \
    lib/struct/channeltablecomboboxdelegate.cpp \
    lib/struct/channeltableviewmodel.cpp \
    lib/struct/librarylistwidgetitem.cpp \
    lib/tool/imagefactory.cpp \
    lib/tool/simplecrypt.cpp \
    lib/tool/tcodefactory.cpp \
    lib/tool/xmath.cpp \
    lib/tool/xtimer.cpp \
    libraryItemSettingsDialog.cpp \
    libraryexclusions.cpp \
    librarywindow.cpp \
    main.cpp \
    mainwindow.cpp \
    noMatchingScriptDialog.cpp \
    playercontrols.cpp \
    settingsdialog.cpp \
    welcomedialog.cpp

HEADERS += \
    CustomControls/rangeslider.h \
    addchanneldialog.h \
    addplaylistdialog.h \
    dlnascriptlinks.h \
    lib/handler/httphandler.h \
    lib/handler/deohandler.h \
    lib/handler/devicehandler.h \
    lib/handler/gamepadhandler.h \
    lib/handler/medialibraryhandler.h \
    lib/handler/synchandler.h \
    lib/handler/videohandler.h \
    lib/handler/vrdevicehandler.h \
    lib/handler/websockethandler.h \
    lib/handler/whirligighandler.h \
    lib/handler/xtpwebhandler.h \
    lib/handler/xvideopreview.h \
    lib/handler/xvideopreviewwidget.h \
    lib/handler/xvideosurface.h \
    lib/handler/xvideowidget.h \
    lib/lookup/AxisNames.h \
    lib/lookup/Constants.h \
    lib/lookup/GamepadAxisNames.h \
    lib/lookup/MediaActions.h \
    lib/lookup/XMedia.h \
    lib/lookup/enum.h \
    lib/handler/funscripthandler.h \
    lib/handler/loghandler.h \
    lib/handler/loghandler.h \
    lib/handler/serialhandler.h \
    lib/handler/settingshandler.h \
    lib/handler/tcodehandler.h \
    lib/handler/udphandler.h \
    lib/lookup/enum.h \
    lib/lookup/tcodechannellookup.h \
    lib/lookup/xvideorenderer.h \
    lib/struct/Bookmark.h \
    lib/struct/ChannelModel.h \
    lib/struct/ConnectionChangedSignal.h \
    lib/struct/DecoderModel.h \
    lib/struct/Funscript.h \
    lib/struct/GamepadState.h \
    lib/struct/LibraryListItem.h \
    lib/struct/LibraryListItem27.h \
    lib/struct/LibraryListItemMetaData.h \
    lib/struct/LibraryListItemMetaData258.h \
    lib/struct/NetworkAddress.h \
    lib/struct/SerialComboboxItem.h \
    lib/struct/VRPacket.h \
    lib/struct/channeltablecomboboxdelegate.h \
    lib/struct/channeltableviewmodel.h \
    lib/struct/librarylistwidgetitem.h \
    lib/tool/boolinq.h \
    lib/tool/imagefactory.h \
    lib/tool/simplecrypt.h \
    lib/tool/tcodefactory.h \
    lib/tool/videoformat.h \
    lib/tool/xmath.h \
    lib/tool/xtimer.h \
    libraryItemSettingsDialog.h \
    libraryexclusions.h \
    librarywindow.h \
    mainwindow.h \
    noMatchingScriptDialog.h \
    playercontrols.h \
    settingsdialog.h \
    welcomedialog.h

FORMS += \
    dlnascriptlinks.ui \
    libraryexclusions.ui \
    mainwindow.ui \
    settings.ui \
    welcomedialog.ui

# LIBS       += -lVLCQtCore -lVLCQtWidgets
unix:!mac {
    QMAKE_RPATHDIR += ../lib
    LIBS += -L$$PWD/../../HttpServer/src/build/release -lhttpServer
    INCLUDEPATH += $$PWD/../../HttpServer/src
    DEPENDPATH += $$PWD/../../HttpServer/src/build/release
}
unix:mac {

#    #INCLUDEPATH += $$QT.core.libs/QtCompress.framework/Versions/5/Headers
#    QMAKE_LFLAGS += -F$$QT.core.libs
##    RPATHDIR *= @loader_path/../Frameworks
##    QMAKE_RPATHDIR += @executable_path/../lib /usr/lib
##    QMAKE_RPATHDIR += @loader_path/../Frameworks
#    themes.files = $$PWD/themes
#    themes.path = Contents/MacOS
#    QMAKE_BUNDLE_DATA += themes;
#    images.files = $$PWD/images
#    images.path = Contents/MacOS
#    QMAKE_BUNDLE_DATA += images;
#    #LIBS += -framework QtCompress
LIBS += -L$$PWD/../../HttpServer/src/build/release -lhttpServer
INCLUDEPATH += $$PWD/../../HttpServer/src/build/release
DEPENDPATH += $$PWD/../../HttpServer/src/build/release
INCLUDEPATH += $$PWD/../../HttpServer/src

    RPATHDIR *= @loader_path/../Frameworks @executable_path/../Frameworks
    QMAKE_LFLAGS_SONAME = -W1,-install_name,@rpath,
    isEmpty(QMAKE_LFLAGS_RPATH): QMAKE_LFLAGS_RPATH=-Wl,-rpath,
    for(R,RPATHDIR) {
        QMAKE_LFLAGS *= \'$${QMAKE_LFLAGS_RPATH}$$R\'
    }
    ICON = $$PWD/images/icons/XTP-icon.icns
}
unix {
    DESTDIR = $$shell_path($$OUT_PWD)
}
win32{
    #LIBS += -L$$QT.core.libs -lQtAV1 -lQtAVWidgets1
    build_pass: CONFIG(debug, debug|release) {
        DESTDIR = $$shell_path($$OUT_PWD/debug)
        #CONFIG(release, debug|release):

        #include($$PWD/../../HttpServer/HttpServer.pro)
        #LIBS += -L$$PWD/../../build-HttpServer-Desktop_Qt_5_15_2_MinGW_64_bit-Debug/debug -lhttpServer
        LIBS += -L$$PWD/../../build-HttpServer-Desktop_Qt_5_15_2_MinGW_64_bit-Debug/src/debug -lhttpServer
        INCLUDEPATH += $$PWD/../../build-HttpServer-Desktop_Qt_5_15_2_MinGW_64_bit-Debug/src/debug
        DEPENDPATH += $$PWD/../../build-HttpServer-Desktop_Qt_5_15_2_MinGW_64_bit-Debug/src/debug
#        TEMPLATE = subdirs
#        #SOURCE_ROOT += ../../
#        SUBDIRS += libQtAV
#        depends += libQtAV
#        libQtAV.file = ../../QtAV/QtAV.pro
#        include(../../QtAV/root.pri)
#        include(../../QtAV/src/libQtAV.pri)
    }
    else: build_pass {
        DESTDIR = $$shell_path($$OUT_PWD/release)
        #LIBS += -L$$PWD/../../build-HttpServer-Desktop_Qt_5_15_2_MinGW_64_bit-Release/release -lhttpServer
        LIBS += -L$$PWD/../../build-HttpServer-Desktop_Qt_5_15_2_MinGW_64_bit-Release/src/release -lhttpServer
        INCLUDEPATH += $$PWD/../../build-HttpServer-Desktop_Qt_5_15_2_MinGW_64_bit-Release/src/release
        DEPENDPATH += $$PWD/../../build-HttpServer-Desktop_Qt_5_15_2_MinGW_64_bit-Release/src/release
        #INCLUDEPATH += ../../QtAV-Builds/Release/x64/include
    }
    INCLUDEPATH += $$PWD/../../HttpServer/src
    RC_FILE = XTPlayer.rc
}

include($$PWD/../../HttpServer/3rdparty/qtpromise/qtpromise.pri)
#mkspecs_features.files    = $$PWD/qss/default.qss
#mkspecs_features.path     = $$OUT_PWD/qss
#INSTALLS                  += mkspecs_features

#https://stackoverflow.com/questions/19066593/copy-a-file-to-build-directory-after-compiling-project-with-qt
#copydata.commands = $(COPY_DIR) \"$$shell_path($$PWD\\qss\\)\" \"$$shell_path($$OUT_PWD)\"
#first.depends = $(first) copydata
#export(first.depends)
#export(copydata.commands)
#QMAKE_EXTRA_TARGETS += first copydata

#execute script
#mypackagerule.target = mypackagerule
#mypackagerule.command = exec my_package_script.sh
#QMAKE_EXTRA_TARGETS += mypackagerule

copydata.commands = $(COPY_DIR) $$shell_path($$PWD/themes) $$shell_path($$DESTDIR/themes) | $(COPY_DIR) $$shell_path($$PWD/www) $$shell_path($$DESTDIR/www)
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

#QMAKE_EXTRA_TARGETS += foo bar
#foo.target = $$shell_path($$DESTDIR/themes)
#foo.commands = $(MKDIR) $$shell_path($$DESTDIR/themes)
#bar.target = $$OUT_PWD/newFolder/file
#bar.commands = $(COPY_DIR) $$shell_path($$PWD/themes/default.qss)
#bar.depends = foo

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    qss/default.qss

RESOURCES += \
    icons.qrc
