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
#include "loghandler.h"
#include "devicehandler.h"
#include "settingshandler.h"
#include "../struct/NetworkAddress.h"
#include "../struct/ConnectionChangedSignal.h"
#include "../struct/DeoPacket.h"

class DeoHandler : public QObject
{
    Q_OBJECT

signals:
    void errorOccurred(QString error);
    void connectionChange(ConnectionChangedSignal status);
    void messageRecieved(DeoPacket message);

public:
    DeoHandler(QObject *parent = nullptr);
    ~DeoHandler();
    void init(NetworkAddress _address, int waitTimeout = 5000);
    void dispose();
    void send(const QString &command);
    bool isConnected();
    bool isPlaying();
    //void togglePause();
    DeoPacket getCurrentDeoPacket();

private:
    void readData();
    void onSocketStateChange (QAbstractSocket::SocketState state);
    void sendKeepAlive();
    void tcpErrorOccured(QAbstractSocket::SocketError);

    DeoPacket* currentDeoPacket = nullptr;
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
