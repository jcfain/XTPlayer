#include "medialibraryhandler.h"

MediaLibraryHandler::MediaLibraryHandler()
{
//    if(extractor)
//        delete extractor;
//    if(thumbNailPlayer)
//        delete thumbNailPlayer;
}
void MediaLibraryHandler::loadLibraryAsync()
{
    QString library = SettingsHandler::getSelectedLibrary();
    QString vrLibrary = SettingsHandler::getVRLibrary();
    if(library.isEmpty() && vrLibrary.isEmpty())
    {
        emit libraryLoading(false);
        return;
    }
    if(!loadingLibraryFuture.isRunning())
    {
        emit libraryLoading(true, library.isEmpty() ? "Loading VR library..." : "Loading library...");
        loadingLibraryFuture = QtConcurrent::run([this, library, vrLibrary]() {
            on_load_library(library.isEmpty() ? vrLibrary : library, library.isEmpty());
        });
    }
}

LibraryListItem MediaLibraryHandler::setupPlaylistItem(QString playlistName)
{
    LibraryListItem item
    {
        LibraryListItemType::PlaylistInternal,
        nullptr, // path
        nullptr, // name
        playlistName, //nameNoExtension
        nullptr, // script
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        QDate::currentDate(),
        0
    };
    emit playListItem(item);
    return item;
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
            return;
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
        LibraryListItem item
        {
            libratyItemType,
            videoPath, // path
            fileName, // name
            fileNameNoExtension, //nameNoExtension
            scriptPath, // script
            scriptNoExtension,
            mediaExtension,
            nullptr,
            zipFile,
            fileinfo.birthTime().date(),
            0
        };
        item.thumbFile = getThumbPath(item);
        emit libraryItemFound(item);
//        LibraryListWidgetItem* qListWidgetItem = new LibraryListWidgetItem(item, vrMode ? nullptr : libraryList);
//        if(!vrMode)
//            libraryList->addItem(qListWidgetItem);
//        vrMode ? cachedVRItems.push_back((LibraryListWidgetItem*)qListWidgetItem->clone()) : cachedLibraryItems.push_back((LibraryListWidgetItem*)qListWidgetItem->clone());
//        if(!vrMode && !scriptPath.isEmpty())
//            funscriptsWithMedia.append(scriptPath);
//        if(!vrMode && !zipFile.isEmpty())
//            funscriptsWithMedia.append(zipFile);
    }

    if(!vrMode && !SettingsHandler::getHideStandAloneFunscriptsInLibrary())
    {
        emit libraryLoading(true, "Searching for lone funscripts...");
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
                return;
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
            LibraryListItem item
            {
                LibraryListItemType::FunscriptType,
                scriptPath, // path
                fileName, // name
                fileNameNoExtension, //nameNoExtension
                scriptPath, // script
                scriptNoExtension,
                mediaExtension,
                nullptr,
                zipFile,
                fileinfo.birthTime().date(),
                0
            };
            emit libraryItemFound(item);
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
        emit libraryLoading(true, "Loading VR library...");
        on_load_library(SettingsHandler::getVRLibrary(), true);
    }

}

LibraryListItem createLibraryListItemFromFunscript(QString funscript)
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
    return
    {
        LibraryListItemType::FunscriptType,
        scriptPath, // path
        fileName, // name
        fileNameNoExtension, //nameNoExtension
        scriptPath, // script
        fileNameNoExtension,
        mediaExtension,
        nullptr,
        zipFile,
        fileinfo.birthTime().date(),
        0
    };
}

QString MediaLibraryHandler::getThumbPath(LibraryListItem libraryListItem)
{
    QStringList imageExtensions;
    imageExtensions << ".jpg" << ".jpeg" << ".png";
    QFileInfo mediaInfo(libraryListItem.path);
    foreach(QString ext, imageExtensions) {
        QString filepath = mediaInfo.absolutePath() + QDir::separator() + libraryListItem.nameNoExtension + ext;
        if(QFile(filepath).exists())
            return filepath;
    }
    return SettingsHandler::getSelectedThumbsDir() + libraryListItem.name + ".jpg";
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

void MediaLibraryHandler::saveSingleThumb(LibraryListItem item, qint64 position)
{
    if(!thumbProcessIsRunning)
    {
        extractor = new VideoFrameExtractor(this);
        thumbNailPlayer = new AVPlayer(this);
        thumbNailPlayer->setInterruptOnTimeout(true);
        thumbNailPlayer->setInterruptTimeout(10000);
        thumbNailPlayer->setAsyncLoad(true);
        extractor->setAsync(true);
    }
    saveThumb(item, position);
}

void MediaLibraryHandler::saveNewThumbs(bool vrMode)
{
    if (thumbProcessIsRunning && thumbNailSearchIterator < (vrMode ? cachedVRItems.count() : cachedLibraryItems.count()))
    {
        //Use a non user modifiable list incase they sort random when getting thumbs.
        int currentindex = thumbNailSearchIterator;
        LibraryListItem item = vrMode ? cachedVRItems.at(thumbNailSearchIterator) : cachedLibraryItems.at(thumbNailSearchIterator);
        thumbNailSearchIterator++;
        QFileInfo thumbInfo(getThumbPath(item));
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
    }
}
void MediaLibraryHandler::saveThumb(LibraryListItem cachedListItem, qint64 position, bool vrMode)
{
    QString videoFile = cachedListItem.path;
    QString thumbFile = getThumbPath(cachedListItem);
    //://images/icons/loading_current.png
    if(cachedListItem.type == LibraryListItemType::Audio || cachedListItem.type == LibraryListItemType::FunscriptType)
    {
        saveNewThumbs(vrMode);
    }
    else
    {
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
           [this, videoFile, vrMode](QtAV::AVError er)
            {
            QString error = tr("Video load error from: ") + videoFile + tr(" Error: ") + er.ffmpegErrorString();
               LogHandler::Error(error);
               emit saveThumbError(vrMode, error);
            });


        connect(extractor,
           &QtAV::VideoFrameExtractor::frameExtracted,
           extractor,
           [this, videoFile, thumbFile, vrMode](const QtAV::VideoFrame& frame)
            {
                if(frame.isValid())
                {
                    bool error = false;
                    QImage img;
                    try{
                        LogHandler::Debug(tr("Saving thumbnail: ") + thumbFile + tr(" for video: ") + videoFile);
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
                    bool hasError = error || img.isNull() || !img.save(thumbFile, nullptr, 15);
                    if (hasError)
                    {
                       errorMessage = tr("Error saving thumbnail: ") + thumbFile + tr(" for video: ") + videoFile;
                       LogHandler::Debug(errorMessage);
                    }
                    emit thumbFileLoaded(hasError, errorMessage);
                }
                disconnect(extractor, &QtAV::VideoFrameExtractor::frameExtracted,  nullptr, nullptr);
                disconnect(extractor, &QtAV::VideoFrameExtractor::error,  nullptr, nullptr);

               saveNewThumbs(vrMode);
            });
        connect(extractor,
           &QtAV::VideoFrameExtractor::error,
           extractor,
           [this, videoFile, vrMode](const QString &errorMessage)
            {
                QString error = tr("Error extracting image from: ") + videoFile + tr(" Error: ") + errorMessage;
                LogHandler::Error(error);
                emit saveThumbError(vrMode, error);
            });

        thumbNailPlayer->setFile(videoFile);
        thumbNailPlayer->load();
    }
}
