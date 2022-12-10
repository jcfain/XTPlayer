#include "videohandler.h"
VideoHandler::VideoHandler(QWidget *parent) : QWidget(parent),
    _player(0), _fullscreenWidget(0), _videoWidget(0)
{
    _parent = parent;
    _player = new QMediaPlayer(this);
    _player->setVolume(SettingsHandler::getPlayerVolume());
    connect(_player, &QMediaPlayer::positionChanged, this, &VideoHandler::on_media_positionChanged, Qt::QueuedConnection);
    connect(_player, &QMediaPlayer::mediaStatusChanged, this, &VideoHandler::on_media_statusChanged, Qt::QueuedConnection);
    connect(_player, &QMediaPlayer::stateChanged, this, &VideoHandler::on_media_stateChanged, Qt::QueuedConnection);
    connect(_player, &QMediaPlayer::durationChanged, this, &VideoHandler::durationChange, Qt::QueuedConnection);
    //connect(_player, &QMediaPlayer::error, this, &VideoHandler::on_media_error, Qt::QueuedConnection);

//    m_audioProbe = new QAudioProbe(this);
//    m_audioProbe->setSource(_player);

//    connect(m_audioProbe, &QAudioProbe::audioBufferProbed, this, &VideoHandler::audioBufferProbed);

    setMinimumHeight(SettingsHandler::getThumbSize());
    setMinimumWidth(SettingsHandler::getThumbSize());
    createLayout();
}

void VideoHandler::createLayout()
{
    setContentsMargins(contentsMargins().left(), contentsMargins().top(), contentsMargins().right(), 0);
    LogHandler::Debug("Create player");
    if(_mediaGrid)
        delete _mediaGrid;
    _mediaGrid = new QGridLayout(this);
    setContentsMargins(contentsMargins().left(), contentsMargins().top(), contentsMargins().right(), 0);

    _mediaGrid->setMargin(0);
    _mediaGrid->setContentsMargins(0,0,0,0);
    setLayout(_mediaGrid);

    if(_videoLoadingMovie)
        delete _videoLoadingMovie;
    if(_videoLoadingLabel)
        delete _videoLoadingLabel;
    _videoLoadingLabel = new QLabel(this);
    _videoLoadingMovie = new QMovie("://images/Eclipse-1s-loading-200px.gif", nullptr, _videoLoadingLabel);
    _videoLoadingMovie->setProperty("cssClass", "mediaLoadingSpinner");
    _videoLoadingLabel->setMovie(_videoLoadingMovie);
    _videoLoadingLabel->setAttribute(Qt::WA_TransparentForMouseEvents );
    _videoLoadingLabel->setMaximumSize(200,200);
    //_videoLoadingLabel->setStyleSheet("* {background: ffffff}");
    _videoLoadingLabel->setProperty("cssClass", "mediaLoadingSpinner");
    _videoLoadingLabel->setAlignment(Qt::AlignCenter);
    //_mediaGrid->addWidget(_videoLoadingLabel, 0, 0);
    setLoading(false);
    if(_videoWidget)
        delete _videoWidget;
    _videoWidget = new XVideoWidget(this);
    connect(_videoWidget, &XVideoWidget::doubleClicked, this, [this](QMouseEvent* e) {emit doubleClicked(e);});
    connect(_videoWidget, &XVideoWidget::singleClicked, this, [this](QMouseEvent* e) {emit singleClicked(e);});
    connect(_videoWidget, &XVideoWidget::keyPressed, this, [this](QKeyEvent* e) {emit keyPressed(e);});
    connect(_videoWidget, &XVideoWidget::keyReleased, this, [this](QKeyEvent* e) {emit keyReleased(e);});
    connect(_videoWidget, &XVideoWidget::mouseEnter, this, [this](QEvent* e) {emit mouseEnter(e);});
    _mediaGrid->addWidget(_videoWidget);
    _player->setVideoOutput(_videoWidget);
}

VideoHandler::~VideoHandler()
{
    delete _mediaGrid;
    delete _player;
    delete _videoWidget;
    if(_fullscreenWidget)
        delete _fullscreenWidget;
}
void VideoHandler::toggleFullscreen() {
    if(!_isFullScreen)
    {
        _isFullScreen = true;
        _fullscreenWidget = new QWidget();
        _fullscreenWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        QGridLayout* layout = new QGridLayout(_fullscreenWidget);
        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(0);
        _fullscreenWidget->setLayout(layout);
        layout->addWidget(_videoWidget);
        _fullscreenWidget->showFullScreen();
        grabKeyboard();
    }
    else
    {
        _isFullScreen = false;
        _mediaGrid->addWidget(_videoWidget);
        delete _fullscreenWidget;
        _fullscreenWidget = 0;
        releaseKeyboard();
    }
}

QString VideoHandler::file()
{
    return _currentFile;
}


void VideoHandler::on_media_positionChanged(qint64 position)
{
    //const QMutexLocker locker(&_mutex);
    emit positionChanged(position);
}

void VideoHandler::on_media_statusChanged(QMediaPlayer::MediaStatus status)
{
    auto xstatus = convertMediaStatus(status);
    emit mediaStatusChanged(xstatus);
    if(xstatus == XMediaStatus::InvalidMedia) {
        QString errorMessage = "Invalid media error. You may need to install system codecs.";
#if defined(Q_OS_WIN)
        errorMessage += "<br>You can use compatible DirectShow decoders such as<br>K-lite<br><a style='background-color:#D3D3D3;' href=\"https://www.codecguide.com/download_kl.htm\">https://www.codecguide.com/download_kl.htm</a><br>or<br>LAV Filters:<br><a style='background-color:#D3D3D3;' href=\"http://forum.doom9.org/showthread.php?t=156191\">http://forum.doom9.org/showthread.php?t=156191</a>";
#elif defined(Q_OS_MAC)
        errorMessage += "";
#elif defined(Q_OS_LINUX)
        errorMessage += "<br>You can use your distibutions gstreamer build.";
#endif
        errorMessage += "<br>You can see more information here:<br><a style='background-color:#D3D3D3;' href='https://wiki.qt.io/Qt_5.13_Multimedia_Backends'>https://wiki.qt.io/Qt_5.13_Multimedia_Backends</a>";
        DialogHandler::Dialog(this->parentWidget(), errorMessage);
    }
}

void VideoHandler::on_media_stateChanged(QMediaPlayer::State state)
{
    auto xstate = convertMediaState(state);
    if(xstate == XMediaState::Playing && _currentState != XMediaState::Paused) {
        on_media_start();
    } else if(xstate == XMediaState::Stopped) {
        on_media_stop();
    }
    _currentState = xstate;
    emit mediaStateChanged(xstate);
}
void VideoHandler::on_media_error(QMediaPlayer::Error error)
{
    LogHandler::Error("Media error: "+ _player->errorString());
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
    return _player ? _player->state() == QMediaPlayer::PlayingState || _player->state() == QMediaPlayer::PausedState : false;
}

void VideoHandler::load()
{
    _player->play();
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
    if(isPaused()) {
        _player->play();
    } else {
        _player->pause();
    }
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
    QUrl mediaUrl = QUrl::fromLocalFile(file);
    QMediaContent mc(mediaUrl);
    _player->setMedia(mc);
}

bool VideoHandler::isPaused()
{
    return _player->state() == QMediaPlayer::PausedState;
}

bool VideoHandler::isMute()
{
    return _player->isMuted();
}

void VideoHandler::toggleMute()
{
    if (!isMute())
    {
        volumeBeforeMute = _player->volume();
        _player->setMuted(true);
    }
    else
    {
        _player->setMuted(false);
        _player->setVolume(volumeBeforeMute);
    }
}

void VideoHandler::setVolume(int value)
{
    _player->setVolume(value);
}
void VideoHandler::setRepeat(int max)
{
    //_player->set(max);
}
XMediaState VideoHandler::state()
{
    return convertMediaState(_player->state());
}

void VideoHandler::setPosition(qint64 position)
{
    //const QMutexLocker locker(&_mutex);
    disconnect(_player, &QMediaPlayer::positionChanged, this, &VideoHandler::on_media_positionChanged);
    _player->setPosition(position);
    connect(_player, &QMediaPlayer::positionChanged, this, &VideoHandler::on_media_positionChanged, Qt::QueuedConnection);
}

void VideoHandler::seek(qint64 position)
{
    _player->setPosition(position);
}

void VideoHandler::setSpeed(qreal speed)
{
    _player->setPlaybackRate(speed);
}

qint64 VideoHandler::position()
{
    return _player->position();
}

qint64 VideoHandler::duration()
{
    return _player->duration();
}

QGridLayout* VideoHandler::layout()
{
    return _mediaGrid;
}

void VideoHandler::setLoading(bool loading)
{
    on_setLoading(loading);
}

void VideoHandler::on_setLoading(bool loading)
{
    if(loading && _videoLoadingMovie->state() != QMovie::MovieState::Running)
    {
        _videoLoadingLabel->show();
        _videoLoadingMovie->start();
    }
    else if(!loading && _videoLoadingMovie->state() == QMovie::MovieState::Running)
    {
        _videoLoadingLabel->hide();
        _videoLoadingMovie->stop();
    }
}

XMediaStatus VideoHandler::convertMediaStatus(QMediaPlayer::MediaStatus status) {
    switch(status) {
        case QMediaPlayer::MediaStatus::LoadingMedia:
            return XMediaStatus::LoadingMedia;
        case QMediaPlayer::MediaStatus::LoadedMedia:
            return XMediaStatus::LoadedMedia;
        case QMediaPlayer::MediaStatus::EndOfMedia:
            return XMediaStatus::EndOfMedia;
        case QMediaPlayer::MediaStatus::NoMedia:
            return XMediaStatus::NoMedia;
        case QMediaPlayer::MediaStatus::BufferingMedia:
            return XMediaStatus::BufferingMedia;
        case QMediaPlayer::MediaStatus::BufferedMedia:
            return XMediaStatus::BufferedMedia;
        case QMediaPlayer::MediaStatus::UnknownMediaStatus:
            return XMediaStatus::UnknownMediaStatus;
        case QMediaPlayer::MediaStatus::StalledMedia:
            return XMediaStatus::StalledMedia;
        case QMediaPlayer::MediaStatus::InvalidMedia:
            return XMediaStatus::InvalidMedia;
    }
}

XMediaState VideoHandler::convertMediaState(QMediaPlayer::State status) {
    switch(status) {
        case QMediaPlayer::State::PausedState:
            return XMediaState::Paused;
        case QMediaPlayer::State::PlayingState:
            return XMediaState::Playing;
        case QMediaPlayer::State::StoppedState:
            return XMediaState::Stopped;
    }
}


//void VideoHandler::audioBufferProbed(const QAudioBuffer &buffer) {
//    LogHandler::Debug("probe0");
//    //const quint16 *data = buffer->constData<quint16>();
//}
