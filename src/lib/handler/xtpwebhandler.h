#ifndef XTPWEBHANDLER_H
#define XTPWEBHANDLER_H

#include <QJsonDocument>
#include <QMutex>
#include <QTimer>
#include "vrdevicehandler.h"
#include "httphandler.h"

class XTPWebHandler : public VRDeviceHandler
{
    Q_OBJECT
signals:
public:
    explicit XTPWebHandler(QObject *parent = nullptr);
    ~XTPWebHandler();
    void init(NetworkAddress _address, int waitTimeout = 5000) override;
    void init(HttpHandler* httpHandler);
    void dispose() override;
    void send(const QString &command) override;
    bool isConnected() override;
    bool isPlaying() override;
    //void togglePause();
    VRPacket getCurrentPacket() override;

private:
    void readData(QByteArray data);
    HttpHandler* _httpHandler = 0;
    VRPacket* _currentPacket = 0;
    QMutex _mutex;
    NetworkAddress _address;
    QString _sendCommand;
    int _waitTimeout = 0;
    bool _isConnected = false;
    bool _isPlaying = false;
    bool _isSelected = false;
    qint64 _currentTime;
};

#endif // XTPWEBHANDLER_H
