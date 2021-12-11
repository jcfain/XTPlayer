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
    void saveNewThumbLoading(LibraryListItem27 item);
    void saveNewThumb(LibraryListItem27 item, bool vrMode,  QString errorMessage, QString thumbFile);
    void saveThumbError(LibraryListItem27 item, bool vrMode, QString error);
    void prepareLibraryLoad();
    void libraryNotFound();
    void libraryItemFound(LibraryListItem27 item, bool vrMode);
    void libraryLoadingStatus(QString message);
    void libraryLoading();
    void libraryLoaded();
    void playListItem(LibraryListItem27 item);
public:
    MediaLibraryHandler();
    ~MediaLibraryHandler();
    void saveSingleThumb(LibraryListItem27 item, qint64 position = 0);
    void startThumbProcess(bool vrMode = false);
    void stopThumbProcess();
    void onPrepareLibraryLoad();
    void onLibraryItemFound(LibraryListItem27 item, bool vrMode);
    void onSaveThumbError(LibraryListItem27 item, bool vrMode, QString errorMessage);
    void loadLibraryAsync();
    bool isLibraryLoading();
    void stopLibraryLoading();
    LibraryListItem27 setupPlaylistItem(QString name);
    QList<LibraryListItem27> getLibraryCache();
    QList<LibraryListItem27> getVRLibraryCache();
    void updateToolTip(LibraryListItem27 &localData);

    //Private?
    QString getThumbPath(LibraryListItem27 item);

private:
    VideoFrameExtractor* extractor = 0;
    AVPlayer* thumbNailPlayer = 0;
    bool thumbProcessIsRunning = false;
    bool loadingLibraryStop = false;
    int thumbNailSearchIterator = 0;
    QList<LibraryListItem27> cachedLibraryItems;
    QList<LibraryListItem27> cachedVRItems;
    QFuture<void> loadingLibraryFuture;

    void on_load_library(QString path, bool vrMode);
    void onLibraryLoaded();
    void saveNewThumbs(bool vrMode = false);
    void saveThumb(LibraryListItem27 item, qint64 position = 0, bool vrMode = false);

    LibraryListItem27 createLibraryListItemFromFunscript(QString funscript);



};

#endif // MEDIALIBRARYHANDLER_H
