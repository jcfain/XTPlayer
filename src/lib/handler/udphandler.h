#ifndef UDPHANDLER_H
#define UDPHANDLER_H
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QThread>
#include <QMutex>
#include "loghandler.h"
#include "settingshandler.h"
#include "../struct/NetworkAddress.h"
#include "../struct/ConnectionChangedSignal.h"

class UdpHandler: public QObject
{

    Q_OBJECT

signals:
    void errorOccurred(QString error);
    void connectionChange(ConnectionChangedSignal status);

public:
    UdpHandler();
    ~UdpHandler();
    void init(NetworkAddress address);
    void Dispose();
    void sendTCode(QString tcode);

private:
    QScopedPointer<QUdpSocket> udpSocketRecieve;
    QScopedPointer<QUdpSocket> udpSocketSend;
    NetworkAddress address;
    QHostAddress addressObj;
    static QMutex mutex;
    bool tCodeConnected = false;
    bool isSelected = false;
    void readData();
    void onSocketStateChange (QAbstractSocket::SocketState state);
};

#endif // UDPHANDLER_H
