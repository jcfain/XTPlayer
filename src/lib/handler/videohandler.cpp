#include "videohandler.h"
VideoHandler::VideoHandler(QWidget *parent) : QWidget(parent),
    _videoRenderer(0)
{
    LogHandler::Debug("Create QHBoxLayout");
    _widgetLayout = new QHBoxLayout(this);
    _widgetLayout->setMargin(0);
    setContentsMargins(contentsMargins().left(), contentsMargins().top(), contentsMargins().right(), 0);
    setLayout(_widgetLayout);
    //QtAV::setLogLevel(QtAV::LogLevel::LogAll);
    //new QOpenGLWidget(this);
    //_videoRenderer = new VideoOutput(this);
    LogHandler::Debug("Create player");
    _player = new AVPlayer(this);
    //_player->setFrameRate(23.65);
    LogHandler::Debug("Create Renderer");
//    _videoRenderer = new VideoOutput(QtAV::VideoRendererId_GLWidget2, this);
//    if (!_videoRenderer || !_videoRenderer->isAvailable() || !_videoRenderer->widget())
//    {
//        LogHandler::Debug("OpenGLWidget failed, trying default");
//        _videoRenderer = new VideoOutput(this);
//        if (!_videoRenderer || !_videoRenderer->isAvailable() || !_videoRenderer->widget())
//        {
//            LogHandler::Dialog("QtAV Video renderer is not availabe on your platform!", XLogLevel::Critical);
//            return;
//        //}
//    }
    setVideoRenderer(SettingsHandler::getSelectedVideoRenderer());
    setDecoderPriority();
    //_player->setVideoDecoderPriority(QStringList() << "CUDA" << "D3D11" << "DXVA" << "VAAPI" << "VideoToolbox" << "FFmpeg");
    QVariantHash opt;
    QVariantHash cuda_opt;
    cuda_opt["surfaces"] = 20; //key is property name, case sensitive
    cuda_opt["copyMode"] = "DirectCopy"; // default is "DirectCopy"
    cuda_opt["flags"] = "Default";
    cuda_opt["deinterlace"] = "Adaptive";
    opt["CUDA"] = cuda_opt; //key is decoder name, case sensitive

    QVariantHash va_opt;
    va_opt["display"] = "X11"; //"GLX", "X11", "DRM"
    va_opt["copyMode"] = "ZeroCopy"; // "ZeroCopy", "OptimizedCopy", "GenericCopy". Default is "ZeroCopy" if possible
    opt["VAAPI"] = va_opt; //key is decoder name, case sensitive

    QVariantHash avfmt_opt;
    avfmt_opt["probesize"] = 4096;
    avfmt_opt["user_agent"] = "xxx";
    opt["avformat"] = avfmt_opt;
    _player->setOptionsForVideoCodec(opt);

    _player->setBufferMode(QtAV::BufferMode::BufferBytes);

    _player->audio()->setVolume(SettingsHandler::getPlayerVolume() * 0.001f);
    _player->setSeekType(QtAV::SeekType::KeyFrameSeek);

    connect(_player, &AVPlayer::positionChanged, this, &VideoHandler::on_media_positionChanged, Qt::QueuedConnection);
    connect(_player, &AVPlayer::mediaStatusChanged, this, &VideoHandler::on_media_statusChanged, Qt::QueuedConnection);
    connect(_player, &AVPlayer::started, this, &VideoHandler::on_media_start, Qt::QueuedConnection);
    connect(_player, &AVPlayer::stopped, this, &VideoHandler::on_media_stop, Qt::QueuedConnection);

    setMinimumHeight(SettingsHandler::getThumbSize());
    setMinimumWidth(SettingsHandler::getThumbSize());
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
void VideoHandler::mousePressEvent(QMouseEvent * e)
{
    if(e->button() == Qt::MouseButton::RightButton)
        emit rightClicked(e);
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
    //const QMutexLocker locker(&_mutex);
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
    emit playing();
}

void VideoHandler::stop()
{
    _player->stop();
    emit stopping();
}

void VideoHandler::togglePause()
{
    _player->togglePause();
    emit togglePaused(isPaused());
}

void VideoHandler::pause()
{
    _player->pause();
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

void VideoHandler::toggleMute()
{
    if (!isMute())
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
    _player->audio()->setVolume(value * 0.001f);
}
void VideoHandler::setRepeat(int max)
{
    _player->setRepeat(max);
}
AVPlayer::State VideoHandler::state()
{
    return _player->state();
}

void VideoHandler::setPosition(qint64 position)
{
    //const QMutexLocker locker(&_mutex);
    disconnect(_player, &AVPlayer::positionChanged, this, &VideoHandler::on_media_positionChanged);
    _player->setPosition(position);
    connect(_player, &AVPlayer::positionChanged, this, &VideoHandler::on_media_positionChanged, Qt::QueuedConnection);
}

void VideoHandler::seek(qint64 position)
{
    _player->seek(position);
}

void VideoHandler::setSpeed(qreal speed)
{
    _player->setSpeed(speed);
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

void VideoHandler::setDecoderPriority()
{
    bool wasPlaying = false;
    if(isPlaying())
    {
        wasPlaying = true;
        stop();
    }
    QStringList stringList;
    QList<DecoderModel> models = SettingsHandler::getDecoderPriority();
    foreach (auto model, models)
        if(model.Enabled)
            stringList.append(model.Name);
    _player->setVideoDecoderPriority(stringList);
    if(wasPlaying)
        play();
}
bool VideoHandler::setVideoRenderer(XVideoRenderer renderer)
{
    QtAV::Widgets::registerRenderers();
    QtAV::VideoRendererId vid = QtAVVideoRendererIdMap.value(renderer);
    VideoRenderer *videoRenderer = new VideoOutput(vid, this);
    if (!videoRenderer || !videoRenderer->isAvailable() || !videoRenderer->widget())
    {
//        LogHandler::Debug(" '" + XVideoRendererReverseMap.value(renderer) + "' failed, trying default");
//        _videoRenderer = new VideoOutput(this);
//        if (!_videoRenderer || !_videoRenderer->isAvailable() || !_videoRenderer->widget())
//        {
            LogHandler::Dialog("QtAV Video renderer '" + XVideoRendererReverseMap.value(renderer) + "' is not availabe on your platform!", XLogLevel::Critical);
            return false;
//        }
    }

    if (_videoRenderer && _videoRenderer->widget())
    {
        _widgetLayout->removeWidget(_videoRenderer->widget());
        _videoRenderer->widget()->close();
//        if (!_videoRenderer->widget()->testAttribute(Qt::WA_DeleteOnClose))
//        {
//            delete _videoRenderer->widget();
//        }
        delete _videoRenderer;
    }
    _videoRenderer = videoRenderer;
    _player->setRenderer(_videoRenderer);
    _widgetLayout->addWidget(_videoRenderer->widget());
    if (vid == VideoRendererId_GLWidget || vid == VideoRendererId_GLWidget2 || vid == VideoRendererId_OpenGLWidget || vid == VideoRendererId_XV)
    {
        _player->renderer()->forcePreferredPixelFormat(true);
    }
    else
    {
        _player->renderer()->forcePreferredPixelFormat(false);
    }
    return true;
}

void VideoHandler::installFilter(AudioFilter* filter)
{
    _player->installFilter(filter);
}

void VideoHandler::clearFilters()
{
    qDeleteAll(_player->videoFilters());
}
