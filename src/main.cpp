#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    qRegisterMetaTypeStreamOperators<QList<QString>>("QList<QString>");
    qRegisterMetaTypeStreamOperators<ChannelModel>("ChannelModel");
    qRegisterMetaTypeStreamOperators<DecoderModel>("DecoderModel");
    qRegisterMetaTypeStreamOperators<LibraryListItem>("LibraryListItem");
    qRegisterMetaTypeStreamOperators<QMap<QString, QList<LibraryListItem>>>("QMap<QString, QList<LibraryListItem>>");
    qRegisterMetaTypeStreamOperators<QList<LibraryListItem>>("QList<LibraryListItem>");
    QApplication a(argc, argv);
    MainWindow w(a.arguments());
    w.show();
    return a.exec();
}
