#include "xvideopreview.h"

XVideoPreview::XVideoPreview(QObject* parent) : QObject(parent), _thumbPlayer(0)
{
    _thumbPlayer = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
    _thumbNailVideoSurface = new XVideoSurface(_thumbPlayer);
    _thumbPlayer->setVideoOutput(_thumbNailVideoSurface);
    _thumbPlayer->setMuted(true);
    connect(_thumbPlayer, &QMediaPlayer::stateChanged, this, &XVideoPreview::on_mediaStateChange);
    connect(_thumbPlayer, &QMediaPlayer::mediaStatusChanged, this, &XVideoPreview::on_mediaStatusChanged);
    connect(_thumbNailVideoSurface, &XVideoSurface::frameCapture, this, &XVideoPreview::on_thumbCapture);
    connect(_thumbNailVideoSurface, &XVideoSurface::frameCaptureError, this, &XVideoPreview::on_thumbError);
    connect(_thumbPlayer, &QMediaPlayer::durationChanged, this, &XVideoPreview::on_durationChanged);
}

void XVideoPreview::setUpThumbPlayer()
{
    LogHandler::Debug("setUpThumbPlayer");
}

void XVideoPreview::setUpInfoPlayer() {
    LogHandler::Debug("setUpThumbPlayer");
}

void XVideoPreview::tearDownPlayer()
{
    LogHandler::Debug("tearDownPlayer");
    if(_thumbPlayer)
    {
        delete _thumbPlayer;
        _thumbPlayer = 0;
    }
}

void XVideoPreview::extract(QString file, qint64 time)
{
    _file = file;
    if(_file.isNull()) {
        emit frameExtractionError("In valid file path.");
        return;
    }
    if(!QFile::exists(_file)) {
        emit frameExtractionError("File: "+file+" does not exist.");
        return;
    }
    _time = time;
    LogHandler::Debug("extract: "+ file);
    LogHandler::Debug("extract: "+ QString::number(time));
    QUrl mediaUrl = QUrl::fromLocalFile(file);
    QMediaContent mc(mediaUrl);
    _thumbPlayer->setMedia(mc);
    if(time > -1)
    {
        _loadingInfo = false;
        _extracting = true;
        _thumbPlayer->setPosition(_time);
    }
    _thumbPlayer->play();
}

void XVideoPreview::load(QString file)
{
    if(_file == file && _lastDuration > 0) {
        emit durationChanged(_lastDuration);
        return;
    } else {
        _lastDuration = 0;
    }
    LogHandler::Debug("load: "+ file);
    _loadingInfo = true;
    extract(file);
}

// Private
void XVideoPreview::on_thumbCapture(QPixmap frame)
{
    if(_extracting) {
        LogHandler::Debug("on_thumbCapture: "+ _file);
        _extracting = false;
        emit frameExtracted(frame);
    }
}

void XVideoPreview::on_thumbError(QString error)
{
    if(_extracting) {
        LogHandler::Debug("on_thumbError: "+ _file);
        emit frameExtractionError(error);
    }
}

void XVideoPreview::on_mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if(status == QMediaPlayer::MediaStatus::LoadedMedia) {
    }
}
void XVideoPreview::on_mediaStateChange(QMediaPlayer::State state)
{
    if(state == QMediaPlayer::State::PlayingState)
    {
        //_thumbPlayer->pause();
    }
    else if(state == QMediaPlayer::State::StoppedState)
    {
        _thumbNailVideoSurface->fnClearPixmap();
    }
}

void XVideoPreview::on_durationChanged(qint64 duration)
{
    if(_loadingInfo && duration > 0)
    {
        LogHandler::Debug("on_durationChanged: "+ _file);
        LogHandler::Debug("on_durationChanged: "+ QString::number(duration));
        _loadingInfo = false;
        _lastDuration = duration;
        emit durationChanged(duration);
    }
}
