#ifndef SYNCHANDLER_H
#define SYNCHANDLER_H

#include <QObject>
#include <QFuture>
#include <QtCompress/qzipwriter.h>
#include <QtCompress/qzipreader.h>
#include "../../settingsdialog.h"
#include "tcodehandler.h"
#include "funscripthandler.h"
#include "videohandler.h"

class SyncHandler: public QObject
{
    Q_OBJECT
signals:
    void funscriptPositionChanged(qint64 msecs);
    void funscriptStatusChanged(QtAV::MediaStatus status);
    void funscriptStopped();
    void funscriptStarted();
    void togglePaused(bool paused);
public:
    SyncHandler(SettingsDialog* xSettings, TCodeHandler* tcodeHandler, VideoHandler* videoHandler, QObject *parent = nullptr);
    ~SyncHandler();
    void togglePause();
    bool isPaused();
    void playStandAlone(QString funscript = nullptr);
    void setStandAloneLoop(bool enabled);
    void syncVRFunscript(QString funscript);
    void syncFunscript();
    void setFunscriptTime(qint64 secs);
    qint64 getFunscriptTime();
    qint64 getFunscriptMin();
    qint64 getFunscriptMax();
    void stopStandAloneFunscript();
    void stopMediaFunscript();
    void stopVRFunscript();
    void stopAll();
    void clear();
    void reset();
    QList<QString> load(QString funscript);
    bool isLoaded();
    bool isPlaying();
    bool isPlayingStandAlone();
    QString getPlayingStandAloneScript();
private:
    SettingsDialog* _xSettings;
    TCodeHandler* _tcodeHandler;
    VideoHandler* _videoHandler;

    QMutex _mutex;
    QString _playingStandAloneFunscript;
    bool _isMediaFunscriptPlaying = false;
    bool _isVRFunscriptPlaying = false;
    bool _isStandAloneFunscriptPlaying = false;
    bool _isPaused = false;
    bool _standAloneLoop;
    qint64 _currentTime = 0;
    qint64 _seekTime = -1;
    QFuture<void> _funscriptMediaFuture;
    QFuture<void> _funscriptVRFuture;
    QFuture<void> _funscriptStandAloneFuture;
    FunscriptHandler* _funscriptHandler;
    QList<FunscriptHandler*> _funscriptHandlers;
    QList<QString> _invalidScripts;

    bool load(QByteArray funscript);
    void loadMFS(QString funscript);
    bool loadMFS(QString channel, QString funscript);
    bool loadMFS(QString channel, QByteArray funscript);
};

#endif // SYNCHANDLER_H
