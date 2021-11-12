#ifndef HTTPHANDLER_H
#define HTTPHANDLER_H

#include <QtPromise>
#include <QTimer>
#include <QBuffer>
#include <QObject>
#include <QJsonArray>
#include <QJsonObject>#include <QJsonDocument>
#include <QJsonValue>


#include "settingshandler.h"
#include "videohandler.h"
#include "httpServer/httpServer.h"
#include "httpServer/httpRequestHandler.h"
#include "httpServer/httpRequestRouter.h"
#include "../struct/librarylistwidgetitem.h"
#include "../tool/videoformat.h"

class HttpHandler : public HttpRequestHandler
{
signals:
    void streamChange(bool running);
public:
    HttpHandler(VideoHandler* videoHandler, QObject *parent = nullptr);
    ~HttpHandler();
    HttpPromise handle(HttpDataPtr data);
    HttpPromise handleVideoStream(HttpDataPtr data);
    HttpPromise handleVideoList(HttpDataPtr data);
    HttpPromise handleThumbFile(HttpDataPtr data);
    HttpPromise handleDeo(HttpDataPtr data);
    void setLibraryLoaded(bool loaded, QList<LibraryListWidgetItem*> cachedLibraryItems, QList<LibraryListWidgetItem*> vrLibraryItems);

private:
    HttpServerConfig config;
    HttpRequestRouter router;
    HttpServer* _server;
    QMimeDatabase mimeDatabase;
    VideoHandler* _videoHandler;

    bool _libraryLoaded = false;
    QList<LibraryListWidgetItem*> _cachedLibraryItems;
    QList<LibraryListWidgetItem*> _vrLibraryItems;

    QJsonObject createMediaObject(LibraryListItem libraryListItem, bool stereoscopic, bool isMFS);
    QJsonObject createDeoObject(LibraryListItem libraryListItem, bool stereoscopic, QString hostAddress);

};

#endif // HTTPHANDLER_H
