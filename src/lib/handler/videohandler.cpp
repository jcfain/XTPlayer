#include "videohandler.h"
VideoHandler::VideoHandler(QWidget *parent) : QWidget(parent)
{
    //QtAV::setLogLevel(QtAV::LogLevel::LogAll);
    //new QOpenGLWidget(this);
    widgetLayout = new QHBoxLayout;
    //QtAV::Widgets::registerRenderers();
    videoRenderer = new VideoOutput(this);
    //videoRenderer = new VideoOutput(QtAV::VideoRendererId_GLWidget2, this);
    if (!videoRenderer || !videoRenderer->isAvailable() || !videoRenderer->widget())
    {
        LogHandler::Dialog("QtAV Video renderer is not availabe on your platform!", XLogLevel::Critical);
        return;
    }
    player = new AVPlayer(videoRenderer->widget());
    player->setRenderer(videoRenderer);
    player->audio()->setVolume(SettingsHandler::getPlayerVolume());
    widgetLayout->addWidget(videoRenderer->widget());

    connect(player, &AVPlayer::positionChanged, this, &VideoHandler::on_media_positionChanged);
    connect(player, &AVPlayer::mediaStatusChanged, this, &VideoHandler::on_media_statusChanged);
    connect(player, &AVPlayer::started, this, &VideoHandler::on_media_start);
    connect(player, &AVPlayer::stopped, this, &VideoHandler::on_media_stop);

    setLayout(widgetLayout);
}

VideoHandler::~VideoHandler()
{
    delete widgetLayout;
    delete player;
    delete videoRenderer;
}

QString VideoHandler::file()
{
    return currentFile;
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
    return player->isPlaying();
}

void VideoHandler::play()
{
    player->play();
}

void VideoHandler::stop()
{
    player->stop();
}

void VideoHandler::togglePause()
{
    player->togglePause();
}

void VideoHandler::setFile(QString file)
{
    currentFile = file;
    player->setFile(file);
}

void VideoHandler::load()
{
    player->load();
}

bool VideoHandler::isPaused()
{
    return player->isPaused();
}

bool VideoHandler::isMute()
{
    return player->audio()->isMute();
}

qreal volumeBeforeMute;
void VideoHandler::toggleMute()
{
    if (!player->audio()->isMute())
    {
        volumeBeforeMute = player->audio()->volume();
        player->audio()->setMute(true);
    }
    else
    {
        player->audio()->setMute(false);
        player->audio()->setVolume(volumeBeforeMute);
    }
}

void VideoHandler::setVolume(int value)
{
    player->audio()->setVolume(value);
}

AVPlayer::State VideoHandler::state()
{
    return player->state();
}

void VideoHandler::setPosition(qint64 position)
{
    player->setPosition(position);
}

qint64 VideoHandler::position()
{
    return player->position();
}

qint64 VideoHandler::duration()
{
    return player->duration();
}

QHBoxLayout* VideoHandler::layout()
{
    return widgetLayout;
}
