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
    if((isLoaded() && _isMediaFunscriptPlaying) || (isLoaded() && _isStandAloneFunscriptPlaying))
    {
        QMutexLocker locker(&_mutex);
        _isPaused = !_isPaused;
        emit togglePaused(isPaused());
    }
}

void SyncHandler::setStandAloneLoop(bool enabled)
{
    QMutexLocker locker(&_mutex);
    _standAloneLoop = enabled;
}

bool SyncHandler::isPaused()
{
    return _isPaused;
}
bool SyncHandler::isPlayingStandAlone()
{
    return _isStandAloneFunscriptPlaying;
}

QList<QString> SyncHandler::load(QString scriptFile)
{
    LogHandler::Debug("Enter syncHandler load");
    reset();
    _xSettings->getSelectedDeviceHandler()->sendTCode(_tcodeHandler->getRunningHome());
    if(!scriptFile.isEmpty())
    {
        QFileInfo scriptInfo(scriptFile);
        if(scriptInfo.exists())
        {
            QString scriptTemp = scriptFile;
            QString scriptFileNoExtension = scriptTemp.remove(scriptTemp.lastIndexOf('.'), scriptTemp.length() -  1);
            QString fileName = scriptInfo.fileName();
            QString scriptNameNoExtension = fileName.remove(fileName.lastIndexOf('.'), scriptTemp.length() -  1);
            if(scriptFile.endsWith(".zip"))
            {
               QZipReader zipFile(scriptFile, QIODevice::ReadOnly);
               if(zipFile.isReadable())
               {
                   QByteArray data = zipFile.fileData(scriptNameNoExtension + ".funscript");
                   if (!data.isEmpty())
                   {
                       if(!load(data))
                       {
                           _invalidScripts.append("Zip file: " + scriptNameNoExtension + ".funscript");
                       }
                   }
                   else
                   {
                       LogHandler::Debug("Main funscript: '"+scriptNameNoExtension + ".funscript' not found in zip");
                   }
               }
            }
            else if(!_funscriptHandler->load(scriptFile))
            {
                _invalidScripts.append(scriptFile);
            }
            loadMFS(scriptFile);
        }
        else
        {
            _invalidScripts.append("File not found: " + scriptFile);
        }
    }
    return _invalidScripts;
}

bool SyncHandler::isLoaded()
{
    return _funscriptHandler->isLoaded() || _funscriptHandlers.length() > 0;
}

void SyncHandler::stopAll()
{
    stopStandAloneFunscript();
    stopMediaFunscript();
    stopVRFunscript();
}

void SyncHandler::stopStandAloneFunscript()
{
    LogHandler::Debug("Stop standalone sync");
    QMutexLocker locker(&_mutex);
    _currentTime = 0;
    _isStandAloneFunscriptPlaying = false;
    locker.unlock();
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
    QMutexLocker locker(&_mutex);
    _isMediaFunscriptPlaying = false;
    locker.unlock();
    if(_funscriptMediaFuture.isRunning())
    {
        _funscriptMediaFuture.cancel();
        _funscriptMediaFuture.waitForFinished();
    }
}

void SyncHandler::stopVRFunscript()
{
    LogHandler::Debug("Stop VR sync");
    QMutexLocker locker(&_mutex);
    _isVRFunscriptPlaying = false;
    locker.unlock();
    if(_funscriptVRFuture.isRunning())
    {
        _funscriptVRFuture.cancel();
        _funscriptVRFuture.waitForFinished();
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
    QMutexLocker locker(&_mutex);
    _currentTime = 0;
    _standAloneLoop = false;
    _isPaused = false;
    _invalidScripts.clear();
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

void SyncHandler::playStandAlone(QString funscript) {
    LogHandler::Debug("play Funscript stand alone start thread");
    if(!funscript.isEmpty()) //Override standalone funscript. aka: skip to moneyshot
        load(funscript);
    QMutexLocker locker(&_mutex);
    _currentTime = 0;
    _isPaused = false;
    _standAloneLoop = false;
    _isMediaFunscriptPlaying = true;
    _isStandAloneFunscriptPlaying = true;
    _playingStandAloneFunscript = funscript;
    locker.unlock();
    emit funscriptStarted();
    LogHandler::Debug("playStandAlone start thread");
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
        qint64 funscriptMax = getFunscriptMax();
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
                    if(_funscriptHandler->isLoaded())
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
                    if(_seekTime == -1 && !_videoHandler->isPlaying())
                        emit funscriptPositionChanged(_currentTime);
                    secCounter1 = secCounter2;
                }
                if(_seekTime > -1)
                    _seekTime = -1;
            }
            timer2 = (round(mSecTimer.nsecsElapsed() / 1000000));
            if(!_standAloneLoop && _currentTime >= funscriptMax)
            {
                _isStandAloneFunscriptPlaying = false;
                if(!_videoHandler->isPlaying())
                    emit funscriptStatusChanged(QtAV::MediaStatus::EndOfMedia);
            }
            else if(_standAloneLoop && _currentTime >= funscriptMax)
            {
                _currentTime = 0;
            }
        }
        QMutexLocker locker(&_mutex);
        _isMediaFunscriptPlaying = false;
        _playingStandAloneFunscript = nullptr;
        _xSettings->resetAxisProgressBars();
        _currentTime = 0;
        LogHandler::Debug("exit play Funscript stand alone thread");
    });
}

void SyncHandler::setFunscriptTime(qint64 msecs)
{
    QMutexLocker locker(&_mutex);
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
    if(_funscriptHandler->getMax() > -1)
        return _funscriptHandler->getMax();
    qint64 otherMax = -1;
    foreach(auto handler, _funscriptHandlers)
    {
        auto max = handler->getMax();
        if(max > -1 && max > otherMax)
            otherMax = max;
    }
    return otherMax;
}

void SyncHandler::syncFunscript()
{
    stopAll();
    QMutexLocker locker(&_mutex);
    _isMediaFunscriptPlaying = true;
    //emit funscriptStatusChanged(QtAV::MediaStatus::LoadedMedia);
    LogHandler::Debug("syncFunscript start thread");
    _funscriptMediaFuture = QtConcurrent::run([this]()
    {
        std::shared_ptr<FunscriptAction> actionPosition;
        QMap<QString, std::shared_ptr<FunscriptAction>> otherActions;
        QElapsedTimer mSecTimer;
        qint64 timer1 = 0;
        qint64 timer2 = 0;
        mSecTimer.start();
        while (_isMediaFunscriptPlaying && _videoHandler->isPlaying())
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

        _isMediaFunscriptPlaying = false;
        _xSettings->resetAxisProgressBars();
        LogHandler::Debug("exit syncFunscript");
    });
}

void SyncHandler::syncVRFunscript(QString funscript)
{
    load(funscript);
    QMutexLocker locker(&_mutex);
    _isVRFunscriptPlaying = true;
    LogHandler::Debug("syncVRFunscript start thread");
    _funscriptVRFuture = QtConcurrent::run([this]()
    {
        std::shared_ptr<FunscriptAction> actionPosition;
        QMap<QString, std::shared_ptr<FunscriptAction>> otherActions;
        VRPacket currentVRPacket;
        QElapsedTimer mSecTimer;
        qint64 timer1 = 0;
        qint64 timer2 = 0;
        mSecTimer.start();
        while (_isVRFunscriptPlaying && _xSettings->getConnectedVRHandler()->isConnected() && !_videoHandler->isPlaying())
        {
            currentVRPacket = _xSettings->getConnectedVRHandler()->getCurrentPacket();
            if(!_isPaused && !SettingsHandler::getLiveActionPaused() && _xSettings->isDeviceConnected() && isLoaded() && !currentVRPacket.path.isEmpty() && currentVRPacket.duration > 0 && currentVRPacket.playing)
            {
                //execute once every millisecond
                if (timer2 - timer1 >= 1)
                {
                    timer1 = timer2;
                    qint64 currentTime = currentVRPacket.currentTime;
                    //LogHandler::Debug("funscriptHandler->getPosition: "+QString::number(currentTime));
                    actionPosition = _funscriptHandler->getPosition(currentTime);
                    if(actionPosition != nullptr) {
                        _xSettings->setAxisProgressBar(TCodeChannelLookup::Stroke(), actionPosition->pos);
//                        LogHandler::Debug("actionPosition != nullptr/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////pos: "+QString::number(actionPosition->pos));
//                        LogHandler::Debug("actionPosition != nullptr/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////speed: "+QString::number(actionPosition->speed));
//                        LogHandler::Debug("actionPosition != nullptr/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////lastPos: "+QString::number(actionPosition->lastPos));
//                        LogHandler::Debug("actionPosition != nullptr/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////lastSpeed: "+QString::number(actionPosition->lastSpeed));
                    }
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
                timer2 = (round(mSecTimer.nsecsElapsed() / 1000000));
                //LogHandler::Debug("timer nsecsElapsed: "+QString::number(timer2));
            }
        }

        _xSettings->resetAxisProgressBars();
        QMutexLocker locker(&_mutex);
        _isVRFunscriptPlaying = false;
        LogHandler::Debug("exit syncVRFunscript");
    });
}

// Private
bool SyncHandler::load(QByteArray funscript)
{
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

void SyncHandler::loadMFS(QString scriptFile)
{
    QString scriptTemp = scriptFile;
    QString scriptFileNoExtension = scriptTemp.remove(scriptTemp.lastIndexOf('.'), scriptTemp.length() -  1);
    QFileInfo scriptFileInfo(scriptFile);
    QZipReader* zipFile;
    if(scriptFile.endsWith(".zip"))
        zipFile = new QZipReader(scriptFile, QIODevice::ReadOnly);

    auto availibleAxis = SettingsHandler::getAvailableAxis();
    foreach(auto axisName, availibleAxis->keys())
    {
        auto track = availibleAxis->value(axisName);
        if(axisName == TCodeChannelLookup::Stroke() || track.Type == AxisType::HalfRange || track.TrackName.isEmpty())
            continue;

        QFileInfo fileInfo(scriptFileNoExtension + "." + track.TrackName + ".funscript");
        if(fileInfo.exists())
        {
            LogHandler::Debug("Loading MFS track: "+ scriptFileNoExtension + "." + track.TrackName + ".funscript");
            if(!loadMFS(axisName, fileInfo.absoluteFilePath()))
                _invalidScripts.append("MFS script: " + fileInfo.absoluteFilePath());
        }
        else if(scriptFile.endsWith(".zip") && zipFile->isReadable())
        {
           QString fileName = scriptFileInfo.fileName();
           QString scriptFileNameNoExtension = fileName.remove(fileName.lastIndexOf('.'), scriptTemp.length() -  1);
           QString trackFileName = scriptFileNameNoExtension + "." + track.TrackName + ".funscript";
           QByteArray data = zipFile->fileData(trackFileName);
           if (!data.isEmpty())
           {
               LogHandler::Debug("Loading MFS track from zip: "+ trackFileName);
               if(!loadMFS(axisName, data))
                   _invalidScripts.append("MFS zip script: " + trackFileName);
           }
        }
    }
    if(scriptFile.endsWith(".zip"))
        delete zipFile;
}
