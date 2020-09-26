#include "videohandler.h"
VideoHandler::VideoHandler(QWidget *parent) : QWidget(parent)
{
    //QtAV::setLogLevel(QtAV::LogLevel::LogAll);
    //new QOpenGLWidget(this);
    _widgetLayout = new QHBoxLayout;
    //QtAV::Widgets::registerRenderers();
    _videoRenderer = new VideoOutput(this);
    //videoRenderer = new VideoOutput(QtAV::VideoRendererId_GLWidget2, this);
    if (!_videoRenderer || !_videoRenderer->isAvailable() || !_videoRenderer->widget())
    {
        LogHandler::Dialog("QtAV Video renderer is not availabe on your platform!", XLogLevel::Critical);
        return;
    }

    _player = new AVPlayer(_videoRenderer->widget());
    _player->setRenderer(_videoRenderer);
    _player->audio()->setVolume(SettingsHandler::getPlayerVolume());
    _widgetLayout->addWidget(_videoRenderer->widget());

    _player->setSeekType(AccurateSeek);

    connect(_player, &AVPlayer::positionChanged, this, &VideoHandler::on_media_positionChanged);
    connect(_player, &AVPlayer::mediaStatusChanged, this, &VideoHandler::on_media_statusChanged);
    connect(_player, &AVPlayer::started, this, &VideoHandler::on_media_start);
    connect(_player, &AVPlayer::stopped, this, &VideoHandler::on_media_stop);

    setLayout(_widgetLayout);
}

VideoHandler::~VideoHandler()
{
    delete _widgetLayout;
    delete _player;
    delete _videoRenderer;
}

QString VideoHandler::file()
{
    return _currentFile;
}

void VideoHandler::mouseDoubleClickEvent(QMouseEvent * e)
{
    emit doubleClicked(e);
}

void VideoHandler::keyPressEvent(QKeyEvent * e)
{
    emit keyPressed(e);
}

void VideoHandler::enterEvent(QEvent * e)
{
    emit mouseEnter(e);
}

void VideoHandler::on_media_positionChanged(qint64 position)
{
    emit positionChanged(position);
}

void VideoHandler::on_media_statusChanged(MediaStatus status)
{
    emit mediaStatusChanged(status);
}

void VideoHandler::on_media_start()
{
    emit started();
}

void VideoHandler::on_media_stop()
{
    emit stopped();
}

bool VideoHandler::isPlaying()
{
    return _player->isPlaying();
}

void VideoHandler::play()
{
    _player->play();
}

void VideoHandler::stop()
{
    _player->stop();
}

void VideoHandler::togglePause()
{
    _player->togglePause();
    emit togglePaused(isPaused());
}

void VideoHandler::setFile(QString file)
{
    _currentFile = file;
    _player->setFile(file);
}

void VideoHandler::load()
{
    _player->load();
}

bool VideoHandler::isPaused()
{
    return _player->isPaused();
}

bool VideoHandler::isMute()
{
    return _player->audio()->isMute();
}

qreal volumeBeforeMute;
void VideoHandler::toggleMute()
{
    if (!_player->audio()->isMute())
    {
        volumeBeforeMute = _player->audio()->volume();
        _player->audio()->setMute(true);
    }
    else
    {
        _player->audio()->setMute(false);
        _player->audio()->setVolume(volumeBeforeMute);
    }
}

void VideoHandler::setVolume(int value)
{
    _player->audio()->setVolume(value);
}

AVPlayer::State VideoHandler::state()
{
    return _player->state();
}

void VideoHandler::setPosition(qint64 position)
{
    _player->setPosition(position);
}

void VideoHandler::seek(qint64 position)
{
    _player->seek(position);
}

qint64 VideoHandler::position()
{
    return _player->position();
}

qint64 VideoHandler::duration()
{
    return _player->duration();
}

QHBoxLayout* VideoHandler::layout()
{
    return _widgetLayout;
}
