#ifndef SYNCHANDLER_H
#define SYNCHANDLER_H

#include <QObject>
#include <QFuture>
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
    void playFunscript(QString funscript);
    void syncVRFunscript();
    void syncFunscript();
    void setFunscriptTime(qint64 secs);
    qint64 getFunscriptTime();
    qint64 getFunscriptMin();
    qint64 getFunscriptMax();
    void stopStandAlone();
    void stopMediaFunscript();
    void stopAll();
    void clear();
    void reset();
    bool load(QString funscript);
    bool load(QByteArray funscript);
    bool loadMFS(QString channel, QString funscript);
    bool loadMFS(QString channel, QByteArray funscript);
    bool isLoaded();
    bool isPlayingStandAlone();
    QString getPlayingStandAloneScript();
private:
    SettingsDialog* _xSettings;
    TCodeHandler* _tcodeHandler;
    VideoHandler* _videoHandler;

    QString _playingStandAloneFunscript;
    bool _isFunscriptPlaying = false;
    bool _isStandAloneFunscriptPlaying = false;
    bool _isPaused = false;
    qint64 _currentTime = 0;
    qint64 _seekTime = -1;
    QFuture<void> _funscriptFuture;
    QFuture<void> _funscriptStandAloneFuture;
    FunscriptHandler* _funscriptHandler;
    QList<FunscriptHandler*> _funscriptHandlers;
};

#endif // SYNCHANDLER_H
