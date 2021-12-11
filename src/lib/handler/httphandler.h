#ifndef HTTPHANDLER_H
#define HTTPHANDLER_H

#include <QtPromise>
#include <QTimer>
#include <QBuffer>
#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QtConcurrent/QtConcurrent>

#include "settingshandler.h"
#include "websockethandler.h"
#include "httpServer/httpServer.h"
#include "httpServer/httpRequestHandler.h"
#include "httpServer/httpRequestRouter.h"
#include "../struct/librarylistwidgetitem.h"
#include "../struct/ConnectionChangedSignal.h"
#include "../tool/videoformat.h"

class HttpHandler : public HttpRequestHandler
{
    Q_OBJECT
signals:
    void readyRead(QByteArray data);
    void tcode(QString tcode);
    void connectTCodeDevice();
    void connectInputDevice(DeviceType deviceType, bool checked);
public slots:
    void on_DeviceConnection_StateChange(ConnectionChangedSignal status);

public:
    HttpHandler(QObject *parent = nullptr);
    ~HttpHandler();
    HttpPromise handle(HttpDataPtr data);
    HttpPromise handleVideoStream(HttpDataPtr data);
    HttpPromise handleVideoList(HttpDataPtr data);
    HttpPromise handleThumbFile(HttpDataPtr data);
    HttpPromise handleFunscriptFile(HttpDataPtr data);
    HttpPromise handleSettings(HttpDataPtr data);
    HttpPromise handleSettingsUpdate(HttpDataPtr data);
    HttpPromise handleDeviceConnected(HttpDataPtr data);
    HttpPromise handleConnectDevice(HttpDataPtr data);
    HttpPromise handleTCodeIn(HttpDataPtr data);
    HttpPromise handleDeo(HttpDataPtr data);
    HttpPromise handleWebTimeUpdate(HttpDataPtr data);
    void setLibraryLoaded(QList<LibraryListItem27> cachedLibraryItems, QList<LibraryListItem27> vrLibraryItems);
    void setLibraryLoading();
    void sendLibraryLoadingStatus(QString message);

    void sendWebSocketTextMessage(QString command, QString message = nullptr);
    void on_webSocketClient_Connected(QWebSocket* client);

private:
    HttpServerConfig config;
    HttpRequestRouter router;
    HttpServer* _server;
    QMimeDatabase mimeDatabase;
    WebSocketHandler* _webSocketHandler;
    bool _libraryLoaded = false;
    QString _libraryLoadingStatus = "Loading...";
    QList<LibraryListItem27> _cachedLibraryItems;
    QList<LibraryListItem27> _vrLibraryItems;

    QJsonObject createMediaObject(LibraryListItem27 libraryListItem, bool stereoscopic, QString hostAddress);
    QJsonObject createDeoObject(LibraryListItem27 libraryListItem, QString hostAddress);
    QString getScreenType(QString mediaPath);
    QString getStereoMode(QString mediaPath);
};

#endif // HTTPHANDLER_H
