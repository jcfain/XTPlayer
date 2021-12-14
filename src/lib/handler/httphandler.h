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
#include "medialibraryhandler.h"
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
    HttpHandler(MediaLibraryHandler* mediaLibraryHandler, QObject *parent = nullptr);
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

    void sendWebSocketTextMessage(QString command, QString message = nullptr);

private:
    HttpServerConfig config;
    HttpRequestRouter router;
    HttpServer* _server;
    QMimeDatabase mimeDatabase;
    WebSocketHandler* _webSocketHandler;
    MediaLibraryHandler* _mediaLibraryHandler;
    bool _libraryLoaded = false;
    QString _libraryLoadingStatus = "Loading...";
    QMutex _mutex;

    QJsonObject createMediaObject(LibraryListItem27 libraryListItem, bool stereoscopic, QString hostAddress);
    QJsonObject createDeoObject(LibraryListItem27 libraryListItem, QString hostAddress);
    QString getScreenType(QString mediaPath);
    QString getStereoMode(QString mediaPath);
    void on_webSocketClient_Connected(QWebSocket* client);
    void onSetLibraryLoaded();
    void onSetLibraryLoading();
    void onLibraryLoadingStatusChange(QString message);

};

#endif // HTTPHANDLER_H
