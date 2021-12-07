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
public slots:
    void on_tCodeDeviceConnection_StateChange(ConnectionChangedSignal status);

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
    void setLibraryLoaded(bool loaded, QList<LibraryListWidgetItem*> cachedLibraryItems, QList<LibraryListWidgetItem*> vrLibraryItems);

private:
    HttpServerConfig config;
    HttpRequestRouter router;
    HttpServer* _server;
    QMimeDatabase mimeDatabase;

    ConnectionChangedSignal _tcodeDeviceStatus = {DeviceType::Serial, ConnectionStatus::Disconnected, "Disconnected"};
    bool _libraryLoaded = false;
    QList<LibraryListWidgetItem*> _cachedLibraryItems;
    QList<LibraryListWidgetItem*> _vrLibraryItems;

    QJsonObject createMediaObject(LibraryListItem libraryListItem, bool stereoscopic, bool isMFS, QString hostAddress);
    QJsonObject createDeoObject(LibraryListItem libraryListItem, QString hostAddress);
    QString getScreenType(QString mediaPath);
    QString getStereoMode(QString mediaPath);
};

#endif // HTTPHANDLER_H
