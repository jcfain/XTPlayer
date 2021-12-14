#ifndef MEDIALIBRARYHANDLER_H
#define MEDIALIBRARYHANDLER_H

#include <QObject>
#include <QtAV>
#include <QFileInfo>
#include <QtConcurrent/QtConcurrent>

#include "../handler/settingshandler.h"
#include "../struct/LibraryListItem.h"

using namespace QtAV;

class MediaLibraryHandler : public QObject
{
    Q_OBJECT
signals:
    void prepareLibraryLoad();
    void libraryNotFound();
    void libraryItemFound(LibraryListItem27 item, bool vrMode);
    void libraryLoadingStatus(QString message);
    void libraryLoading();
    void libraryLoaded();
    void playListItem(LibraryListItem27 item);

    void thumbProcessBegin();
    void thumbProcessEnd();
    void saveNewThumbLoading(LibraryListItem27 item);
    void saveNewThumb(LibraryListItem27 item, bool vrMode, QString thumbFile);
    void saveThumbError(LibraryListItem27 item, bool vrMode, QString error);
    void frameExtracted(LibraryListItem27 item, bool vrMode, const QtAV::VideoFrame& frame);
    void frameExtractedError(LibraryListItem27 item, bool vrMode, const QString &errorMessage);
    void videoLoadError(LibraryListItem27 item, bool vrMode, QtAV::AVError er);
public:
    MediaLibraryHandler(QObject* parent = nullptr);
    ~MediaLibraryHandler();
    void saveSingleThumb(LibraryListItem27 item, qint64 position = 0);
    void startThumbProcess(bool vrMode = false);
    void stopThumbProcess();
    void loadLibraryAsync();
    bool isLibraryLoading();
    void stopLibraryLoading();
    LibraryListItem27 setupPlaylistItem(QString name);
    QList<LibraryListItem27> getLibraryCache();
    QList<LibraryListItem27> getVRLibraryCache();
    void setLiveProperties(LibraryListItem27 &item);
    void lockThumb(LibraryListItem27 &item);
    void unlockThumb(LibraryListItem27 &item);

private:
    int _libraryItemIDTracker = 1;
    VideoFrameExtractor* _extractor = 0;
    AVPlayer* _thumbNailPlayer = 0;
    bool _thumbProcessIsRunning = false;
    bool _loadingLibraryStop = false;
    int _thumbNailSearchIterator = 0;
    QList<LibraryListItem27> _cachedLibraryItems;
    QList<LibraryListItem27> _cachedVRItems;
    QFuture<void> _loadingLibraryFuture;
    QTimer _thumbTimeoutTimer;
    QMutex _mutex;

    void on_load_library(QString path, bool vrMode);
    void onLibraryLoaded();
    //void saveThumbs(QList<LibraryListItem27> items, qint64 position = 0, bool vrMode = false);
    void onPrepareLibraryLoad();
    void onLibraryItemFound(LibraryListItem27 item, bool vrMode);
    void onSaveThumb(LibraryListItem27 item, bool vrMode, QString errorMessage = nullptr);
    void setThumbPath(LibraryListItem27 &item);
    void saveNewThumbs(bool vrMode = false);
    void saveThumb(LibraryListItem27 item, qint64 position = 0, bool vrMode = false);
    void updateToolTip(LibraryListItem27 &item);
    void assignID(LibraryListItem27 &item);

    LibraryListItem27 createLibraryListItemFromFunscript(QString funscript);



};

#endif // MEDIALIBRARYHANDLER_H
