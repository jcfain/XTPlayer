#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaTypeStreamOperators<ChannelModel>("ChannelModel");
    qRegisterMetaTypeStreamOperators<DecoderModel>("DecoderModel");
    QApplication a(argc, argv);
    MainWindow w(a.arguments());
    w.show();
    return a.exec();
}
