#ifndef UDPHANDLER_H
#define UDPHANDLER_H
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <math.h>
#include "loghandler.h"
#include "devicehandler.h"
#include "settingshandler.h"
#include "../struct/NetworkAddress.h"

class UdpHandler : public DeviceHandler
{

public:
    explicit UdpHandler(QObject *parent = nullptr);
    ~UdpHandler();
    void init(NetworkAddress _address, int waitTimeout = 5000);
    void dispose() override;
    void sendTCode(const QString &tcode) override;
    bool isConnected() override;

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
