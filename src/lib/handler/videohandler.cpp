#include "videohandler.h"
#include <QOpenGLWidget>

VideoHandler::VideoHandler(QWidget *parent) : QWidget(parent)
{
    //new QOpenGLWidget(this);
    layout = new QHBoxLayout;
    player = new AVPlayer(this);
    videoRenderer = new VideoOutput(this);
    //videoRenderer = new VideoOutput(QtAV::VideoRendererId_GLWidget2, this);
    //QtAV::Widgets::registerRenderers();
    if (!videoRenderer || !videoRenderer->isAvailable() || !videoRenderer->widget()) {
        LogHandler::Dialog("QtAV Video renderer is  not availabe on your platform!", XLogLevel::Critical);
        return;
    }
    player->setRenderer(videoRenderer);
    player->audio()->setVolume(SettingsHandler::playerVolume);
    layout->addWidget(videoRenderer->widget());
    setLayout(layout);
}

VideoHandler::~VideoHandler()
{
    delete player;
    delete videoRenderer;
}

void VideoHandler::mouseDoubleClickEvent(QMouseEvent * e)
{
    emit doubleClicked(e);
}
