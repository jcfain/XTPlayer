#include "httphandler.h"

HttpHandler::HttpHandler(VideoHandler* videoHandler, QObject *parent):
    HttpRequestHandler(parent)
{
    _videoHandler = videoHandler;
    config.port = 80;
    config.requestTimeout = 20;
    config.verbosity = HttpServerConfig::Verbose::All;
    config.maxMultipartSize = 512 * 1024 * 1024;
    config.errorDocumentMap[HttpStatus::NotFound] = "data/404_2.html";
    config.errorDocumentMap[HttpStatus::InternalServerError] = "data/404_2.html";
    config.errorDocumentMap[HttpStatus::BadGateway] = "data/404_2.html";
    _server = new HttpServer(config, this);
    _server->listen();


    //router.addRoute("GET", "^/videos/[\\w,\\s-]+\\.[A-Za-z]{3}$", this, &HttpHandler::handleRoot);
    router.addRoute("GET", "^/media", this, &HttpHandler::handleVideoList);
    router.addRoute("GET", "^/thumb/.*$", this, &HttpHandler::handleThumbFile);
    router.addRoute("GET", "^/video/.*$", this, &HttpHandler::handleVideoStream);
//    router.addRoute("GET", "^/users/(\\w*)/?$", this, &HttpHandler::handleGetUsername);
//    router.addRoute({"GET", "POST"}, "^/gzipTest/?$", this, &HttpHandler::handleGzipTest);
//    router.addRoute({"GET", "POST"}, "^/formTest/?$", this, &HttpHandler::handleFormTest);
//    router.addRoute("GET", "^/fileTest/(\\d*)/?$", this, &HttpHandler::handleFileTest);
//    router.addRoute("GET", "^/errorTest/(\\d*)/?$", this, &HttpHandler::handleErrorTest);
//    router.addRoute("GET", "^/asyncTest/(\\d*)/?$", this, &HttpHandler::handleAsyncTest);
}

HttpHandler::~HttpHandler()
{

}

void HttpHandler::setLibraryLoaded(bool loaded, QList<LibraryListWidgetItem*> cachedLibraryItems, QList<LibraryListWidgetItem*> vrLibraryItems)
{
    _libraryLoaded = loaded;
    _cachedLibraryItems = cachedLibraryItems;
    _vrLibraryItems = vrLibraryItems;
}

HttpPromise HttpHandler::handle(HttpDataPtr data)
{
    bool foundRoute;
    HttpPromise promise = router.route(data, &foundRoute);
    if (foundRoute)
        return promise;
//    QString playerHtml = QStringLiteral(
//                                        "<!doctype html>"
//                                        "<html lang='en'>"
//                                        "   <head>"
//                                        "       <title>DeoVR Example</title>"
//                                        "       <link rel='stylesheet' type='text/css' href='https://s3.deovr.com/version/1/css/styles.css'/>"
//                                        "       <script src='https://s3.deovr.com/version/1/js/bundle.js' async></script>"
//                                        "   </head>"
//                                        "   <body>"
//                                        "       <deo-video format='mono'>"
//                                        "           <source src='http://127.0.0.1/video/Alay720P.mp4' quality='720p'/>"
//                                        "       </deo-video>"
//                                        "   </body>"
//                                        "</html>");
//    if (data->request->mimeType().compare("application/json", Qt::CaseInsensitive) != 0)
//        throw HttpException(HttpStatus::BadRequest, "Request body content type must be application/json");

//    QJsonDocument jsonDocument = data->request->parseJsonBody();
//    if (jsonDocument.isNull())
//        throw HttpException(HttpStatus::BadRequest, "Invalid JSON body");

    //data->response->setStatus(HttpStatus::Ok, playerHtml, "text/html");
    auto path = data->request->uri().path();
    auto root = SettingsHandler::getHttpServerRoot();
    if(path == "/") {
        data->response->sendFile(root+"/index.html");
        data->response->setStatus(HttpStatus::Ok);
    }
    else if(path.contains("favicon.ico"))
    {
        data->response->sendFile("://images/icons/XTP-window-icon.ico", "image/x-icon");
        data->response->setStatus(HttpStatus::Ok);
    }
    else
    {
        QString localPath;
        if(path.startsWith("/:"))
        {
            localPath = path.remove(0,1);
        }
        else
        {
            localPath = root + path;
        }
        QFile file(localPath);
        if(file.exists())
        {
            data->response->sendFile(localPath);
            data->response->setStatus(HttpStatus::Ok);
        }
        else
            data->response->setStatus(HttpStatus::BadRequest);
    }
    return HttpPromise::resolve(data);
}

HttpPromise HttpHandler::handleVideoList(HttpDataPtr data)
{
    QJsonArray media;
    foreach(auto widgetItem, _cachedLibraryItems)
    {
        QJsonObject object;
        auto item = widgetItem->getLibraryListItem();
        if(item.type == LibraryListItemType::PlaylistInternal || item.type == LibraryListItemType::FunscriptType)
            continue;
        media.append(createMediaObject(item, false, widgetItem->isMFS()));
    }

    foreach(auto widgetItem, _vrLibraryItems)
    {
        QJsonObject object;
        auto item = widgetItem->getLibraryListItem();
        if(item.type == LibraryListItemType::PlaylistInternal || item.type == LibraryListItemType::FunscriptType)
            continue;
        media.append(createMediaObject(item, true, widgetItem->isMFS()));
    }

    data->response->setStatus(HttpStatus::Ok, QJsonDocument(media));
    return HttpPromise::resolve(data);
}

QJsonObject HttpHandler::createMediaObject(LibraryListItem item, bool stereoscopic, bool isMFS)
{
    //VideoFormat videoFormat;
    QJsonObject object;
    object["name"] = item.nameNoExtension;
    QString relativePath = item.path.replace(SettingsHandler::getSelectedLibrary(), "");
    object["relativePath"] = QString(QUrl::toPercentEncoding(relativePath));
    QString relativeThumb = item.thumbFile.isEmpty() ? "://images/icons/error.png" : item.thumbFile.replace(SettingsHandler::getSelectedThumbsDir(), "");
    object["relativeThumb"] = QString(QUrl::toPercentEncoding(relativeThumb));
    object["thumbSize"] = SettingsHandler::getThumbSize();
    object["type"] = item.type;
    object["duration"] = QJsonValue::fromVariant(item.duration);
    object["modifiedDate"] = item.modifiedDate.toString(Qt::DateFormat::ISODate);
    object["isStereoscopic"] = stereoscopic; //videoFormat.is3D((SettingsHandler::getSelectedLibrary() + item.path).toLocal8Bit().data()) == VideoFormatResultCode::E_Found3D;
    object["isMFS"] = isMFS;
    object["hasScript"] = !item.script.isEmpty() || !item.zipFile.isEmpty();
    return object;
}

HttpPromise HttpHandler::handleThumbFile(HttpDataPtr data)
{
    auto match = data->state["match"].value<QRegularExpressionMatch>();
    QString parameter = match.captured();
    QString thumbName = parameter.remove("/thumb/");
    if(thumbName.startsWith(":"))
        data->response->sendFile(thumbName);
    else
        data->response->sendFile(SettingsHandler::getSelectedThumbsDir() + thumbName);
    data->response->setStatus(HttpStatus::Ok);
    return HttpPromise::resolve(data);
}
HttpPromise HttpHandler::handleVideoStream(HttpDataPtr data)
{
    QElapsedTimer timer;
    LogHandler::Debug("Enter Video stream");
    timer.start();
    auto match = data->state["match"].value<QRegularExpressionMatch>();
    QString parameter = match.captured();
    QString mediaName = parameter.remove("/video/");
    LogHandler::Debug("Looking for media in library: " + mediaName);
    // QString filename = "\\\\RASPBERRYPI.local\\STK\\RealTouchScripts\\Alay720P.mp4";
    QString filename = SettingsHandler::getSelectedLibrary() + "/" + mediaName;
    //filename = _videoHandler->transcode(filename);
    QFile file(filename);
    if (!file.exists())
    {
      if (config.verbosity >= HttpServerConfig::Verbose::Warning)
         qWarning().noquote() << QString("File does not exist (%1): %2").arg(filename).arg(file.errorString());

      data->response->setStatus(HttpStatus::NotFound);
      return HttpPromise::resolve(data);
    }
    if (!file.open(QIODevice::ReadOnly))
    {
        if (config.verbosity >= HttpServerConfig::Verbose::Info)
            qInfo().noquote() << QString("Unable to open file to be sent (%1): %2").arg(filename).arg(file.errorString());

        data->response->setStatus(HttpStatus::Forbidden);
        return HttpPromise::resolve(data);
    }

    QString range;
    data->request->header<QString>("range", &range);
    QStringList rangeKeyValue = range.split('=');
    int startByte = 0;
    int endByte = 0;
    if(rangeKeyValue.length() > 1)
    {
        QStringList rangeEnds = rangeKeyValue[1].split('-');
        if(rangeEnds.length() > 0)
        {
            startByte = rangeEnds[0].toInt();
            if(rangeEnds.length() > 1)
                endByte = rangeEnds[1].toInt();
        }

    }
    qint64 chunkSize = SettingsHandler::getHTTPChunkSize();
    if(!endByte)
        endByte = startByte + chunkSize;
    if(endByte >= file.bytesAvailable())
        endByte = file.bytesAvailable();
    if(startByte < endByte)
    {
        QString requestBytes = "bytes " + QString::number(startByte) + "-" + QString::number(endByte) + "/" + QString::number(file.bytesAvailable() +1);
        LogHandler::Debug("Request bytes: "+requestBytes);
        if(startByte)
        {
            int numBytes = file.skip(startByte);
            LogHandler::Debug("Skipped bytes: "+QString::number(numBytes));
        }
        data->response->setStatus(HttpStatus::PartialContent);
        int contentLength = (endByte - startByte) + 1;
        LogHandler::Debug("Start bytes: " + QString::number(startByte));
        LogHandler::Debug("End bytes: " + QString::number(endByte));
        LogHandler::Debug("Content length: " + QString::number(contentLength));

        data->response->setHeader("Accept-Ranges", "bytes");
        data->response->setHeader("Content-Range", requestBytes);
        data->response->setHeader("Content-Length", contentLength);

        QString mimeType = mimeDatabase.mimeTypeForFile(filename, QMimeDatabase::MatchExtension).name();
        LogHandler::Debug("Video stream read: "+ QString::number(timer.elapsed()));
        QByteArray* byteArray = new QByteArray(file.read(chunkSize));
        QBuffer buffer(byteArray);
        LogHandler::Debug("Video stream open buffer: "+ QString::number(timer.elapsed()));
        if (!buffer.open(QIODevice::ReadOnly))
        {
            if (config.verbosity >= HttpServerConfig::Verbose::Info)
                qInfo().noquote() << QString("Unable to open buffer to be sent (%1): %2").arg(filename).arg(file.errorString());

            data->response->setStatus(HttpStatus::Forbidden);
            return HttpPromise::resolve(data);
        }

        LogHandler::Debug("Video stream send chunk: "+ QString::number(timer.elapsed()));
        data->response->sendFile(&buffer, mimeType);
        LogHandler::Debug("Video stream send chunk finish: "+ QString::number(timer.elapsed()));
        delete byteArray;
    }
    else
        data->response->setStatus(HttpStatus::Ok);

    LogHandler::Debug("Video stream resolve: "+ QString::number(timer.elapsed()));
    return HttpPromise::resolve(data);
}
