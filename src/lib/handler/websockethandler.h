#ifndef WEBSOCKETHANDLER_H
#define WEBSOCKETHANDLER_H

#include <QObject>
#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"

#include "settingshandler.h"
#include "../struct/ConnectionChangedSignal.h"

class WebSocketHandler: public QObject
{
    Q_OBJECT
signals:
    void connectTCodeDevice();
    void tcode(QString tcode);
public:
    WebSocketHandler(QObject *parent = nullptr);
    ~WebSocketHandler();
    void sendCommand(QString command, QString message = nullptr);
    void sendDeviceConnectionStatus(ConnectionChangedSignal status);

private:
    QWebSocketServer *m_pWebSocketServer;
    QList<QWebSocket *> m_clients;
    ConnectionChangedSignal _tcodeDeviceStatus = {DeviceType::Serial, ConnectionStatus::Disconnected, "Disconnected"};

private:
    void closed();
    void onNewConnection();
    void processBinaryMessage(QByteArray message);
    void processTextMessage(QString message);
    void socketDisconnected();
};

#endif // WEBSOCKETHANDLER_H
