#include "mainwindow.h"

#include <QApplication>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char *argv[])
{
    #ifdef _WIN32
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
    #endif
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    qRegisterMetaType<QItemSelection>();
    qRegisterMetaTypeStreamOperators<QList<QString>>("QList<QString>");
    qRegisterMetaTypeStreamOperators<ChannelModel>("ChannelModel");
    qRegisterMetaType<AxisDimension>("AxisDimension");
    qRegisterMetaType<AxisType>("AxisType");
    qRegisterMetaTypeStreamOperators<AxisName>("AxisName");
    qRegisterMetaTypeStreamOperators<DecoderModel>("DecoderModel");
    qRegisterMetaTypeStreamOperators<XMediaStatus>("XMediaStatus");
    qRegisterMetaType<LibraryListItem>();
    qRegisterMetaTypeStreamOperators<LibraryListItem>("LibraryListItem");
    qRegisterMetaTypeStreamOperators<QMap<QString, QList<LibraryListItem>>>("QMap<QString, QList<LibraryListItem>>");
    qRegisterMetaTypeStreamOperators<QList<LibraryListItem>>("QList<LibraryListItem>");
    qRegisterMetaType<LibraryListItem27>();
    qRegisterMetaTypeStreamOperators<LibraryListItem27>("LibraryListItem27");
    qRegisterMetaTypeStreamOperators<QMap<QString, QList<LibraryListItem27>>>("QMap<QString, QList<LibraryListItem27>>");
    qRegisterMetaTypeStreamOperators<QList<LibraryListItem27>>("QList<LibraryListItem27>");
    qRegisterMetaTypeStreamOperators<TCodeVersion>("TCodeVersion");
    qRegisterMetaTypeStreamOperators<LibraryListItemMetaData>("LibraryListItemMetaData");
    qRegisterMetaTypeStreamOperators<LibraryListItemMetaData258>("LibraryListItemMetaData258");
    qRegisterMetaTypeStreamOperators<Bookmark>("Bookmark");
    qRegisterMetaType<QVector<int> >("QVector<int>");
    QApplication a(argc, argv);
    MainWindow w(a.arguments());
    w.show();
    return a.exec();
}
