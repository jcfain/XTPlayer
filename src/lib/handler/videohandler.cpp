#include "videohandler.h"
VideoHandler::VideoHandler(QWidget *parent) : QWidget(parent)
{
    //QtAV::setLogLevel(QtAV::LogLevel::LogAll);
    //new QOpenGLWidget(this);
    layout = new QHBoxLayout;
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
    layout->addWidget(videoRenderer->widget());
    setLayout(layout);
}

VideoHandler::~VideoHandler()
{
    delete layout;
    delete player;
    delete videoRenderer;
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

