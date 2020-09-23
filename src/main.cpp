#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaTypeStreamOperators<ChannelModel>("ChannelModel");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
