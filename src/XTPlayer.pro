QT += core gui serialport network texttospeech  websockets multimedia multimediawidgets bluetooth


greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}
equals(QT_MAJOR_VERSION, 5) {
    QT +=  gamepad compress
    DEFINES += BUILD_QT5=1
}
equals(QT_MAJOR_VERSION, 6) {
    QT += httpserver gamepadlegacy
    DEFINES += BUILD_QT6=1
}
CONFIG += c++17
#-DFFMPEG_DIR=/usr/local/ffmpeg/

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/lib/widgets
INCLUDEPATH += $$PWD/lib/widgets/library
INCLUDEPATH += $$PWD/lib/widgets/video
INCLUDEPATH += $$PWD/lib/model
INCLUDEPATH += $$PWD/lib/model/library
INCLUDEPATH += $$PWD/lib/common

SOURCES += \
    gettextdialog.cpp \
    lib/widgets/library/LibraryItemMetadataDialog.cpp \
    lib/widgets/library/tagManager.cpp \
    lib/widgets/video/xvideographicsview.cpp \
    lib/widgets/video/xvideowidget.cpp \
    lib/widgets/video/xvideopreviewwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    xtpsettings.cpp \
    noMatchingScriptDialog.cpp \
    settingsdialog.cpp \
    welcomedialog.cpp \
    addchanneldialog.cpp \
    addplaylistdialog.cpp \
    dlnascriptlinks.cpp \
    inputmapwidget.cpp \
    lib/common/dialoghandler.cpp \
    lib/model/channeltablecomboboxdelegate.cpp \
    lib/model/channeltableviewmodel.cpp \
    lib/model/library/librarylistviewmodel.cpp \
    lib/model/library/librarysortfilterproxymodel.cpp \
    lib/model/library/playlistviewmodel.cpp \
    lib/widgets/library/libraryManager.cpp \
    lib/widgets/library/libraryexclusions.cpp \
    lib/widgets/library/librarywindow.cpp \
    lib/widgets/heatmapwidget.cpp \
    lib/widgets/timeline.cpp \
    lib/widgets/xwidget.cpp \
    lib/widgets/library/xlibrarylist.cpp \
    lib/widgets/video/playercontrols.cpp \
    lib/widgets/rangeslider.cpp \

HEADERS += \
    gettextdialog.h \
    lib/widgets/library/LibraryItemMetadataDialog.h \
    lib/widgets/library/tagManager.h \
    lib/widgets/video/xvideographicsview.h \
    lib/widgets/video/xvideowidget.h \
    lib/widgets/video/xvideopreviewwidget.h \
    mainwindow.h \
    xtpsettings.h \
    noMatchingScriptDialog.h \
    settingsdialog.h \
    welcomedialog.h \
    addchanneldialog.h \
    addplaylistdialog.h \
    dlnascriptlinks.h \
    inputmapwidget.h \
    lib/common/dialoghandler.h \
    lib/model/channeltablecomboboxdelegate.h \
    lib/model/channeltableviewmodel.h \
    lib/model/library/librarylistviewmodel.h \
    lib/model/library/librarysortfilterproxymodel.h \
    lib/model/library/playlistviewmodel.h \
    lib/widgets/library/libraryManager.h \
    lib/widgets/library/libraryexclusions.h \
    lib/widgets/library/librarywindow.h \
    lib/widgets/heatmapwidget.h \
    lib/widgets/timeline.h \
    lib/widgets/xwidget.h \
    lib/widgets/library/xlibrarylist.h \
    lib/widgets/video/playercontrols.h \
    lib/widgets/rangeslider.h \

equals(QT_MAJOR_VERSION, 6) {
HEADERS += \
    lib/widgets/video/videohandler.h
SOURCES += \
    lib/widgets/video/videohandler.cpp
}
FORMS += \
    dlnascriptlinks.ui \
    lib/widgets/library/libraryexclusions.ui \
    mainwindow.ui \
    settings.ui \
    welcomedialog.ui


CONFIG(debug, debug|release) {
    DESTDIR = $$shell_path($$OUT_PWD/debug)
    LIBS += -L$$PWD/../../XTEngine/build-debug/debug -lxtengine
} else:CONFIG(release, debug|release): {
    DESTDIR = $$shell_path($$OUT_PWD/release)
    LIBS += -L$$PWD/../../XTEngine/build-release/release -lxtengine
}
INCLUDEPATH += $$PWD/../../XTEngine/src
DEPENDPATH += $$PWD/../../XTEngine/src

unix:!mac {
    #QMAKE_RPATHDIR += ../lib
    CONFIG(debug, debug|release) {
        #LIBS += -L$$PWD/../../XTEngine/build/debug -lxtengine
        equals(QT_MAJOR_VERSION, 5) {
            LIBS += -L$$PWD/../../HttpServer/src/build/debug -lhttpServer
        }
    }
    else:CONFIG(release, debug|release): {
        #LIBS += -L$$PWD/../../XTEngine/build/release -lxtengine
        equals(QT_MAJOR_VERSION, 5) {
            LIBS += -L$$PWD/../../HttpServer/src/build/release -lhttpServer
        }
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
    CONFIG(debug, debug|release) {
        equals(QT_MAJOR_VERSION, 5) {
            #LIBS += -L$$PWD/../../XTEngine/build/debug -lxtengine
            LIBS += -L$$PWD/../../HttpServer/src/build/debug -lhttpServer
        }

        greaterThan(QT_MAJOR_VERSION, 5) {
            #LIBS += -L$$PWD/../../XTEngine/build/debug -lxtengine
        }
    }
    else:CONFIG(release, debug|release): {
        equals(QT_MAJOR_VERSION, 5) {
            #LIBS += -L$$PWD/../../XTEngine/build/release -lxtengine
            LIBS += -L$$PWD/../../HttpServer/src/build/release -lhttpServer
        }

        greaterThan(QT_MAJOR_VERSION, 5) {
            #LIBS += -L$$PWD/../../XTEngine/build/release -lxtengine
        }
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
        #DESTDIR = $$shell_path($$OUT_PWD/debug)
        #LIBS += -L$$PWD/../../XTEngine/build/debug -lXTEngine

        equals(QT_MAJOR_VERSION, 5) {
            LIBS += -L$$PWD/../../HttpServer/build/debug -lhttpServer
            INCLUDEPATH += $$PWD/../../HttpServer/build/debug
            LIBS += -L$$PWD/../../zlib-1.3.1/build/Desktop_Qt_5_15_2_MinGW_64_bit-Debug -lzlib
        }
    }
    else:build_pass:CONFIG(release, debug|release): {
        #DESTDIR = $$shell_path($$OUT_PWD/release)
        #LIBS += -L$$PWD/../../XTEngine/build/release -lXTEngine

        equals(QT_MAJOR_VERSION, 5) {
            LIBS += -L$$PWD/../../HttpServer/build/release -lhttpServer
            INCLUDEPATH += $$PWD/../../HttpServer/build/release
            LIBS += -L$$PWD/../../zlib-1.3.1/build/Desktop_Qt_5_15_2_MinGW_64_bit-Release -lzlib
        }
    }
    equals(QT_MAJOR_VERSION, 5) {
        INCLUDEPATH += $$PWD/../../HttpServer/src
    }
    RC_FILE = XTPlayer.rc
}

equals(QT_MAJOR_VERSION, 5) {
    include($$PWD/../../HttpServer/3rdparty/qtpromise/qtpromise.pri)
}
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

win32 {
    copydata.commands = $(COPY_DIR) $$shell_path($$PWD/themes) $$shell_path($$DESTDIR/themes) | $(COPY_DIR) $$shell_path($$PWD/../../XTEngine/src/www) $$shell_path($$DESTDIR/www)
} else {
    copydata.commands = $(COPY_DIR) $$PWD/themes $$DESTDIR | $(COPY_DIR) $$PWD/../../XTEngine/src/www $$DESTDIR
}
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

# defineTest(copyToDestDir) {
#     files = $$1
#     dir = $$2
#     # replace slashes in destination path for Windows
#     win32:dir ~= s,/,\\,g

#     for(file, files) {
#         # replace slashes in source path for Windows
#         win32:file ~= s,/,\\,g

#         QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$shell_quote($$file) $$shell_quote($$dir) $$escape_expand(\\n\\t)
#     }

#     export(QMAKE_POST_LINK)
# }

# copyToDestDir($$PWD/../../XTEngine/src/www, $$DESTDIR)
# copyToDestDir($$PWD/themes, $$DESTDIR)

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

