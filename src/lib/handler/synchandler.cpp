#include "synchandler.h"

SyncHandler::SyncHandler(SettingsDialog* xSettings, TCodeHandler* tcodeHandler, VideoHandler* videoHandler, QObject* parent):
    QObject(parent)
{
    _xSettings = xSettings;
    _tcodeHandler = tcodeHandler;
    _videoHandler = videoHandler;
    _funscriptHandler = new FunscriptHandler(TCodeChannelLookup::Stroke());
}

SyncHandler::~SyncHandler()
{
    delete _funscriptHandler;
    qDeleteAll(_funscriptHandlers);
}

void SyncHandler::togglePause()
{
    if((isLoaded() && _isFunscriptPlaying) || (isLoaded() && _isStandAloneFunscriptPlaying))
    {
        _isPaused = !_isPaused;
        emit togglePaused(isPaused());
    }
}

bool SyncHandler::isPaused()
{
    return _isPaused;
}
bool SyncHandler::isPlayingStandAlone()
{
    return _isStandAloneFunscriptPlaying;
}
bool SyncHandler::load(QString funscript)
{
    clear();
    return _funscriptHandler->load(funscript);
}

bool SyncHandler::load(QByteArray funscript)
{
    clear();
    return _funscriptHandler->load(funscript);
}

bool SyncHandler::loadMFS(QString channel, QString funscript)
{
    FunscriptHandler* otherFunscript = new FunscriptHandler(channel);
    if(!otherFunscript->load(funscript))
        return false;
    else
        _funscriptHandlers.append(otherFunscript);
    return true;
}

bool SyncHandler::loadMFS(QString channel, QByteArray funscript)
{
    FunscriptHandler* otherFunscript = new FunscriptHandler(channel);
    if(!otherFunscript->load(funscript))
        return false;
    else
        _funscriptHandlers.append(otherFunscript);
    return true;
}

bool SyncHandler::isLoaded()
{
    return _funscriptHandler->isLoaded();
}

void SyncHandler::stopAll()
{
    stopStandAlone();
    stopMediaFunscript();
}

void SyncHandler::stopStandAlone()
{
    LogHandler::Debug("Stop standalone sync");
    _currentTime = 0;
    _isStandAloneFunscriptPlaying = false;
    if(_funscriptStandAloneFuture.isRunning())
    {
        _funscriptStandAloneFuture.cancel();
        _funscriptStandAloneFuture.waitForFinished();
        emit funscriptStopped();
    }
}

void SyncHandler::stopMediaFunscript()
{
    LogHandler::Debug("Stop media sync");
    _isFunscriptPlaying = false;
    if(_funscriptFuture.isRunning())
    {
        _funscriptFuture.cancel();
        _funscriptFuture.waitForFinished();
    }
}

void SyncHandler::clear()
{
    LogHandler::Debug("Clear sync");
    _funscriptHandler->setLoaded(false);
    if(_funscriptHandlers.length() > 0)
    {
        qDeleteAll(_funscriptHandlers);
        _funscriptHandlers.clear();
    }
    _currentTime = 0;
}

void SyncHandler::reset()
{
    LogHandler::Debug("Reset sync");
    stopAll();
    clear();
}

QString SyncHandler::getPlayingStandAloneScript()
{
    return _playingStandAloneFunscript;
}
void SyncHandler::playFunscript(QString funscript) {
    LogHandler::Debug("play Funscript stand alone start thread");
    if(_isFunscriptPlaying)
        stopAll();
    load(funscript);
    _isFunscriptPlaying = true;
    _isStandAloneFunscriptPlaying = true;
    _playingStandAloneFunscript = funscript;
    emit funscriptStarted();
    _funscriptStandAloneFuture = QtConcurrent::run([this]()
    {
        std::shared_ptr<FunscriptAction> actionPosition;
        QMap<QString, std::shared_ptr<FunscriptAction>> otherActions;
        int secCounter1 = 0;
        int secCounter2 = 0;
        QElapsedTimer mSecTimer;
        qint64 timer1 = 0;
        qint64 timer2 = 0;
        mSecTimer.start();
        while (_isStandAloneFunscriptPlaying)
        {
            if (timer2 - timer1 >= 1)
            {
                timer1 = timer2;
                if(!_isPaused && !SettingsHandler::getLiveActionPaused() && _xSettings->isDeviceConnected())
                {
                    if(_seekTime > -1)
                    {
                        _currentTime = _seekTime;
                    }
                    else
                    {
                        _currentTime++;
                    }
                    actionPosition = _funscriptHandler->getPosition(_currentTime);
                    if(actionPosition != nullptr)
                        _xSettings->setAxisProgressBar(TCodeChannelLookup::Stroke(), actionPosition->pos);
                    foreach(auto funscriptHandlerOther, _funscriptHandlers)
                    {
                        auto otherAction = funscriptHandlerOther->getPosition(_currentTime);
                        if(otherAction != nullptr)
                        {
                            otherActions.insert(funscriptHandlerOther->channel(), otherAction);
                            _xSettings->setAxisProgressBar(funscriptHandlerOther->channel(), otherAction->pos);
                        }
                    }
                    QString tcode = _tcodeHandler->funscriptToTCode(actionPosition, otherActions);
                    if(tcode != nullptr)
                        _xSettings->getSelectedDeviceHandler()->sendTCode(tcode);
                    otherActions.clear();
                }
                secCounter2 = round(mSecTimer.elapsed() / 1000);
                if(secCounter2 - secCounter1 >= 1)
                {
                    if(_seekTime == -1)
                        emit funscriptPositionChanged(_currentTime);
                    secCounter1 = secCounter2;
                }
                if(_seekTime > -1)
                    _seekTime = -1;
            }
            timer2 = (round(mSecTimer.nsecsElapsed() / 1000000));
            if(_currentTime >= getFunscriptMax())
            {
                _isStandAloneFunscriptPlaying = false;
                emit funscriptStatusChanged(QtAV::MediaStatus::EndOfMedia);
            }
        }
        _isFunscriptPlaying = false;
        _playingStandAloneFunscript = nullptr;
        _xSettings->resetAxisProgressBars();
        _currentTime = 0;
        LogHandler::Debug("exit play Funscript stand alone thread");
    });
}

void SyncHandler::setFunscriptTime(qint64 msecs)
{
    _currentTime = msecs;
}

qint64 SyncHandler::getFunscriptTime()
{
    return _currentTime;
}

qint64 SyncHandler::getFunscriptMin()
{
    return _funscriptHandler->getMin();
}

qint64 SyncHandler::getFunscriptMax()
{
    return _funscriptHandler->getMax();
}

void SyncHandler::syncVRFunscript()
{
    LogHandler::Debug("syncVRFunscript start thread");
    stopAll();
    _isFunscriptPlaying = true;
    _funscriptFuture = QtConcurrent::run([this]()
    {
        if(_videoHandler->isPlaying())
        {
            //on_media_stop();
            _funscriptHandler->setLoaded(false);
        }
        QList<FunscriptHandler*> funscriptHandlers;
        std::shared_ptr<FunscriptAction> actionPosition;
        QMap<QString, std::shared_ptr<FunscriptAction>> otherActions;
        VRPacket currentVRPacket = _xSettings->getConnectedVRHandler()->getCurrentPacket();
        QString currentVideo;
        qint64 timeTracker = 0;
        qint64 lastVRTime = 0;
        QElapsedTimer mSecTimer;
        qint64 timer1 = 0;
        qint64 timer2 = 0;
        bool deviceHomed = false;
        //qint64 elapsedTracker = 0;
    //    QElapsedTimer timer;
    //    timer.start();
        mSecTimer.start();
        while (_isFunscriptPlaying && _xSettings->getConnectedVRHandler()->isConnected() && !_videoHandler->isPlaying())
        {
            //timer.start();
            if(!_isPaused && !SettingsHandler::getLiveActionPaused() && _xSettings->isDeviceConnected() && _funscriptHandler->isLoaded() && !currentVRPacket.path.isEmpty() && currentVRPacket.duration > 0 && currentVRPacket.playing)
            {
                //execute once every millisecond
                if (timer2 - timer1 >= 1)
                {
    //                LogHandler::Debug("timer1: "+QString::number(timer1));
    //                LogHandler::Debug("timer2: "+QString::number(timer2));
                    //LogHandler::Debug("timer2 - timer1 "+QString::number(timer2-timer1));
    //                LogHandler::Debug("Out timeTracker: "+QString::number(timeTracker));
                    timer1 = timer2;
                    qint64 currentTime = currentVRPacket.currentTime;
                    //LogHandler::Debug("VR time reset: "+QString::number(currentTime));
                    bool hasRewind = lastVRTime > currentTime;
                    if (currentTime > timeTracker + 100 || hasRewind)
                    {
                        lastVRTime = currentTime;
//                        LogHandler::Debug("current time reset: " + QString::number(currentTime));
//                        LogHandler::Debug("timeTracker: " + QString::number(timeTracker));
                        timeTracker = currentTime;
                    }
                    else
                    {
                        timeTracker++;
                        currentTime = timeTracker;
                    }
                    //LogHandler::Debug("funscriptHandler->getPosition: "+QString::number(currentTime));
                    actionPosition = _funscriptHandler->getPosition(currentTime);
                    if(actionPosition != nullptr) {
                        _xSettings->setAxisProgressBar(TCodeChannelLookup::Stroke(), actionPosition->pos);
//                        LogHandler::Debug("actionPosition != nullptr/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////pos: "+QString::number(actionPosition->pos));
//                        LogHandler::Debug("actionPosition != nullptr/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////speed: "+QString::number(actionPosition->speed));
//                        LogHandler::Debug("actionPosition != nullptr/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////lastPos: "+QString::number(actionPosition->lastPos));
//                        LogHandler::Debug("actionPosition != nullptr/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////lastSpeed: "+QString::number(actionPosition->lastSpeed));
                    }
                    foreach(auto funscriptHandlerOther, funscriptHandlers)
                    {
                        auto otherAction = funscriptHandlerOther->getPosition(currentTime);
                        if(otherAction != nullptr)
                        {
                            otherActions.insert(funscriptHandlerOther->channel(), otherAction);
                            _xSettings->setAxisProgressBar(funscriptHandlerOther->channel(), otherAction->pos);
                        }
                    }
                    QString tcode = _tcodeHandler->funscriptToTCode(actionPosition, otherActions);
                    if(tcode != nullptr)
                        _xSettings->getSelectedDeviceHandler()->sendTCode(tcode);
                    otherActions.clear();
               /*     LogHandler::Debug("timer "+QString::number((round(timer.nsecsElapsed()) / 1000000)));
                    timer.start()*/;
                }
                timer2 = (round(mSecTimer.nsecsElapsed() / 1000000));
                //LogHandler::Debug("timer nsecsElapsed: "+QString::number(timer2));
            }
            else if(!currentVRPacket.path.isEmpty() && !_funscriptHandler->isLoaded() && _xSettings->isDeviceConnected() && currentVRPacket.duration > 0 && currentVRPacket.playing)
            {
                //LogHandler::Debug("Enter syncDeoFunscript load funscript");
                QString funscriptPath = SettingsHandler::getDeoDnlaFunscript(currentVRPacket.path);
                currentVideo = currentVRPacket.path;
                if(!funscriptPath.isEmpty())
                {
                    QFileInfo fileInfo(funscriptPath);
                    if(fileInfo.exists())
                    {
                        _funscriptHandler->load(funscriptPath);

                        qDeleteAll(funscriptHandlers);
                        funscriptHandlers.clear();
                        if(!deviceHomed)
                        {
                            deviceHomed = true;
                            _xSettings->getSelectedDeviceHandler()->sendTCode(_tcodeHandler->getRunningHome());
                        }

                        auto availibleAxis = SettingsHandler::getAvailableAxis();
                        foreach(auto axisName, availibleAxis->keys())
                        {
                            auto trackName = availibleAxis->value(axisName).TrackName;
                            if(axisName == TCodeChannelLookup::Stroke() || trackName.isEmpty())
                                continue;
                            QString funscriptPathTemp = funscriptPath;
                            auto funscriptNoExtension = funscriptPathTemp.remove(funscriptPathTemp.lastIndexOf('.'), funscriptPathTemp.length() -  1);
                            QFileInfo fileInfo(funscriptNoExtension + "." + trackName + ".funscript");
                            if(fileInfo.exists())
                            {
                                FunscriptHandler* otherFunscript = new FunscriptHandler(axisName);
                                otherFunscript->load(fileInfo.absoluteFilePath());
                                funscriptHandlers.append(otherFunscript);
                            }
                        }
                    }
                }
            }

            if(currentVideo != currentVRPacket.path)
            {
                LogHandler::Debug("Enter syncDeoFunscript change funscript");
                currentVideo = currentVRPacket.path;
                _funscriptHandler->setLoaded(false);
            }

            //LogHandler::Debug("Get deo packet: "+QString::number((round(timer.nsecsElapsed()) / 1000000)));
            currentVRPacket = _xSettings->getConnectedVRHandler()->getCurrentPacket();
            //QThread::currentThread()->usleep(10);
            //LogHandler::Debug("After get deo packet: "+QString::number((round(timer.nsecsElapsed()) / 1000000)));
            //QThread::currentThread()->msleep(1);
        }

        _isFunscriptPlaying = false;
        _xSettings->resetAxisProgressBars();
        LogHandler::Debug("exit syncVRFunscript");
    });
}

void SyncHandler::syncFunscript()
{
    LogHandler::Debug("syncFunscript start thread");
    stopAll();
    _isFunscriptPlaying = true;

    emit funscriptStatusChanged(QtAV::MediaStatus::LoadedMedia);
    _funscriptFuture = QtConcurrent::run([this]()
    {
        std::shared_ptr<FunscriptAction> actionPosition;
        QMap<QString, std::shared_ptr<FunscriptAction>> otherActions;
        QElapsedTimer mSecTimer;
        qint64 timer1 = 0;
        qint64 timer2 = 0;
        mSecTimer.start();
        while (_isFunscriptPlaying && _videoHandler->isPlaying())
        {
            if (timer2 - timer1 >= 1)
            {
                timer1 = timer2;
                if(!_isPaused && !SettingsHandler::getLiveActionPaused() && _xSettings->isDeviceConnected())
                {
                    qint64 currentTime = _videoHandler->position();
                    actionPosition = _funscriptHandler->getPosition(currentTime);
                    if(actionPosition != nullptr)
                        _xSettings->setAxisProgressBar(TCodeChannelLookup::Stroke(), actionPosition->pos);
                    foreach(auto funscriptHandlerOther, _funscriptHandlers)
                    {
                        auto otherAction = funscriptHandlerOther->getPosition(currentTime);
                        if(otherAction != nullptr)
                        {
                            otherActions.insert(funscriptHandlerOther->channel(), otherAction);
                            _xSettings->setAxisProgressBar(funscriptHandlerOther->channel(), otherAction->pos);
                        }
                    }
                    QString tcode = _tcodeHandler->funscriptToTCode(actionPosition, otherActions);
                    if(tcode != nullptr)
                        _xSettings->getSelectedDeviceHandler()->sendTCode(tcode);
                    otherActions.clear();
                }
            }
            timer2 = (round(mSecTimer.nsecsElapsed() / 1000000));
        }

        _isFunscriptPlaying = false;
        _xSettings->resetAxisProgressBars();
        LogHandler::Debug("exit syncFunscript");
    });
}
