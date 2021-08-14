#ifndef DEOHANDLER_H
#define DEOHANDLER_H
#include <QTcpSocket>
#include <QNetworkDatagram>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include "vrdevicehandler.h"

class DeoHandler : public VRDeviceHandler
{

    Q_OBJECT
signals:
//    void errorOccurred(QString error);
//    void connectionChange(ConnectionChangedSignal status);
//    void messageRecieved(VRPacket message);

public:
    explicit DeoHandler(QObject *parent = nullptr);
    ~DeoHandler();
    void init(NetworkAddress _address, int waitTimeout = 5000) override;
    void dispose() override;
    void send(const QString &command) override;
    bool isConnected() override;
    bool isPlaying() override;
    //void togglePause();
    VRPacket getCurrentPacket() override;

private:
    void readData();
    void onSocketStateChange (QAbstractSocket::SocketState state);
    void tcpErrorOccured(QAbstractSocket::SocketError);
    void sendKeepAlive();
    void tearDown();

    VRPacket* currentPacket = nullptr;
    QTcpSocket* tcpSocket = nullptr;
    QTimer* keepAliveTimer = nullptr;
    QMutex _mutex;
    NetworkAddress _address;
    QString _sendCommand;
    int _waitTimeout = 0;
    bool _isConnected = false;
    bool _isPlaying = false;
    bool _isSelected = false;
    quint64 _currentTime;
};

#endif // DEOHANDLER_H
