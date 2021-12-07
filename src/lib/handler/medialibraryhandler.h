#ifndef MEDIALIBRARYHANDLER_H
#define MEDIALIBRARYHANDLER_H

#include <QObject>
#include <QtAV>
#include <QFileInfo>
#include <QtConcurrent/QtConcurrent>

#include "../handler/settingshandler.h"
#include "../struct/LibraryListItem.h"

using namespace QtAV;

class MediaLibraryHandler : QObject
{
    Q_OBJECT
signals:
    void saveThumbError(bool vrMode, QString error);
    void thumbFileLoaded(bool hasError, QString error);
    void libraryNotFound();
    void prepareLibraryLoad();
    void libraryItemFound(LibraryListItem item);
    void libraryLoading(bool loading, QString message = nullptr);
    void libraryLoaded();
    void playListItem(LibraryListItem item);
public:
    MediaLibraryHandler();
    void saveSingleThumb(LibraryListItem item, qint64 position = 0);
    void startThumbProcess(bool vrMode = false);
    void stopThumbProcess();
    void onPrepareLibraryLoad();
    void loadLibraryAsync();
    LibraryListItem setupPlaylistItem(QString name);

    //Private?
    QString getThumbPath(LibraryListItem item);

private:
    VideoFrameExtractor* extractor = 0;
    AVPlayer* thumbNailPlayer = 0;
    bool thumbProcessIsRunning = false;
    bool loadingLibraryStop = false;
    int thumbNailSearchIterator = 0;
    QList<LibraryListItem> cachedLibraryItems;
    QList<LibraryListItem> cachedVRItems;
    QFuture<void> loadingLibraryFuture;

    void on_load_library(QString path, bool vrMode);
    void saveNewThumbs(bool vrMode = false);
    void saveThumb(LibraryListItem item, qint64 position = 0, bool vrMode = false);



};

#endif // MEDIALIBRARYHANDLER_H
