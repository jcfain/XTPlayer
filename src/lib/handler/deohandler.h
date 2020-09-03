#ifndef DEOHANDLER_H
#define DEOHANDLER_H
#include <QTcpSocket>
#include <QDebug>
#include <QHostAddress>

class DeoHandler
{
public:
    DeoHandler();

private:
    QTcpSocket  _socket;
};

#endif // DEOHANDLER_H
