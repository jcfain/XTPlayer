#include "xvideopreviewwidget.h"

XVideoPreviewWidget::XVideoPreviewWidget(QWidget* parent) : QWidget(parent)
{
    _thumbPlayer = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
    _thumbNailVideoSurface = new XVideoSurface(this);
    _thumbPlayer->setVideoOutput(_thumbNailVideoSurface);
    _thumbPlayer->setMuted(true);
    connect(_thumbNailVideoSurface, &XVideoSurface::fnSurfaceStopped, this, &XVideoPreviewWidget::on_thumbCapture);
    //connect(_thumbPlayer, &QMediaPlayer::mediaStatusChanged, this, &XVideoPreviewWidget::on_mediaStatusChanged);
    connect(_thumbPlayer, &QMediaPlayer::stateChanged, this, &XVideoPreviewWidget::on_mediaStateChange);
    _layout = new QGridLayout(this);
    _label = new QLabel(this);
    _label->setScaledContents(true);
    setLayout(_layout);
    _layout->addWidget(_label);
}

void XVideoPreviewWidget::setFile(QString path) {
    _file = path;
}
void XVideoPreviewWidget::setTimestamp(qint64 pos) {
    _time = pos;
}
void XVideoPreviewWidget::preview() {
    getThumb(_file, _time);
}

void XVideoPreviewWidget::closeEvent(QCloseEvent *event) {
  QWidget::closeEvent(event);
}

void XVideoPreviewWidget::getThumb(QString videoPath, qint64 time) {
    if(!_thumbRetrieving) {
        _thumbRetrieving = true;
        QUrl mediaUrl = QUrl::fromLocalFile(videoPath);
        QMediaContent mc(mediaUrl);
        _thumbPlayer->setMedia(mc);
        _thumbPlayer->setPosition(_time);
        _thumbPlayer->play();
    }
}

void XVideoPreviewWidget::on_thumbCapture(QPixmap thumb) {
    _label->setPixmap(thumb);
    _label->update();
    _thumbPlayer->stop();
    _thumbRetrieving = false;
}

void XVideoPreviewWidget::on_mediaStatusChanged(QMediaPlayer::MediaStatus status) {
//    if(status == QMediaPlayer::MediaStatus::LoadedMedia) {
//    }
}
void XVideoPreviewWidget::on_mediaStateChange(QMediaPlayer::State state) {
    if(state == QMediaPlayer::State::PlayingState) {

    } else if(state == QMediaPlayer::State::StoppedState) {
        _thumbNailVideoSurface->fnClearPixmap();
    }
}
