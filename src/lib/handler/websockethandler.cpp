#include "websockethandler.h"

WebSocketHandler::WebSocketHandler(QObject *parent):
    QObject(parent),
    m_pWebSocketServer(new QWebSocketServer(QStringLiteral("XTP Websocket"),
                                            QWebSocketServer::NonSecureMode, this))
{
    if (m_pWebSocketServer->listen(QHostAddress::Any, 8080))
    {
        LogHandler::Debug("Websocket listening on port " +QString::number(8080));
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &WebSocketHandler::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &WebSocketHandler::closed);
    }
}

WebSocketHandler::~WebSocketHandler()
{
    sendCommand("connectionClosed");
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

void WebSocketHandler::sendCommand(QString command, QString message)
{
    QString commandJson;
    if(message.isEmpty())
        commandJson = "{ \"command\": \""+command+"\"}";
    else if( message.contains("{"))
        commandJson = "{ \"command\": \""+command+"\", \"message\": "+message+" }";
    else
        commandJson = "{ \"command\": \""+command+"\", \"message\": \""+message+"\"}";
    for (QWebSocket *pClient : qAsConst(m_clients))
        pClient->sendTextMessage(commandJson);
}

void  WebSocketHandler::sendDeviceConnectionStatus(ConnectionChangedSignal status)
{
    QString messageJson = "{ \"status\": "+QString::number(status.status)+", \"deviceType\": "+QString::number(status.deviceType)+", \"message\": \""+status.message+"\" }";
    if(status.deviceType == DeviceType::Serial || status.deviceType == DeviceType::Network)
    {
        _tcodeDeviceStatus = status;
        sendCommand("deviceStatus", messageJson);
    }
    else
    {
        _syncDeviceStatus = status;
        sendCommand("syncDeviceStatus", messageJson);
    }


}
void WebSocketHandler::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &WebSocketHandler::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived, this, &WebSocketHandler::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &WebSocketHandler::socketDisconnected);

    m_clients << pSocket;
}
#include <QJsonDocument>
void WebSocketHandler::processTextMessage(QString message)
{
    //QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    //LogHandler::Debug("WEBSOCKET Message received: " + message);

    QJsonObject json;

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if(!doc.isNull())
        if(doc.isObject())
            json = doc.object();
        else
            LogHandler::Error("Document is not an object");
    else
        LogHandler::Error("Invalid JSON...");
    QString command = json["command"].toString();
    if (command == "connectTCodeDevice") {
        emit connectTCodeDevice();
    } else if (command == "tcode") {
        QString commandMessage = json["message"].toString();
        emit tcode(commandMessage);
    } else if (command == "connectionStatus") {
        sendDeviceConnectionStatus(_tcodeDeviceStatus);
    }  else if (command == "connectSyncDevice") {
        QJsonObject obj = json["message"].toObject();
        emit connectSyncDevice((DeviceType)obj["deviceType"].toInt(), obj["enabled"].toBool());
    } else if (command == "syncConnectionStatus") {
        sendDeviceConnectionStatus(_syncDeviceStatus);
    }


//    if (pClient)
//        pClient->sendTextMessage(message);
}

void WebSocketHandler::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    LogHandler::Debug("WEBSOCKET Binary Message received: " + message);
    if (pClient)
        pClient->sendBinaryMessage(message);
}

void WebSocketHandler::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    LogHandler::Debug("WEBSOCKET socketDisconnected: " + pClient->objectName());
    if (pClient)
    {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

void WebSocketHandler::closed()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    LogHandler::Debug("WEBSOCKET closed: " + pClient->objectName());
}
