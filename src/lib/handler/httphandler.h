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
#include "httpServer/httpServer.h"
#include "httpServer/httpRequestHandler.h"
#include "httpServer/httpRequestRouter.h"
#include "../struct/librarylistwidgetitem.h"

class HttpHandler : public HttpRequestHandler
{
signals:
    void streamChange(bool running);
public:
    HttpHandler(QObject *parent = nullptr);
    ~HttpHandler();
    HttpPromise handle(HttpDataPtr data);
    HttpPromise handleVideoStream(HttpDataPtr data);
    HttpPromise handleVideoList(HttpDataPtr data);
    HttpPromise handleThumbFile(HttpDataPtr data);
    void setLibraryLoaded(bool loaded, QList<LibraryListWidgetItem*> cachedLibraryItems);

private:
    HttpServerConfig config;
    HttpRequestRouter router;
    HttpServer* _server;
    QMimeDatabase mimeDatabase;

    qint64 _chunkSize = 1048576;
    bool _libraryLoaded = false;
    QList<LibraryListWidgetItem*> _cachedLibraryItems;

};

#endif // HTTPHANDLER_H
