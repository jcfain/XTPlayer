#include "medialibraryhandler.h"

MediaLibraryHandler::MediaLibraryHandler(QObject* parent)
    : QObject(parent)
{
//    if(extractor)
//        delete extractor;
//    if(thumbNailPlayer)
//        delete thumbNailPlayer;
    connect(this, &MediaLibraryHandler::prepareLibraryLoad, this, &MediaLibraryHandler::onPrepareLibraryLoad);
    connect(this, &MediaLibraryHandler::libraryLoaded, this, &MediaLibraryHandler::onLibraryLoaded);


}

MediaLibraryHandler::~MediaLibraryHandler()
{
    if(loadingLibraryFuture.isRunning())
    {
        loadingLibraryStop = true;
        loadingLibraryFuture.cancel();
        loadingLibraryFuture.waitForFinished();
    }
    //qDeleteAll(cachedVRItems);
    //cachedVRItems.clear();
}
bool MediaLibraryHandler::isLibraryLoading()
{
    return loadingLibraryFuture.isRunning();
}

void MediaLibraryHandler::stopLibraryLoading()
{
    loadingLibraryStop = true;
    loadingLibraryFuture.cancel();
    loadingLibraryFuture.waitForFinished();
}

void MediaLibraryHandler::onPrepareLibraryLoad()
{
    stopThumbProcess();
    //_xSettings->setLibraryLoaded(false, cachedLibraryItems, cachedVRItems);
    const QMutexLocker locker(&_mutex);
    cachedLibraryItems.clear();
    cachedVRItems.clear();
    libraryItemIDTracker = 1;
}

void MediaLibraryHandler::loadLibraryAsync()
{
    emit libraryLoading();
    QString library = SettingsHandler::getSelectedLibrary();
    QString vrLibrary = SettingsHandler::getVRLibrary();
    if(library.isEmpty() && vrLibrary.isEmpty())
    {
        emit libraryLoadingStatus("No media folder specified");
        emit libraryLoaded();
        return;
    }
    if(!isLibraryLoading())
    {
        emit libraryLoadingStatus(library.isEmpty() ? "Loading VR media..." : "Loading media...");
        loadingLibraryFuture = QtConcurrent::run([this, library, vrLibrary]() {
            on_load_library(library.isEmpty() ? vrLibrary : library, library.isEmpty());
        });
    }
}
void MediaLibraryHandler::on_load_library(QString path, bool vrMode)
{
    if (path.isEmpty())
    {
        return;
    }
    else
    {
        QDir directory(path);
        if(!directory.exists())
        {
            emit libraryNotFound();
            emit libraryLoaded();
            return;
        }
    }
    QString thumbPath = SettingsHandler::getSelectedThumbsDir();
    QDir thumbDir(thumbPath);
    if (!thumbDir.exists())
    {
        thumbDir.mkdir(thumbPath);
    }
//    QStringList videoTypes = QStringList()
//            << "*.mp4"
//            << "*.avi"
//            << "*.mpg"
//            << "*.wmv"
//            << "*.mkv"
//            << "*.webm"
//            << "*.mp2"
//            << "*.mpeg"
//            << "*.mpv"
//            << "*.ogg"
//            << "*.m4p"
//            << "*.m4v"
//            << "*.mov"
//            << "*.qt"
//            << "*.flv"
//            << "*.swf"
//            << "*.avchd";

//    QStringList audioTypes = QStringList()
//            << "*.m4a"
//            << "*.mp3"
//            << "*.aac"
//            << "*.flac"
//            << "*.wav"
//            << "*.wma";
    QStringList playlistTypes = QStringList()
            << "*.m3u";

    QStringList mediaTypes;
    QStringList videoTypes;
    QStringList audioTypes;
    foreach(auto ext, SettingsHandler::getVideoExtensions())
        videoTypes.append("*."+ext);
    foreach(auto ext, SettingsHandler::getAudioExtensions())
        audioTypes.append("*."+ext);
    mediaTypes.append(videoTypes);
    mediaTypes.append(audioTypes);
    QDirIterator library(path, mediaTypes, QDir::Files, QDirIterator::Subdirectories);

    if(!vrMode)
    {
        emit prepareLibraryLoad();
        QThread::sleep(1);

        auto playlists = SettingsHandler::getPlaylists();
        foreach(auto playlist, playlists.keys())
        {
            setupPlaylistItem(playlist);
        }
    }
    QStringList funscriptsWithMedia;
    QList<QString> excludedLibraryPaths = SettingsHandler::getLibraryExclusions();
    while (library.hasNext())
    {
        if(loadingLibraryStop)
        {
            emit libraryLoaded();
            return;
        }
        QFileInfo fileinfo(library.next());
        QString fileDir = fileinfo.dir().path();
        bool isExcluded = false;
        foreach(QString dir, excludedLibraryPaths)
        {
            if(dir != path && (fileDir.startsWith(dir, Qt::CaseInsensitive)))
                isExcluded = true;
        }
        if (isExcluded)
            continue;
        QString videoPath = fileinfo.filePath();
        QString videoPathTemp = fileinfo.filePath();
        QString fileName = fileinfo.fileName();
        QString fileNameTemp = fileinfo.fileName();
        QString fileNameNoExtension = fileNameTemp.remove(fileNameTemp.lastIndexOf('.'), fileNameTemp.length() -  1);
        QString scriptFile = fileNameNoExtension + ".funscript";
        QString scriptPath;
        QString scriptNoExtension = videoPathTemp.remove(videoPathTemp.lastIndexOf('.'), videoPathTemp.length() - 1);
        fileNameTemp = fileinfo.fileName();
        QString mediaExtension = "*" + fileNameTemp.remove(0, fileNameTemp.length() - (fileNameTemp.length() - fileNameTemp.lastIndexOf('.')));

        if (SettingsHandler::getSelectedFunscriptLibrary() == Q_NULLPTR)
        {
            scriptPath = scriptNoExtension + ".funscript";
        }
        else
        {
            scriptNoExtension = SettingsHandler::getSelectedFunscriptLibrary() + QDir::separator() + fileNameNoExtension;
            scriptPath = SettingsHandler::getSelectedFunscriptLibrary() + QDir::separator() + scriptFile;
        }

        QFile fpath(scriptPath);
        if (!fpath.exists())
        {
            scriptPath = nullptr;
        }

        LibraryListItemType libratyItemType = LibraryListItemType::Video;
        QFileInfo scriptZip(scriptNoExtension + ".zip");
        QString zipFile;
        if(scriptZip.exists())
            zipFile = scriptNoExtension + ".zip";
        if(audioTypes.contains(mediaExtension))
        {
            libratyItemType = LibraryListItemType::Audio;
        }
        LibraryListItem27 item;
        item.type = libratyItemType,
        item.path = videoPath, // path
        item.name = fileName, // name
        item.nameNoExtension = fileNameNoExtension, //nameNoExtension
        item.script = scriptPath, // script
        item.scriptNoExtension = scriptNoExtension,
        item.mediaExtension = mediaExtension,
        item.thumbFile = nullptr,
        item.zipFile = zipFile,
        item.modifiedDate = fileinfo.birthTime().date(),
        item.duration = 0;
        assignID(item);
        setLiveProperties(item);

        if(!vrMode && !scriptPath.isEmpty())
            funscriptsWithMedia.append(scriptPath);
        if(!vrMode && !zipFile.isEmpty())
            funscriptsWithMedia.append(zipFile);

        onLibraryItemFound(item, vrMode);
        emit libraryItemFound(item, vrMode);
    }

    if(!vrMode && !SettingsHandler::getHideStandAloneFunscriptsInLibrary())
    {
        emit libraryLoadingStatus("Searching for lone funscripts...");
        QStringList funscriptTypes = QStringList()
                << "*.funscript"
                << "*.zip";
        mediaTypes.clear();
        mediaTypes.append(funscriptTypes);
        QDirIterator funscripts(path, mediaTypes, QDir::Files, QDirIterator::Subdirectories);
        auto availibleAxis = SettingsHandler::getAvailableAxis();
        while (funscripts.hasNext())
        {
            if(loadingLibraryStop)
            {
                emit libraryLoaded();
                return;
            }
            QFileInfo fileinfo(funscripts.next());
            QString fileName = fileinfo.fileName();
            QString fileNameTemp = fileinfo.fileName();
            QString scriptPath = fileinfo.filePath();
            QString scriptPathTemp = fileinfo.filePath();
            QString scriptNoExtension = scriptPathTemp.remove(scriptPathTemp.lastIndexOf('.'), scriptPathTemp.length() - 1);
            QString scriptNoExtensionTemp = QString(scriptNoExtension);
            if(funscriptsWithMedia.contains(scriptPath, Qt::CaseSensitivity::CaseInsensitive))
                continue;

            QString scriptMFSExt = scriptNoExtensionTemp.remove(0, scriptNoExtensionTemp.length() - (scriptNoExtensionTemp.length() - scriptNoExtensionTemp.lastIndexOf('.')));
            bool isMfs = false;
            foreach(auto axisName, availibleAxis->keys())
            {
                auto track = availibleAxis->value(axisName);
                if("."+track.TrackName == scriptMFSExt)
                {
                    isMfs = true;
                    break;
                }
            }
            if(isMfs)
                continue;

            QString fileDir = fileinfo.dir().path();
            bool isExcluded = false;
            foreach(QString dir, excludedLibraryPaths)
            {
                if(dir != path && (fileDir.startsWith(dir, Qt::CaseInsensitive)))
                    isExcluded = true;
            }
            if (isExcluded)
                continue;
            QString zipFile = nullptr;
            if(scriptPath.endsWith(".zip", Qt::CaseInsensitive))
            {
                zipFile = scriptPath;
            }
            fileNameTemp = fileinfo.fileName();
            QString fileNameNoExtension = fileNameTemp.remove(fileNameTemp.lastIndexOf('.'), fileNameTemp.length() -  1);
            fileNameTemp = fileinfo.fileName();
            QString mediaExtension = "*" + fileNameTemp.remove(0, fileNameTemp.length() - (fileNameTemp.length() - fileNameTemp.lastIndexOf('.')));

            LibraryListItem27 item;
            item.type = LibraryListItemType::FunscriptType,
            item.path = scriptPath, // path
            item.name = fileName, // name
            item.nameNoExtension = fileNameNoExtension, //nameNoExtension
            item.script = scriptPath, // script
            item.scriptNoExtension = scriptNoExtension,
            item.mediaExtension = mediaExtension,
            item.zipFile = zipFile,
            item.modifiedDate = fileinfo.birthTime().date(),
            item.duration = 0;
            assignID(item);
            setLiveProperties(item);
            onLibraryItemFound(item, vrMode);
            emit libraryItemFound(item, false);
        }
    }
    if(vrMode)
        emit libraryLoaded();
    else
    {
        QString vrLibrary = SettingsHandler::getVRLibrary();
        if(vrLibrary.isEmpty())
        {
            emit libraryLoaded();
            return;
        }
        QFileInfo vrLibraryInfo(vrLibrary);
        if(!vrLibraryInfo.exists())
        {
            emit libraryLoaded();
            return;
        }
        emit libraryLoadingStatus("Loading VR library...");
        on_load_library(SettingsHandler::getVRLibrary(), true);
    }

}

void MediaLibraryHandler::onLibraryItemFound(LibraryListItem27 item, bool vrMode)
{
    const QMutexLocker locker(&_mutex);
    if(vrMode)
        cachedVRItems.push_back(item);
    else
        cachedLibraryItems.push_back(item);
}
LibraryListItem27 MediaLibraryHandler::createLibraryListItemFromFunscript(QString funscript)
{

    QFileInfo fileinfo(funscript);
    QString fileName = fileinfo.fileName();
    QString fileNameTemp = fileinfo.fileName();
    QString scriptPath = fileinfo.filePath();
    QString scriptPathTemp = fileinfo.filePath();
    QString scriptNoExtension = scriptPathTemp.remove(scriptPathTemp.lastIndexOf('.'), scriptPathTemp.length() - 1);
    QString scriptNoExtensionTemp = QString(scriptNoExtension);
    QString fileDir = fileinfo.dir().path();
    QString zipFile = nullptr;
    if(scriptPath.endsWith(".zip", Qt::CaseInsensitive))
    {
        zipFile = scriptPath;
        scriptPath = nullptr;
    }
    fileNameTemp = fileinfo.fileName();
    QString fileNameNoExtension = fileNameTemp.remove(fileNameTemp.lastIndexOf('.'), fileNameTemp.length() -  1);
    fileNameTemp = fileinfo.fileName();
    QString mediaExtension = "*" + fileNameTemp.remove(0, fileNameTemp.length() - (fileNameTemp.length() - fileNameTemp.lastIndexOf('.')));
    LibraryListItem27 item;
    item.type = LibraryListItemType::FunscriptType;
    item.path = scriptPath; // path
    item.name = fileName; // name
    item.nameNoExtension = fileNameNoExtension; //nameNoExtension
    item.script = scriptPath; // script
    item.scriptNoExtension = fileNameNoExtension;
    item.mediaExtension = mediaExtension;
    item.zipFile = zipFile;
    item.modifiedDate = fileinfo.birthTime().date();
    item.duration = 0;
    assignID(item);
    setLiveProperties(item);

    const QMutexLocker locker(&_mutex);
    cachedLibraryItems.push_back(item);
    return item;
}

//QString MediaLibraryHandler::getThumbPath(LibraryListItem27 libraryListItem)
//{
//    if(libraryListItem.type == LibraryListItemType::Audio)
//    {
//        return "://images/icons/audio.png";
//    }
//    else if(libraryListItem.type == LibraryListItemType::PlaylistInternal)
//    {
//        return "://images/icons/playlist.png";
//    }
//    else if(libraryListItem.type == LibraryListItemType::FunscriptType)
//    {
//        return "://images/icons/funscript.png";
//    }
//    QStringList imageExtensions;
//    imageExtensions << ".jpg" << ".jpeg" << ".png";
//    QFileInfo mediaInfo(libraryListItem.path);
//    foreach(QString ext, imageExtensions) {
//        QString filepath = mediaInfo.absolutePath() + QDir::separator() + libraryListItem.nameNoExtension + ext;
//        if(QFile(filepath).exists())
//            return filepath;
//    }
//    if(SettingsHandler::getUseMediaDirForThumbs())
//        return QFileInfo(libraryListItem.path).absoluteDir().path() + libraryListItem.nameNoExtension + ".jpg";

//    return SettingsHandler::getSelectedThumbsDir() + libraryListItem.name + ".jpg";
//}

void MediaLibraryHandler::onLibraryLoaded()
{
    startThumbProcess();
    //_xSettings->setLibraryLoaded(true, cachedLibraryItems, cachedVRItems);

}

void MediaLibraryHandler::startThumbProcess(bool vrMode)
{
    stopThumbProcess();
    thumbProcessIsRunning = true;
    extractor = new VideoFrameExtractor(this);
    thumbNailPlayer = new AVPlayer(this);
    thumbNailPlayer->setInterruptOnTimeout(true);
    thumbNailPlayer->setInterruptTimeout(10000);
    thumbNailPlayer->setAsyncLoad(true);
    extractor->setAsync(true);
    saveNewThumbs(vrMode);
    emit thumbProcessBegin();
}

void MediaLibraryHandler::stopThumbProcess()
{
    if(thumbProcessIsRunning)
    {
        disconnect(extractor, &QtAV::VideoFrameExtractor::frameExtracted,  nullptr, nullptr);
        disconnect(extractor, &QtAV::VideoFrameExtractor::error,  nullptr, nullptr);
        disconnect(thumbNailPlayer, &AVPlayer::loaded,  nullptr, nullptr);
        disconnect(thumbNailPlayer, &AVPlayer::error,  nullptr, nullptr);
        thumbNailSearchIterator = 0;
        thumbProcessIsRunning = false;
        delete extractor;
        delete thumbNailPlayer;
    }
}

void MediaLibraryHandler::saveSingleThumb(LibraryListItem27 item, qint64 position)
{
    if(!thumbProcessIsRunning && !item.thumbFile.contains(".lock."))
    {
        extractor = new VideoFrameExtractor(this);
        thumbNailPlayer = new AVPlayer(this);
        thumbNailPlayer->setInterruptOnTimeout(true);
        thumbNailPlayer->setInterruptTimeout(10000);
        thumbNailPlayer->setAsyncLoad(true);
        extractor->setAsync(true);
        saveThumb(item, position);
    }
}

void MediaLibraryHandler::saveNewThumbs(bool vrMode)
{
    if (thumbProcessIsRunning && thumbNailSearchIterator < (vrMode ? cachedVRItems.count() : cachedLibraryItems.count()))
    {
        LibraryListItem27 item = vrMode ? cachedVRItems.at(thumbNailSearchIterator) : cachedLibraryItems.at(thumbNailSearchIterator);
        thumbNailSearchIterator++;
        QFileInfo thumbInfo(item.thumbFile);
        if (item.type == LibraryListItemType::Video && !thumbInfo.exists())
        {
            disconnect(extractor, nullptr,  nullptr, nullptr);
            disconnect(thumbNailPlayer, nullptr,  nullptr, nullptr);
            saveThumb(item, -1, vrMode);
        }
        else
        {
            saveNewThumbs(vrMode);
        }
    }
    else
    {
        stopThumbProcess();
        if(!vrMode)
            startThumbProcess(true);
        else
            emit thumbProcessEnd();
    }
}
void MediaLibraryHandler::saveThumb(LibraryListItem27 cachedListItem, qint64 position, bool vrMode)
{

    QString videoFile = cachedListItem.path;
    //://images/icons/loading_current.png
    if(thumbProcessIsRunning && (cachedListItem.type == LibraryListItemType::Audio || cachedListItem.type == LibraryListItemType::FunscriptType || cachedListItem.thumbFile.contains(".lock.")))
    {
        saveNewThumbs(vrMode);
    }
    else
    {
        if(!vrMode)
            emit saveNewThumbLoading(cachedListItem);
        // Get the duration and randomize the position with in the video.
        connect(thumbNailPlayer,
           &AVPlayer::loaded,
           thumbNailPlayer,
           [this, videoFile, position]()
            {
               LogHandler::Debug(tr("Loaded video for thumb duration: ") + QString::number(thumbNailPlayer->duration()));
               qint64 randomPosition = position > 0 ? position : XMath::rand((qint64)1, thumbNailPlayer->duration());

               disconnect(thumbNailPlayer, &AVPlayer::loaded,  nullptr, nullptr);
               disconnect(thumbNailPlayer, &AVPlayer::error,  nullptr, nullptr);
               extractor->setSource(videoFile);
               extractor->setPosition(randomPosition);
            });


        connect(thumbNailPlayer,
           &AVPlayer::error,
           thumbNailPlayer,
           [this, cachedListItem, videoFile, vrMode](QtAV::AVError er)
            {
                QString error = tr("Video load error from: ") + videoFile + tr(" Error: ") + er.ffmpegErrorString();
               LogHandler::Error(error);
               onSaveThumbError(cachedListItem, vrMode, error);
            });


        connect(extractor,
           &QtAV::VideoFrameExtractor::frameExtracted,
           extractor,
           [this, cachedListItem, videoFile, vrMode](const QtAV::VideoFrame& frame)
            {
                if(frame.isValid())
                {
                    bool error = false;
                    QImage img;
                    try{
                        LogHandler::Debug(tr("Saving thumbnail: ") + cachedListItem.thumbFile + tr(" for video: ") + videoFile);
                        img = frame.toImage();
//                        auto vf = VideoFormat::pixelFormatFromImageFormat(QImage::Format_ARGB32);
//                        auto vf2 = VideoFormat(vf);
//                        VideoFrame f = frame.to(vf2, {frame.width(), frame.height()}, QRect(0,0,frame.width(), frame.height()));
//                        QImage img(f.frameDataPtr(), f.width(), f.height(), f.bytesPerLine(0), QImage::Format_ARGB32);
                    }
                    catch (...) {
                        error = true;
                    }
                    QString errorMessage;
                    bool hasError = error || img.isNull() || !img.save(cachedListItem.thumbFile, nullptr, 15);
                    if (hasError)
                    {
                       errorMessage = tr("Error saving thumbnail: ") + cachedListItem.thumbFile + tr(" for video: ") + videoFile;
                       LogHandler::Error(errorMessage);
                       onSaveThumbError(cachedListItem, vrMode, errorMessage);
                       return;
                    }

                }

                onSaveThumbSuccess(cachedListItem, vrMode);
            });
        connect(extractor,
           &QtAV::VideoFrameExtractor::error,
           extractor,
           [this, cachedListItem, videoFile, vrMode](const QString &errorMessage)
            {
                QString error = tr("Error extracting image from: ") + videoFile + tr(" Error: ") + errorMessage;
                LogHandler::Error(error);
                onSaveThumbError(cachedListItem, vrMode, error);
            });

        thumbNailPlayer->setFile(videoFile);
        thumbNailPlayer->load();
    }
}

void MediaLibraryHandler::onSaveThumbSuccess(LibraryListItem27 item, bool vrMode)
{
    disconnect(extractor, &QtAV::VideoFrameExtractor::frameExtracted,  nullptr, nullptr);
    disconnect(extractor, &QtAV::VideoFrameExtractor::error,  nullptr, nullptr);
    disconnect(thumbNailPlayer, &AVPlayer::loaded,  nullptr, nullptr);
    disconnect(thumbNailPlayer, &AVPlayer::error,  nullptr, nullptr);
    LibraryListItem27 &cachedItem = vrMode ? cachedVRItems[cachedVRItems.indexOf(item)] : cachedLibraryItems[cachedLibraryItems.indexOf(item)];
    setThumbPath(cachedItem);
    emit saveNewThumb(item, vrMode, item.thumbFile);
    if(thumbProcessIsRunning)
        saveNewThumbs(vrMode);
}

void MediaLibraryHandler::onSaveThumbError(LibraryListItem27 item, bool vrMode, QString errorMessage)
{
    onSaveThumbSuccess(item, vrMode);
    LibraryListItem27 &cachedItem = vrMode ? cachedVRItems[cachedVRItems.indexOf(item)] : cachedLibraryItems[cachedLibraryItems.indexOf(item)];
    cachedItem.thumbFile = item.thumbFileError;
    emit saveThumbError(cachedItem, vrMode, errorMessage);
}

QList<LibraryListItem27> MediaLibraryHandler::getLibraryCache()
{
    const QMutexLocker locker(&_mutex);
    return cachedLibraryItems;
}
QList<LibraryListItem27> MediaLibraryHandler::getVRLibraryCache()
{
    const QMutexLocker locker(&_mutex);
    return cachedVRItems;
}
LibraryListItem27 MediaLibraryHandler::setupPlaylistItem(QString playlistName)
{
    LibraryListItem27 item;
    item.type = LibraryListItemType::PlaylistInternal;
    item.nameNoExtension = playlistName; //nameNoExtension
    item.modifiedDate = QDate::currentDate();
    item.duration = 0;
    assignID(item);
    setLiveProperties(item);
    const QMutexLocker locker(&_mutex);
    cachedLibraryItems.push_front(item);
    emit playListItem(item);
    return item;
}

void MediaLibraryHandler::setLiveProperties(LibraryListItem27 &libraryListItem)
{
    setThumbPath(libraryListItem);
    updateToolTip(libraryListItem);
}

void MediaLibraryHandler::lockThumb(LibraryListItem27 &item)
{
    QFile file(item.thumbFile);
    if(!item.thumbFile.contains(".lock."))
    {
        QString path = item.thumbFile;
        QString localpath = path;
        QString thumbTemp = localpath;
        int indexOfSuffix = localpath.lastIndexOf(".");
        QString extension = thumbTemp.remove(0, thumbTemp.length() - (thumbTemp.length() - thumbTemp.lastIndexOf('.')));
        QString newName = localpath.replace(indexOfSuffix, localpath.length() - indexOfSuffix, ".lock"+extension);
        bool success = file.rename(newName);
        if(success) {
            LogHandler::Debug("File renamed: "+ newName);
            item.thumbFile = newName;
        } else {
            LogHandler::Error("File NOT renamed: "+ newName);
        }
    }
}
void MediaLibraryHandler::unlockThumb(LibraryListItem27 &item)
{
    QFile file(item.thumbFile);
    if(item.thumbFile.contains(".lock."))
    {
        QString path = item.thumbFile;
        QString newName = path.remove(".lock");
        bool success = file.rename(newName);
        if(success) {
            LogHandler::Debug("File renamed: "+ newName);
            item.thumbFile = newName;
        } else {
            LogHandler::Error("File NOT renamed: "+ newName);
        }
    }
}
void MediaLibraryHandler::setThumbPath(LibraryListItem27 &libraryListItem)
{
    if(libraryListItem.type == LibraryListItemType::Audio)
    {
        libraryListItem.thumbFile = "://images/icons/audio.png";
        return;
    }
    else if(libraryListItem.type == LibraryListItemType::PlaylistInternal)
    {
        libraryListItem.thumbFile = "://images/icons/playlist.png";
        return;
    }
    else if(libraryListItem.type == LibraryListItemType::FunscriptType)
    {
        libraryListItem.thumbFile = "://images/icons/funscript.png";
        return;
    }
    QStringList imageExtensions;
    imageExtensions << ".jpg" << ".jpeg" << ".png";
    QFileInfo mediaInfo(libraryListItem.path);
    foreach(QString ext, imageExtensions) {
        QString filepath = mediaInfo.absolutePath() + QDir::separator() + libraryListItem.nameNoExtension + ext;
        if(QFileInfo(filepath).exists())
        {
            libraryListItem.thumbFile = filepath;
            return;
        }
        QString filepathLocked = mediaInfo.absolutePath() + QDir::separator() + libraryListItem.nameNoExtension + ".lock" + ext;
        if(QFileInfo(filepathLocked).exists())
        {
            libraryListItem.thumbFile = filepathLocked;
            return;
        }
    }
    if(SettingsHandler::getUseMediaDirForThumbs())
    {
        libraryListItem.thumbFile = mediaInfo.absolutePath() + libraryListItem.nameNoExtension + ".jpg";
        return;
    }

    libraryListItem.thumbFile = SettingsHandler::getSelectedThumbsDir() + libraryListItem.name + ".jpg";
}

void MediaLibraryHandler::updateToolTip(LibraryListItem27 &localData)
{
    localData.isMFS = false;
    QFileInfo scriptInfo(localData.script);
    QFileInfo zipScriptInfo(localData.zipFile);
    localData.toolTip = localData.nameNoExtension + "\nMedia:";
    if (localData.type != LibraryListItemType::PlaylistInternal && !scriptInfo.exists() && !zipScriptInfo.exists())
    {
        localData.toolTip = localData.path + "\nNo script file of the same name found.\nRight click and Play with funscript.";
    }
    else if (localData.type != LibraryListItemType::PlaylistInternal)
    {
        localData.toolTip += "\n";
        localData.toolTip += localData.path;
        localData.toolTip += "\n";
        localData.toolTip += "Scripts:\n";
        if(zipScriptInfo.exists())
        {
            localData.toolTip += localData.zipFile;
            localData.isMFS = true;
        }
        else
        {
            localData.toolTip += localData.script;
        }
        auto availibleAxis = SettingsHandler::getAvailableAxis();
        foreach(auto axisName, availibleAxis->keys())
        {
            auto track = availibleAxis->value(axisName);
            if(axisName == TCodeChannelLookup::Stroke() || track.Type == AxisType::HalfRange || track.TrackName.isEmpty())
                continue;

            QString script = localData.scriptNoExtension + "." + track.TrackName + ".funscript";
            QFileInfo fileInfo(script);
            if (fileInfo.exists())
            {
                localData.isMFS = true;
                localData.toolTip += "\n";
                localData.toolTip += script;
            }
        }
    }
    else if (localData.type == LibraryListItemType::PlaylistInternal)
    {
        auto playlists = SettingsHandler::getPlaylists();
        auto playlist = playlists.value(localData.nameNoExtension);
        for(auto i = 0; i < playlist.length(); i++)
        {
            localData.toolTip += "\n";
            localData.toolTip += QString::number(i + 1);
            localData.toolTip += ": ";
            localData.toolTip += playlist[i].nameNoExtension;
        }
    }
}

void MediaLibraryHandler::assignID(LibraryListItem27 &item)
{
    //.replace(/^[^a-z]+|[^\w:.-]+/gi, "")+"item"+i
    // /^[^a-zA-Z]+|[^\\w\\s:.-]+/g
    QString name = item.nameNoExtension;
    item.ID = name.remove(" ") + "Item" + QString::number(libraryItemIDTracker);
    libraryItemIDTracker++;
}
