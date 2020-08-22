#ifndef UDPHANDLER_H
#define UDPHANDLER_H
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "loghandler.h"
#include "settingshandler.h"
#include "../struct/NetworkAddress.h"
#include "../struct/ConnectionChangedSignal.h"

class UdpHandler : public QThread
{
    Q_OBJECT

signals:
    void errorOccurred(QString error);
    void connectionChange(ConnectionChangedSignal status);

public:
    explicit UdpHandler(QObject *parent = nullptr);
    ~UdpHandler();
    void init(NetworkAddress _address, int waitTimeout = 5000);
    void dispose();
    void sendTCode(QString tcode);
    bool isConnected();

private:
    void run() override;
    void readData();
    void onSocketStateChange (QAbstractSocket::SocketState state);

    NetworkAddress _address;
    QWaitCondition _cond;
    QMutex _mutex;
    QString _tcode;
    int _waitTimeout = 0;
    bool _stop = false;
    bool _isConnected = false;
    bool _isSelected = false;
};

#endif // UDPHANDLER_H
