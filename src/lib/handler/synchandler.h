#ifndef SYNCHANDLER_H
#define SYNCHANDLER_H

#include <QFuture>
#include "../../settingsdialog.h"
#include "tcodehandler.h"
#include "funscripthandler.h"
#include "videohandler.h"

class SyncHandler: QObject
{
    Q_OBJECT
public:
    SyncHandler(SettingsDialog* xSettings, TCodeHandler* tcodeHandler, VideoHandler* videoHandler, QObject *parent = nullptr);
    ~SyncHandler();
    void playFunscript(QString funscript);
    void syncVRFunscript();
    void syncFunscript();
    void setFunscriptTime(qint64 secs);
    qint64 getFunscriptTime();
    qint64 getFunscriptMin();
    qint64 getFunscriptMax();
    void stop();
    void clear();
    void reset();
    bool load(QString funscript);
    bool load(QByteArray funscript);
    bool loadMFS(QString channel, QString funscript);
    bool loadMFS(QString channel, QByteArray funscript);
    bool isLoaded();
signals:
    void funscriptTimeCode(qint64 msecs);
private:
    SettingsDialog* _xSettings;
    TCodeHandler* _tcodeHandler;
    VideoHandler* _videoHandler;

    bool _isFunscriptPlaying = false;
    qint64 _currentTime = 0;
    qint64 _seekTime = -1;
    QFuture<void> _funscriptFuture;
    FunscriptHandler* _funscriptHandler;
    QList<FunscriptHandler*> _funscriptHandlers;
};

#endif // SYNCHANDLER_H
