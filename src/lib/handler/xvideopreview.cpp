#include "xvideopreview.h"

XVideoPreview::XVideoPreview(QObject* parent) : QObject(parent)
{
    _thumbPlayer = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
    _thumbNailVideoSurface = new XVideoSurface(this);
    _thumbPlayer->setVideoOutput(_thumbNailVideoSurface);
    _thumbPlayer->setMuted(true);
    connect(_thumbNailVideoSurface, &XVideoSurface::fnSurfaceStopped, this, &XVideoPreview::on_thumbCapture);
    connect(_thumbNailVideoSurface, &XVideoSurface::fnSurfaceError, this, &XVideoPreview::on_thumbError);
    connect(_thumbPlayer, &QMediaPlayer::stateChanged, this, &XVideoPreview::on_mediaStateChange);
    connect(_thumbPlayer, &QMediaPlayer::mediaStatusChanged, this, &XVideoPreview::on_mediaStatusChanged);
    connect(_thumbPlayer, &QMediaPlayer::durationChanged, this, &XVideoPreview::on_durationChanged);
}

void XVideoPreview::extract(QString videoPath, qint64 time) {
    if(!_thumbRetrieving) {
        _time = time;
        _file = videoPath;
        _thumbRetrieving = true;
        QUrl mediaUrl = QUrl::fromLocalFile(videoPath);
        QMediaContent mc(mediaUrl);
        _thumbPlayer->setMedia(mc);
        _thumbPlayer->setPosition(_time);
        _thumbPlayer->play();
    }
}

void XVideoPreview::extractLoaded(qint64 time) {
    _time = time;
    if(_file.isNull()) {
        emit frameExtractionError("No file to extract thumb from.");
        return;
    }
    extract(_file, time);
    _file = nullptr;
}

void XVideoPreview::load(QString mediaPath) {
    if(!_thumbRetrieving) {
        _loadingInfo = true;
        _file = mediaPath;
        QUrl mediaUrl = QUrl::fromLocalFile(_file);
        QMediaContent mc(mediaUrl);
        _thumbPlayer->setMedia(mc);
        _thumbPlayer->play();
    }
}

// Private

void XVideoPreview::on_thumbCapture(QPixmap frame) {
    emit frameExtracted(frame);
    _thumbPlayer->stop();
    _thumbRetrieving = false;
}

void XVideoPreview::on_thumbError(QString error) {
    emit frameExtractionError(error);
}

void XVideoPreview::on_mediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if(status == QMediaPlayer::MediaStatus::LoadedMedia) {
        if(!_loadingInfo) {
        }
    }
}
void XVideoPreview::on_mediaStateChange(QMediaPlayer::State state) {
    if(state == QMediaPlayer::State::PlayingState) {
        //_thumbPlayer->pause();
    } else if(state == QMediaPlayer::State::StoppedState) {
        _thumbNailVideoSurface->fnClearPixmap();
    }
}

void XVideoPreview::on_durationChanged(qint64 duration) {
    if(_loadingInfo) {
        _thumbPlayer->stop();
        _loadingInfo = false;
    }
    emit durationChanged(duration);
}
