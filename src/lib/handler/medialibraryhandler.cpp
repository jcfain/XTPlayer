#include "medialibraryhandler.h"

MediaLibraryHandler::MediaLibraryHandler(QObject* parent)
    : QObject(parent)
{
    _thumbTimeoutTimer.setSingleShot(true);
    connect(this, &MediaLibraryHandler::prepareLibraryLoad, this, &MediaLibraryHandler::onPrepareLibraryLoad);
    connect(this, &MediaLibraryHandler::libraryLoaded, this, &MediaLibraryHandler::onLibraryLoaded);
}

MediaLibraryHandler::~MediaLibraryHandler()
{
    if(_loadingLibraryFuture.isRunning())
    {
        _loadingLibraryStop = true;
        _loadingLibraryFuture.cancel();
        _loadingLibraryFuture.waitForFinished();
    }
//    if(_extractor)
//        delete _extractor;
    //    if(thumbNailPlayer)
    //        delete thumbNailPlayer;
    //qDeleteAll(cachedVRItems);
    //cachedVRItems.clear();
}
bool MediaLibraryHandler::isLibraryLoading()
{
    return _loadingLibraryFuture.isRunning();
}

void MediaLibraryHandler::stopLibraryLoading()
{
    _loadingLibraryStop = true;
    _loadingLibraryFuture.cancel();
    _loadingLibraryFuture.waitForFinished();
}

void MediaLibraryHandler::onPrepareLibraryLoad()
{
    stopThumbProcess();
    //_xSettings->setLibraryLoaded(false, cachedLibraryItems, cachedVRItems);
    const QMutexLocker locker(&_mutex);
    _cachedLibraryItems.clear();
    _cachedVRItems.clear();
    _libraryItemIDTracker = 1;
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
        _loadingLibraryFuture = QtConcurrent::run([this, library, vrLibrary]() {
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
        if(_loadingLibraryStop)
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
        item.modifiedDate = fileinfo.birthTime().isValid() ? fileinfo.birthTime().date() : fileinfo.created().date();
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
            if(_loadingLibraryStop)
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
            item.modifiedDate = fileinfo.birthTime().isValid() ? fileinfo.birthTime().date() : fileinfo.created().date();
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
        _cachedVRItems.push_back(item);
    else
        _cachedLibraryItems.push_back(item);
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
    item.modifiedDate = fileinfo.birthTime().isValid() ? fileinfo.birthTime().date() : fileinfo.created().date();
    item.duration = 0;
    assignID(item);
    setLiveProperties(item);

    const QMutexLocker locker(&_mutex);
    _cachedLibraryItems.push_back(item);
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
    QString thumbPath = SettingsHandler::getSelectedThumbsDir();
    QDir thumbDir(thumbPath);
    if (!thumbDir.exists())
    {
        thumbDir.mkdir(thumbPath);
    }
    stopThumbProcess();
    LogHandler::Debug("Start thumb process, vrMode: " + QString::number(vrMode));
    _thumbProcessIsRunning = true;
    _extractor = new XVideoPreview(this);
    saveNewThumbs(vrMode);
    emit thumbProcessBegin();
}

void MediaLibraryHandler::stopThumbProcess()
{
    if(_thumbProcessIsRunning)
    {
        LogHandler::Debug("Stop thumb process");
        disconnect(_extractor, &XVideoPreview::frameExtracted,  nullptr, nullptr);
        disconnect(_extractor, &XVideoPreview::frameExtractionError,  nullptr, nullptr);
        disconnect(_extractor, &XVideoPreview::mediaLoaded,  nullptr, nullptr);
        _thumbNailSearchIterator = 0;
        _thumbProcessIsRunning = false;
        if(_thumbTimeoutTimer.isActive())
        {
            _thumbTimeoutTimer.stop();
            disconnect(&_thumbTimeoutTimer, &QTimer::timeout, nullptr, nullptr);
        }
        delete _extractor;
//        delete _thumbNailPlayer;
    }
}

void MediaLibraryHandler::saveSingleThumb(LibraryListItem27 item, qint64 position)
{
    if(!_thumbProcessIsRunning && !item.thumbFile.contains(".lock."))
    {
        _extractor = new XVideoPreview(this);
        saveThumb(item, position);
    }
}

void MediaLibraryHandler::saveNewThumbs(bool vrMode)
{
    if (_thumbProcessIsRunning && _thumbNailSearchIterator < (vrMode ? _cachedVRItems.count() : _cachedLibraryItems.count()))
    {
        LibraryListItem27 item = vrMode ? _cachedVRItems.at(_thumbNailSearchIterator) : _cachedLibraryItems.at(_thumbNailSearchIterator);
        _thumbNailSearchIterator++;
        QFileInfo thumbInfo(item.thumbFile);
        if (item.type == LibraryListItemType::Video && !thumbInfo.exists())
        {
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
        {
            LogHandler::Debug("Thumb process finished");
            emit thumbProcessEnd();
        }
    }
}

void MediaLibraryHandler::saveThumb(LibraryListItem27 cachedListItem, qint64 position, bool vrMode)
{
    if(_thumbProcessIsRunning && (cachedListItem.type == LibraryListItemType::Audio || cachedListItem.type == LibraryListItemType::FunscriptType || cachedListItem.thumbFile.contains(".lock.")))
    {
        saveNewThumbs(vrMode);
    }
    else
    {
        _thumbTimeoutTimer.stop();
        disconnect(&_thumbTimeoutTimer, &QTimer::timeout, nullptr, nullptr);
        disconnect(_extractor, nullptr,  nullptr, nullptr);
        connect(&_thumbTimeoutTimer, &QTimer::timeout, &_thumbTimeoutTimer, [this, cachedListItem, vrMode]() {
            if(_thumbProcessIsRunning)
            {
                disconnect(_extractor, nullptr,  nullptr, nullptr);
                onSaveThumb(cachedListItem, vrMode, "Thumb loading timed out.");
            }
        });
        _thumbTimeoutTimer.start(30000);
        if(!vrMode)
            emit saveNewThumbLoading(cachedListItem);
        // Get the duration and randomize the position with in the video.
        QString videoFile = cachedListItem.path;
        LogHandler::Debug("Getting thumb: " + cachedListItem.thumbFile);
        connect(_extractor, &XVideoPreview::durationChanged, this,
           [this, videoFile, position](qint64 duration)
            {
               disconnect(_extractor, &XVideoPreview::durationChanged,  nullptr, nullptr);
               LogHandler::Debug("Loaded video for thumb. Duration: " + QString::number(duration));
               qint64 randomPosition = position > 0 ? position : XMath::rand((qint64)1, duration);
               LogHandler::Debug("Extracting at: " + QString::number(randomPosition));
               _extractor->extract(videoFile, randomPosition);
            });


//        connect(_thumbNailPlayer, &AVPlayer::error, _thumbNailPlayer,
//           [this, cachedListItem, vrMode](QtAV::AVError er)
//            {
//                QString error = "Video load error from: " + cachedListItem.path + " Error: " + er.ffmpegErrorString();
//                onSaveThumb(cachedListItem, vrMode, error);
//            });


        connect(_extractor, &XVideoPreview::frameExtracted, this,
           [this, cachedListItem, vrMode](QPixmap frame)
            {
                disconnect(_extractor, &XVideoPreview::frameExtracted,  nullptr, nullptr);
                disconnect(_extractor, &XVideoPreview::frameExtractionError,  nullptr, nullptr);
                if(!frame.isNull())
                {
                    bool error = false;
                    QImage img;
                    try{
                        LogHandler::Debug("Saving thumbnail");
                        img = frame.toImage();
                    }
                    catch (...) {
                        error = true;
                    }
                    QString errorMessage;
                    bool hasError = error || img.isNull() || !img.save(cachedListItem.thumbFile, nullptr, 15);
                    if (hasError)
                    {
                       onSaveThumb(cachedListItem, vrMode, "Error saving thumbnail");
                       return;
                    }

                }

                onSaveThumb(cachedListItem, vrMode);
            });

        connect(_extractor, &XVideoPreview::frameExtractionError, this,
           [this, cachedListItem, vrMode](const QString &errorMessage)
            {
                disconnect(_extractor, &XVideoPreview::frameExtracted,  nullptr, nullptr);
                disconnect(_extractor, &XVideoPreview::frameExtractionError,  nullptr, nullptr);
                QString error = "Error extracting image from: " + cachedListItem.path + " Error: " + errorMessage;
                onSaveThumb(cachedListItem, vrMode, error);
            });

        _extractor->load(videoFile);
    }
}

//void MediaLibraryHandler::saveThumbs(QList<LibraryListItem27> items, qint64 position, bool vrMode)
//{
//    thumbNailPlayer->setAsyncLoad(false);
//    extractor->setAsync(false);
//    //extractor->setAutoExtract(false);
//    QtConcurrent::run([this, items, position, vrMode]() {
//        foreach(LibraryListItem27 item, items)
//        {
//            if(!thumbProcessIsRunning)
//                return;
//            if(thumbProcessIsRunning && (item.type == LibraryListItemType::Audio || item.type == LibraryListItemType::FunscriptType || item.thumbFile.contains(".lock.")))
//                continue;
//            thumbNailPlayer->setFile(item.path);
//            if(!thumbNailPlayer->load()) {
//                QString error = tr("Video load error from: ") + item.path;
//                onSaveThumb(item, vrMode, error);
//                continue;
//            }
//            extractor->setSource(item.path);
//            qint64 randomPosition = position > 0 ? position : XMath::rand((qint64)1, thumbNailPlayer->duration());
//            extractor->setPosition(randomPosition);
//        }
//    });
//}

void MediaLibraryHandler::onSaveThumb(LibraryListItem27 item, bool vrMode, QString errorMessage)
{
    int cachedIndex = vrMode ? _cachedVRItems.indexOf(item) : _cachedLibraryItems.indexOf(item);
    if(cachedIndex == -1) {
        LibraryListItem27 emptyItem;
        emit saveThumbError(emptyItem, vrMode, "Missing media");
        return;
    }
    LibraryListItem27 &cachedItem = vrMode ? _cachedVRItems[cachedIndex] : _cachedLibraryItems[cachedIndex];
    if(!errorMessage.isEmpty())
    {
        cachedItem.thumbFile = item.thumbFileError;
        LogHandler::Error("Save thumb error: " + errorMessage);
        emit saveThumbError(cachedItem, vrMode, errorMessage);
    }
    else
    {
        setThumbPath(cachedItem);
        LogHandler::Debug("Thumb saved: " + item.thumbFile);
        emit saveNewThumb(item, vrMode, item.thumbFile);
    }
    if(_thumbProcessIsRunning)
        saveNewThumbs(vrMode);
}

QList<LibraryListItem27> MediaLibraryHandler::getLibraryCache()
{
    const QMutexLocker locker(&_mutex);
    return _cachedLibraryItems;
}
QList<LibraryListItem27> MediaLibraryHandler::getVRLibraryCache()
{
    const QMutexLocker locker(&_mutex);
    return _cachedVRItems;
}
LibraryListItem27 MediaLibraryHandler::setupPlaylistItem(QString playlistName)
{
    LibraryListItem27 item;
    item.type = LibraryListItemType::PlaylistInternal;
    item.nameNoExtension = playlistName; //nameNoExtension
    item.modifiedDate = QDateTime::currentDateTime().date();
    item.duration = 0;
    assignID(item);
    setLiveProperties(item);
    const QMutexLocker locker(&_mutex);
    _cachedLibraryItems.push_front(item);
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
        LogHandler::Debug("Rename: "+ item.thumbFile);
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
            LogHandler::Error("File rename failed: "+ newName);
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
        libraryListItem.thumbFileExists = true;
        return;
    }
    else if(libraryListItem.type == LibraryListItemType::PlaylistInternal)
    {
        libraryListItem.thumbFile = "://images/icons/playlist.png";
        libraryListItem.thumbFileExists = true;
        return;
    }
    else if(libraryListItem.type == LibraryListItemType::FunscriptType)
    {
        libraryListItem.thumbFile = "://images/icons/funscript.png";
        libraryListItem.thumbFileExists = true;
        return;
    }
    QStringList imageExtensions;
    imageExtensions << ".jpg" << ".jpeg" << ".png" << ".jfif" << ".webp";
    QFileInfo mediaInfo(libraryListItem.path);
    foreach(QString ext, imageExtensions) {
        QString filepath = mediaInfo.absolutePath() + QDir::separator() + libraryListItem.nameNoExtension + ext;
        libraryListItem.thumbFileExists = QFileInfo(filepath).exists();
        if(libraryListItem.thumbFileExists)
        {
            libraryListItem.thumbFile = filepath;
            return;
        }
        QString filepathLocked = mediaInfo.absolutePath() + QDir::separator() + libraryListItem.nameNoExtension + ".lock" + ext;
        libraryListItem.thumbFileExists = QFileInfo(filepathLocked).exists();
        if(QFileInfo(filepathLocked).exists())
        {
            libraryListItem.thumbFileExists = true;
            libraryListItem.thumbFile = filepathLocked;
            return;
        }
        QString filepathGlobalLocked =  SettingsHandler::getSelectedThumbsDir() + libraryListItem.name + ".lock" + ext;
        if(QFileInfo(filepathGlobalLocked).exists())
        {
            libraryListItem.thumbFileExists = true;
            libraryListItem.thumbFile = filepathGlobalLocked;
            return;
        }
    }
    if(SettingsHandler::getUseMediaDirForThumbs())
    {
        libraryListItem.thumbFile = mediaInfo.absolutePath() + libraryListItem.nameNoExtension + ".jpg";
        libraryListItem.thumbFileExists = QFileInfo(libraryListItem.thumbFile).exists();
        return;
    }

    libraryListItem.thumbFile = SettingsHandler::getSelectedThumbsDir() + libraryListItem.name + ".jpg";
    libraryListItem.thumbFileExists = QFileInfo(libraryListItem.thumbFile).exists();
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
    item.ID = name.remove(" ") + "Item" + QString::number(_libraryItemIDTracker);
    _libraryItemIDTracker++;
}
