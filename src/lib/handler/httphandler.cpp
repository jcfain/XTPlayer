#include "httphandler.h"

HttpHandler::HttpHandler(VideoHandler* videoHandler, QObject *parent):
    HttpRequestHandler(parent)
{
    _videoHandler = videoHandler;
    config.port = SettingsHandler::getHTTPPort();
    config.requestTimeout = 20;
    if(LogHandler::getUserDebug())
        config.verbosity = HttpServerConfig::Verbose::All;
    config.maxMultipartSize = 512 * 1024 * 1024;
//    config.errorDocumentMap[HttpStatus::NotFound] = "data/404_2.html";
//    config.errorDocumentMap[HttpStatus::InternalServerError] = "data/404_2.html";
//    config.errorDocumentMap[HttpStatus::BadGateway] = "data/404_2.html";
    _server = new HttpServer(config, this);
    _server->listen();

    //router.addRoute("GET", "^/videos/[\\w,\\s-]+\\.[A-Za-z]{3}$", this, &HttpHandler::handleRoot);
    router.addRoute("GET", "^/media", this, &HttpHandler::handleVideoList);
    router.addRoute("GET", "^/thumb/.*$", this, &HttpHandler::handleThumbFile);
    router.addRoute("GET", "^/funscript/(.*\\.((funscript)$))?[.]*$", this, &HttpHandler::handleFunscriptFile);
    QString extensions;
    extensions += _videoHandler->getVideoExtensions().join("|");
    extensions += "|";
    extensions += _videoHandler->getAudioExtensions().join("|");
    router.addRoute("GET", "^/video/(.*\\.(("+extensions+")$))?[.]*$", this, &HttpHandler::handleVideoStream);
    router.addRoute("GET", "^/deotest", this, &HttpHandler::handleDeo);
    router.addRoute("GET", "^/settings", this, &HttpHandler::handleSettings);
    router.addRoute("POST", "^/xtpweb", this, &HttpHandler::handleWebTimeUpdate);
//    router.addRoute("GET", "^/users/(\\w*)/?$", this, &HttpHandler::handleGetUsername);
//    router.addRoute({"GET", "POST"}, "^/formTest/?$", this, &HttpHandler::handleFormTest);
//    router.addRoute("GET", "^/fileTest/(\\d*)/?$", this, &HttpHandler::handleFileTest);
//    router.addRoute("GET", "^/errorTest/(\\d*)/?$", this, &HttpHandler::handleErrorTest);
//    router.addRoute("GET", "^/asyncTest/(\\d*)/?$", this, &HttpHandler::handleAsyncTest);
}

HttpHandler::~HttpHandler()
{
    delete _server;
}

HttpPromise HttpHandler::handleWebTimeUpdate(HttpDataPtr data)
{
    auto body = data->request->body();
    LogHandler::Debug("HTTP time sync update: "+QString(body));
    emit readyRead(body);
    data->response->setStatus(HttpStatus::Ok);
    return HttpPromise::resolve(data);
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

    auto path = data->request->uri().path();
    auto root = SettingsHandler::getHttpServerRoot();
    if(path == "/") {
        LogHandler::Debug("Sending root index.html");
        if(!QFileInfo(root+"/index.html").exists())
        {
            LogHandler::Debug("file does not exist: "+root+"/index.html");
            data->response->setStatus(HttpStatus::BadRequest);
        }
        else
        {
            data->response->sendFile(root+"/index.html");
            data->response->setStatus(HttpStatus::Ok);
        }
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

HttpPromise HttpHandler::handleSettings(HttpDataPtr data) {
    QJsonObject root;
    QJsonObject availableAxisJson;
    auto availableAxis = SettingsHandler::getAvailableAxis();
    foreach(auto channel, availableAxis->keys()) {
        QJsonObject value;
        if(availableAxis->value(channel).Type != AxisType::HalfRange && availableAxis->value(channel).Type != AxisType::None)
        {
            value["axisName"] = availableAxis->value(channel).AxisName;
            value["channel"] = availableAxis->value(channel).Channel;
            value["damperEnabled"] = availableAxis->value(channel).DamperEnabled;
            value["damperValue"] = availableAxis->value(channel).DamperValue;
            value["dimension"] = (int)availableAxis->value(channel).Dimension;
            value["friendlyName"] = availableAxis->value(channel).FriendlyName;
            value["inverted"] = availableAxis->value(channel).Inverted;
            value["linkToRelatedMFS"] = availableAxis->value(channel).LinkToRelatedMFS;
            value["max"] = availableAxis->value(channel).Max;
            value["mid"] = availableAxis->value(channel).Mid;
            value["min"] = availableAxis->value(channel).Min;
            value["multiplierEnabled"] = availableAxis->value(channel).MultiplierEnabled;
            value["multiplierValue"] = availableAxis->value(channel).MultiplierValue;
            value["relatedChannel"] = availableAxis->value(channel).RelatedChannel;
            value["trackName"] = availableAxis->value(channel).TrackName;
            value["type"] = (int)availableAxis->value(channel).Type;
            value["userMax"] = availableAxis->value(channel).UserMax;
            value["userMid"] = availableAxis->value(channel).UserMid;
            value["userMin"] = availableAxis->value(channel).UserMin;
            availableAxisJson[channel] = value;
        }
    }
    root["availableAxis"] = availableAxisJson;
    data->response->setStatus(HttpStatus::Ok, QJsonDocument(root));
    return HttpPromise::resolve(data);
}

HttpPromise HttpHandler::handleVideoList(HttpDataPtr data)
{
    QJsonArray media;
    QString hostAddress = "http://" + data->request->headerDefault("Host", "") + "/";
    foreach(auto widgetItem, _cachedLibraryItems)
    {
        QJsonObject object;
        auto item = widgetItem->getLibraryListItem();
        if(item.type == LibraryListItemType::PlaylistInternal || item.type == LibraryListItemType::FunscriptType)
            continue;
        media.append(createMediaObject(item, false, widgetItem->isMFS(), hostAddress));
    }

    foreach(auto widgetItem, _vrLibraryItems)
    {
        QJsonObject object;
        auto item = widgetItem->getLibraryListItem();
        if(item.type == LibraryListItemType::PlaylistInternal || item.type == LibraryListItemType::FunscriptType)
            continue;
        media.append(createMediaObject(item, true, widgetItem->isMFS(), hostAddress));
    }

    data->response->setStatus(HttpStatus::Ok, QJsonDocument(media));
    return HttpPromise::resolve(data);
}

QJsonObject HttpHandler::createMediaObject(LibraryListItem item, bool stereoscopic, bool isMFS, QString hostAddress)
{
    //VideoFormat videoFormat;
    QJsonObject object;
    object["name"] = item.nameNoExtension;
    QString path = item.path.replace(SettingsHandler::getSelectedLibrary(), "");
    QString relativePath = item.path.replace(SettingsHandler::getSelectedLibrary() +"/", "");
    object["path"] = hostAddress + "video/" + QString(QUrl::toPercentEncoding(relativePath));
    QString scriptNoExtensionRelativePath = item.scriptNoExtension.replace(SettingsHandler::getSelectedLibrary(), "");
    object["scriptNoExtensionRelativePath"] = "funscript/" + QString(QUrl::toPercentEncoding(scriptNoExtensionRelativePath));
    object["relativePath"] = "/" + QString(QUrl::toPercentEncoding(relativePath));
    QString thumbFile = item.thumbFile.replace(SettingsHandler::getSelectedThumbsDir(), "");
    QString relativeThumb = thumbFile;
    object["thumb"] = hostAddress + "thumb/" + QString(QUrl::toPercentEncoding(relativeThumb));
    object["relativeThumb"] = QString(QUrl::toPercentEncoding(relativeThumb));
    object["thumbSize"] = SettingsHandler::getThumbSize();
    object["type"] = item.type;
    object["duration"] = QJsonValue::fromVariant(item.duration);
    object["modifiedDate"] = item.modifiedDate.toString(Qt::DateFormat::ISODate);
    object["isStereoscopic"] = getStereoMode(item.path) != "off" || stereoscopic; //videoFormat.is3D((SettingsHandler::getSelectedLibrary() + item.path).toLocal8Bit().data()) == VideoFormatResultCode::E_Found3D;
    object["isMFS"] = isMFS;
    object["hasScript"] = !item.script.isEmpty() || !item.zipFile.isEmpty();

    return object;
}
#include <QHostInfo>
HttpPromise HttpHandler::handleDeo(HttpDataPtr data)
{
    QString hostAddress = "http://" + data->request->headerDefault("Host", "") + "/";
    QJsonObject root;
    QJsonArray scenes;
    QJsonObject library;
    library["name"] = "XTP Library";
    QJsonArray list;

    foreach(auto widgetItem, _cachedLibraryItems)
    {
        QJsonObject object;
        auto item = widgetItem->getLibraryListItem();
        if(item.type == LibraryListItemType::PlaylistInternal || item.type == LibraryListItemType::FunscriptType)
            continue;
        list.append(createDeoObject(item, hostAddress));
    }

    foreach(auto widgetItem, _vrLibraryItems)
    {
        QJsonObject object;
        auto item = widgetItem->getLibraryListItem();
        if(item.type == LibraryListItemType::PlaylistInternal || item.type == LibraryListItemType::FunscriptType)
            continue;
        list.append(createDeoObject(item, hostAddress));
    }

    library["list"] = list;
    scenes.append(library);
    root["scenes"] = scenes;
    data->response->setStatus(HttpStatus::Ok, QJsonDocument(root));
    return HttpPromise::resolve(data);
}

QJsonObject HttpHandler::createDeoObject(LibraryListItem item, QString hostAddress)
{
    QJsonObject root;
    QJsonArray encodings;
    QJsonObject encodingsObj;
    QJsonArray videoSources;
    QJsonObject videoSource;
    QString relativePath = item.path.replace(SettingsHandler::getSelectedLibrary(), "");
//    videoSource["resolution"] = 1080;
//    videoSource["url"] = hostAddress + "video" + QString(QUrl::toPercentEncoding(relativePath));
//    videoSources.append(videoSource);
//    //encodingsObj["name"] = "h264";
//    encodingsObj["videoSources"] = videoSources;
//    encodings.append(encodingsObj);
//    root["encodings"] = encodings;

    root["title"] = item.nameNoExtension;
    root["video_url"] = hostAddress + "video/" + relativePath;
    //root["id"] = item.nameNoExtension;
    root["videoLength"] = (int)(item.duration / 1000);
    root["is3d"] = true;
    root["screenType"] = getScreenType(item.path);
    //fisheye" - 180 degrees fisheye mesh, mkx200, "mkx200" - 200 degrees fisheye mesh
    root["stereoMode"] = getStereoMode(item.path);
//    root["skipIntro"] = 0;
    QString relativeThumb = item.thumbFile.isEmpty() ? "://images/icons/error.png" : item.thumbFile.replace(SettingsHandler::getSelectedThumbsDir(), "");
    root["thumbnailUrl"] = hostAddress + "thumb/" + relativeThumb;

    return root;
}

HttpPromise HttpHandler::handleFunscriptFile(HttpDataPtr data)
{
    auto match = data->state["match"].value<QRegularExpressionMatch>();
    QString parameter = match.captured();
    QString funscriptName = parameter.remove("/funscript/");
    QString filePath = SettingsHandler::getSelectedLibrary() + funscriptName;
    if(!QFile(filePath).exists())
    {
        data->response->setStatus(HttpStatus::NotFound);
        return HttpPromise::resolve(data);
    }
    data->response->sendFile(filePath, "text/json");
    data->response->setStatus(HttpStatus::Ok);
    return HttpPromise::resolve(data);
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
    return HttpPromise::resolve(data).then([this](HttpDataPtr data) {
        QElapsedTimer timer;
        LogHandler::Debug("Enter Video stream");
        timer.start();
        auto match = data->state["match"].value<QRegularExpressionMatch>();
        QString parameter = match.captured();
        QString mediaName = parameter.remove("/video/");
        LogHandler::Debug("Looking for media in library: " + mediaName);
        QString filename = SettingsHandler::getSelectedLibrary() + "/" + mediaName;
        //filename = _videoHandler->transcode(filename);
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly))
        {
            LogHandler::Error(QString("Unable to open file to be sent (%1): %2").arg(filename).arg(file.errorString()));
            data->response->setStatus(HttpStatus::Forbidden);
            return data;
        }
        qint64 bytesAvailable = file.bytesAvailable();

        QString range;
        data->request->header<QString>("range", &range);
        LogHandler::Debug("Requested range: "+range);
        QStringList rangeKeyValue = range.split('=');
        qint64 startByte = 0;
        qint64 endByte = 0;
        if(rangeKeyValue.length() > 1)
        {
            QStringList rangeEnds = rangeKeyValue[1].split('-');
            if(rangeEnds.length() > 0)
            {
                startByte = rangeEnds[0].toLongLong();
                if(rangeEnds.length() > 1)
                    endByte = rangeEnds[1].toLongLong();
            }

        }
        qint64 chunkSize = SettingsHandler::getHTTPChunkSize();
        if(!endByte)
            endByte = startByte + chunkSize;
        if(endByte >= bytesAvailable)
            endByte = bytesAvailable;
        if(startByte < endByte)
        {
            QString requestBytes = "bytes " + QString::number(startByte) + "-" + QString::number(endByte) + "/" + QString::number(bytesAvailable +1);
            LogHandler::Debug("Request bytes: "+requestBytes);
            if(startByte)
                file.skip(startByte);
            LogHandler::Debug("Video stream read start: "+ QString::number(timer.elapsed()));
            QByteArray* byteArray = new QByteArray(file.read(chunkSize));
            LogHandler::Debug("Video stream read end: "+ QString::number(timer.elapsed()));
            QBuffer buffer(byteArray);
            //LogHandler::Debug("Chunk bytes: "+ QString::number(buffer.bytesAvailable()));
            //LogHandler::Debug("Video stream open buffer: "+ QString::number(timer.elapsed()));
            if (!buffer.open(QIODevice::ReadOnly))
            {
                LogHandler::Error(QString("Unable to open buffer to be sent (%1): %2").arg(filename).arg(file.errorString()));

                data->response->setStatus(HttpStatus::Forbidden);
                delete byteArray;
                file.close();
                return data;
            }

            data->response->setStatus(HttpStatus::PartialContent);
            qint64 contentLength = (endByte - startByte) + 1;
            //LogHandler::Debug("Start bytes: " + QString::number(startByte));
            //LogHandler::Debug("End bytes: " + QString::number(endByte));
            //LogHandler::Debug("Content length: " + QString::number(contentLength));
            data->response->setHeader("Accept-Ranges", "bytes");
            data->response->setHeader("Content-Range", requestBytes);
            data->response->setHeader("Content-Length", contentLength);
            //LogHandler::Debug("Video stream send chunk: "+ QString::number(timer.elapsed()));
            QString mimeType = mimeDatabase.mimeTypeForFile(filename, QMimeDatabase::MatchExtension).name();
            data->response->sendFile(&buffer, mimeType);
            LogHandler::Debug("Video stream send chunk finish: "+ QString::number(timer.elapsed()));
            delete byteArray;
            file.close();
            return data;
        }
        else
            data->response->setStatus(HttpStatus::Ok);
        //LogHandler::Debug("Video stream resolve: "+ QString::number(timer.elapsed()));
        return data;
    });
}

QString HttpHandler::getScreenType(QString mediaPath)
{
    if(mediaPath.contains("360", Qt::CaseSensitivity::CaseInsensitive))
        return "360";
    if(mediaPath.contains("180", Qt::CaseSensitivity::CaseInsensitive))
        return "180";
    if(mediaPath.contains("fisheye", Qt::CaseSensitivity::CaseInsensitive))
        return "fisheye";
    if(mediaPath.contains("mkx200", Qt::CaseSensitivity::CaseInsensitive))
        return "mkx200";
    if(mediaPath.contains("vrca220", Qt::CaseSensitivity::CaseInsensitive))
        return "vrca220";
    return "flat";
}

QString HttpHandler::getStereoMode(QString mediaPath)
{
    if(mediaPath.contains("tb", Qt::CaseSensitivity::CaseInsensitive))
        return "TB";
    if(mediaPath.contains("sbs", Qt::CaseSensitivity::CaseInsensitive))
        return "SBS";
    if(mediaPath.contains("3DH", Qt::CaseSensitivity::CaseInsensitive))
        return "3DH";
    if(mediaPath.contains(" lr ", Qt::CaseSensitivity::CaseInsensitive) || mediaPath.contains("_lr", Qt::CaseSensitivity::CaseInsensitive))
        return "LR";
    if(mediaPath.contains("OverUnder", Qt::CaseSensitivity::CaseInsensitive))
        return "OverUnder";
    if(mediaPath.contains("3DV", Qt::CaseSensitivity::CaseInsensitive))
        return "3DV";
    return "off";
}
