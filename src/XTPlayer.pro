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
    inputmapwidget.cpp \
    lib/handler/dialoghandler.cpp \
    lib/handler/videohandler.cpp \
    lib/handler/xtpsettings.cpp \
    lib/handler/xvideopreviewwidget.cpp \
    lib/handler/xvideowidget.cpp \
    lib/struct/channeltablecomboboxdelegate.cpp \
    lib/struct/channeltableviewmodel.cpp \
    lib/struct/librarylistviewmodel.cpp \
    lib/struct/librarysortfilterproxymodel.cpp \
    lib/struct/playlistviewmodel.cpp \
    libraryItemSettingsDialog.cpp \
    libraryManager.cpp \
    libraryexclusions.cpp \
    librarywindow.cpp \
    main.cpp \
    mainwindow.cpp \
    noMatchingScriptDialog.cpp \
    playercontrols.cpp \
    settingsdialog.cpp \
    welcomedialog.cpp \
    widgets/heatmapwidget.cpp \
    xlibrarylist.cpp

HEADERS += \
    CustomControls/rangeslider.h \
    addchanneldialog.h \
    addplaylistdialog.h \
    dlnascriptlinks.h \
    inputmapwidget.h \
    lib/handler/dialoghandler.h \
    lib/handler/videohandler.h \
    lib/handler/xtpsettings.h \
    lib/handler/xvideopreviewwidget.h \
    lib/handler/xvideowidget.h \
    lib/struct/channeltablecomboboxdelegate.h \
    lib/struct/channeltableviewmodel.h \
    lib/struct/librarylistviewmodel.h \
    lib/struct/librarysortfilterproxymodel.h \
    lib/struct/playlistviewmodel.h \
    libraryItemSettingsDialog.h \
    libraryManager.h \
    libraryexclusions.h \
    librarywindow.h \
    mainwindow.h \
    noMatchingScriptDialog.h \
    playercontrols.h \
    settingsdialog.h \
    welcomedialog.h \
    widgets/heatmapwidget.h \
    xlibrarylist.h

FORMS += \
    dlnascriptlinks.ui \
    libraryexclusions.ui \
    mainwindow.ui \
    settings.ui \
    welcomedialog.ui

unix {
    DESTDIR = $$shell_path($$OUT_PWD)
}
unix:!mac {
    #QMAKE_RPATHDIR += ../lib
    INCLUDEPATH += $$PWD/../../XTEngine/src
    DEPENDPATH += $$PWD/../../XTEngine/src
    INCLUDEPATH += $$PWD/../../HttpServer/src
    DEPENDPATH += $$PWD/../../HttpServer/src
    CONFIG(debug, debug|release) {
        LIBS += -L$$PWD/../../XTEngine/build-XTEngine-Desktop_Qt_5_15_2_GCC_64bit-Debug/debug -lxtengine
        LIBS += -L$$PWD/../../HttpServer/src/build/debug -lhttpServer
    }
    else:CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../XTEngine/build-XTEngine-Desktop_Qt_5_15_2_GCC_64bit-Release/release -lxtengine
        LIBS += -L$$PWD/../../HttpServer/src/build/release -lhttpServer
    }
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
    INCLUDEPATH += $$PWD/../../XTEngine/src
    DEPENDPATH += $$PWD/../../XTEngine/src
    DEPENDPATH += $$PWD/../../HttpServer/src
    INCLUDEPATH += $$PWD/../../HttpServer/src
    CONFIG(debug, debug|release) {
        LIBS += -L$$PWD/../../XTEngine/build-XTEngine-Desktop_Qt_5_15_2_clang_64bit-Debug/debug -lxtengine
        LIBS += -L$$PWD/../../HttpServer/src/build/debug -lhttpServer
    }
    else:CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../XTEngine/build-XTEngine-Desktop_Qt_5_15_2_clang_64bit-Release/release -lxtengine
        LIBS += -L$$PWD/../../HttpServer/src/build/release -lhttpServer
    }

    RPATHDIR *= @loader_path/../Frameworks @executable_path/../Frameworks
    QMAKE_LFLAGS_SONAME = -W1,-install_name,@rpath,
    isEmpty(QMAKE_LFLAGS_RPATH): QMAKE_LFLAGS_RPATH=-Wl,-rpath,
    for(R,RPATHDIR) {
        QMAKE_LFLAGS *= \'$${QMAKE_LFLAGS_RPATH}$$R\'
    }
    ICON = $$PWD/images/icons/XTP-icon.icns
}
win32{
    #LIBS += -L$$QT.core.libs -lQtAV1 -lQtAVWidgets1
    build_pass: CONFIG(debug, debug|release) {
        DESTDIR = $$shell_path($$OUT_PWD/debug)
        #CONFIG(release, debug|release):

        #include($$PWD/../../HttpServer/HttpServer.pro)
        #LIBS += -L$$PWD/../../build-HttpServer-Desktop_Qt_5_15_2_MinGW_64_bit-Debug/debug -lhttpServer
        LIBS += -L$$PWD/../../XTEngine/build-XTEngine-Desktop_Qt_5_15_2_MinGW_64_bit-Debug/debug -lXTEngine
        LIBS += -L$$PWD/../../build-HttpServer-Desktop_Qt_5_15_2_MinGW_64_bit-Debug/src/debug -lhttpServer
#        TEMPLATE = subdirs
#        #SOURCE_ROOT += ../../
#        SUBDIRS += libQtAV
#        depends += libQtAV
#        libQtAV.file = ../../QtAV/QtAV.pro
#        include(../../QtAV/root.pri)
#        include(../../QtAV/src/libQtAV.pri)
    }
    else:build_pass:CONFIG(release, debug|release): {
        DESTDIR = $$shell_path($$OUT_PWD/release)
        LIBS += -L$$PWD/../../XTEngine/build-XTEngine-Desktop_Qt_5_15_2_MinGW_64_bit-Release/release -lXTEngine
        LIBS += -L$$PWD/../../build-HttpServer-Desktop_Qt_5_15_2_MinGW_64_bit-Release/src/release -lhttpServer
        #INCLUDEPATH += ../../QtAV-Builds/Release/x64/include
    }
    INCLUDEPATH += $$PWD/../../XTEngine/src
    DEPENDPATH += $$PWD/../../XTEngine/src
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

copydata.commands = $(COPY_DIR) $$shell_path($$PWD/themes) $$shell_path($$DESTDIR/themes) | $(COPY_DIR) $$shell_path($$PWD/../../XTEngine/src/www) $$shell_path($$DESTDIR/www)
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

