#include "videohandler.h"
#include <QMediaMetaData>
VideoHandler::VideoHandler(PlayerControls* controls, XLibraryList* libraryList, XVideoPreviewWidget* videoPreview, QWidget *parent) : XWidget(parent),
    m_libraryListFrame(0),
    _player(0),
    _fullscreenWidget(0),
    _videoWidget(0),
    m_controls(controls),
    m_libraryList(libraryList),
    m_videoPreview(videoPreview)
{
    _player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    _player->setAudioOutput(m_audioOutput);
    setVolume(SettingsHandler::getPlayerVolume());
    connect(_player, &QMediaPlayer::positionChanged, this, &VideoHandler::on_media_positionChanged, Qt::QueuedConnection);
    connect(_player, &QMediaPlayer::mediaStatusChanged, this, &VideoHandler::on_media_statusChanged, Qt::QueuedConnection);
    connect(_player, &QMediaPlayer::playbackStateChanged, this, &VideoHandler::on_media_stateChanged, Qt::QueuedConnection);
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

    // _mediaGrid->setMargin(0);
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
    _videoWidget->setAttribute(Qt::WA_TransparentForMouseEvents );
    connect(this, &XWidget::doubleClicked, this, [this](QMouseEvent* e) {emit doubleClicked(e);});
    connect(this, &XWidget::singleClicked, this, [this](QMouseEvent* e) {emit singleClicked(e);});
    connect(this, &XWidget::keyPressed, this, [this](QKeyEvent* e) {emit keyPressed(e);});
    connect(this, &XWidget::keyReleased, this, [this](QKeyEvent* e) {emit keyReleased(e);});
    connect(this, &XWidget::mouseEnter, this, [this](QEnterEvent* e) {emit mouseEnter(e);});
    connect(&m_overlayTimer, &QTimer::timeout, this, [this]() {
        hideControlsTimeout();
        hideLibraryTimeout();
    });
    _mediaGrid->addWidget(_videoWidget);
    _player->setVideoOutput(_videoWidget);
}

void VideoHandler::mouseMove( QMouseEvent* e ) {
    if (_fullscreenWidget && _isFullScreen) {
//        LogHandler::Debug("x: "+QString::number(e->pos().x()));
//        LogHandler::Debug("y: "+QString::number(e->pos().y()));
//        LogHandler::Debug("m_libraryRect x: "+QString::number(m_libraryRect.x()));
//        LogHandler::Debug("m_libraryRect y: "+QString::number(m_libraryRect.y()));
//        LogHandler::Debug("m_controlsRect x: "+QString::number(m_controlsRect.x()));
//        LogHandler::Debug("m_controlsRect y: "+QString::number(m_controlsRect.y()));
//        if(m_controls) {
//            if (m_controlsRect.contains( e->pos())) {
//                showControls();
//            } else {
//                hideControls();
//            }
//        }

        // qApp->restoreOverrideCursor();
        qApp->setOverrideCursor(Qt::CursorShape::ArrowCursor);
        showControls();
        showLibrary();
        m_overlayTimer.start(3000);
//        if(m_libraryList) {
//            if (m_libraryRect.contains( e->pos())) {
//                showLibrary();
//            } else {
//                hideLibrary();
//            }
//        }
    }
}

bool VideoHandler::isFullScreen() {
    return _isFullScreen;
}
void VideoHandler::showNormal() {
    showLibrary();
    showControls();
    auto flags = m_videoPreview->windowFlags();
    m_videoPreview->setParent(this->parentWidget(), flags);
    _mediaGrid->addWidget(_videoWidget);
    m_controls->setProperty("cssClass", "windowedControls");
    m_controls->style()->unpolish(m_controls);
    m_controls->style()->polish(m_controls);
    m_libraryList->setProperty("cssClass", "windowedLibrary");
    m_libraryList->style()->unpolish(m_libraryList);
    m_libraryList->style()->polish(m_libraryList);
    m_libraryList->setMinimumSize(QSize(0, 0));
    m_libraryList->setMaximumSize(QSize(16777215, 16777215));
    releaseKeyboard();
    delete _fullscreenWidget;
    _fullscreenWidget = 0;
//    delete m_libraryListFrame;
    m_libraryListFrame = 0;
    _isFullScreen = false;
    // qApp->restoreOverrideCursor();
    qApp->setOverrideCursor(Qt::CursorShape::ArrowCursor);
}

void VideoHandler::showFullscreen(QSize screenSize, bool libraryWindowed) {
    _isFullScreen = true;
    m_screenSize = screenSize;
    _fullscreenWidget = new XWidget(this);
    _fullscreenWidget->setAttribute(Qt::WA_StyledBackground);
    _fullscreenWidget->setMouseTracking(true);
    _fullscreenWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Window);
    connect(_fullscreenWidget, &XWidget::mouseMove, this, &VideoHandler::mouseMove);
    connect(_fullscreenWidget, &XWidget::doubleClicked, this, [this](QMouseEvent* e) {emit doubleClicked(e);});
    connect(_fullscreenWidget, &XWidget::singleClicked, this, [this](QMouseEvent* e) {emit singleClicked(e);});
    connect(_fullscreenWidget, &XWidget::keyPressed, this, [this](QKeyEvent* e) {emit keyPressed(e);});
    connect(_fullscreenWidget, &XWidget::keyReleased, this, [this](QKeyEvent* e) {emit keyReleased(e);});
    connect(_fullscreenWidget, &XWidget::mouseEnter, this, [this](QEnterEvent* e) {emit mouseEnter(e);});
    //_fullscreenWidget->setStyleSheet("background-color: transparent; color: white;");
//    _fullscreenWidget->setProperty("cssClass", "fullScreenControls");
//    _fullscreenWidget->style()->unpolish(_fullscreenWidget);
//    _fullscreenWidget->style()->polish(_fullscreenWidget);

//    QGraphicsOpacityEffect* opacityFx = new QGraphicsOpacityEffect(_fullscreenWidget);
//    opacityFx->setOpacity(0.5);
//    _fullscreenWidget->setGraphicsEffect(opacityFx);
    auto flags = m_videoPreview->windowFlags();
    m_videoPreview->setParent(_fullscreenWidget, flags);
    QGridLayout* layout = new QGridLayout(_fullscreenWidget);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    // _fullscreenWidget->setLayout(layout);
    _fullscreenWidget->setProperty("cssClass", "fullScreenWidget");
    int rows = m_screenSize.height() / m_controls->height();
    layout->addWidget(_videoWidget, 0, 0, rows -1, 4);
    layout->setColumnStretch(0, 1);
    layout->addWidget(m_controls, rows - 1, 0, 1, 5);
    layout->columnMinimumWidth(0);
    m_controls->setProperty("cssClass", "fullScreenControls");
    m_controls->style()->unpolish(m_controls);
    m_controls->style()->polish(m_controls);
//    QGraphicsOpacityEffect* opacityFxControls = new QGraphicsOpacityEffect(m_controls);
//    opacityFxControls->setOpacity(0.5);
//    m_controls->setGraphicsEffect(opacityFxControls);

    //m_controls->setAttribute(Qt::WA_TranslucentBackground);
    m_controlsRect = QRect(0, m_screenSize.height() - m_controls->height(), m_screenSize.width() - 10, m_controls->height() - 10);
    placeLibraryList(libraryWindowed);
    hideControls();
    //_fullscreenWidget->show();
    //_fullscreenWidget->init();
    _fullscreenWidget->showFullScreen();
    grabKeyboard();
}

void VideoHandler::placeLibraryList(bool libraryWindowed) {
    m_libraryWindowed = libraryWindowed;
    if(_fullscreenWidget && !libraryWindowed) {
        m_libraryListFrame = new QFrame(_fullscreenWidget);
//        QGraphicsOpacityEffect* opacityFx = new QGraphicsOpacityEffect(_fullscreenWidget);
//        opacityFx->setOpacity(0.5);
//        m_libraryListFrame->setGraphicsEffect(opacityFx);
//        m_libraryList->setGraphicsEffect(opacityFx);
        auto libraryWidth = (SettingsHandler::getThumbSize() * 3) + ((SettingsHandler::getThumbSize() * 3) * 0.5);
        m_libraryListFrame->setFixedWidth(libraryWidth);
        auto libraryHeight = m_controlsRect.top();
        m_libraryListFrame->setFixedHeight(libraryHeight);
//        m_libraryListFrame->setProperty("cssClass", "fullScreenLibrary");
//        m_libraryListFrame->style()->unpolish(m_libraryListFrame);
//        m_libraryListFrame->style()->polish(m_libraryListFrame);
        // m_libraryList->setProperty("cssClass", "fullScreenLibrary");
        m_libraryList->style()->unpolish(m_libraryList);
        m_libraryList->style()->polish(m_libraryList);
        int rows = m_screenSize.height() / m_controls->height();
        auto layout = new QGridLayout(m_libraryListFrame);
        layout->addWidget(m_libraryList, 1, 0, 20, 12);
        m_libraryListFrame->setLayout(layout);
        ((QGridLayout*)_fullscreenWidget->layout())->addWidget(m_libraryListFrame, 0, 4, rows - 1, 1);
        m_libraryRect = QRect(m_screenSize.width() - libraryWidth, 0, libraryWidth-10, libraryHeight);
        hideLibrary();
    }
}

QGridLayout* VideoHandler::libraryListLayout() {
    if(m_libraryListFrame)
        return (QGridLayout*)m_libraryListFrame->layout();
    else
        return 0;
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

void VideoHandler::on_media_stateChanged(QMediaPlayer::PlaybackState state)
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
    LogHandler::Info("on_media_stop");
    _player->setSource(QUrl());
    emit stopped();
}

bool VideoHandler::isPlaying()
{
    return _player ? _player->playbackState() == QMediaPlayer::PlayingState || _player->playbackState() == QMediaPlayer::PausedState : false;
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
    // QMediaContent mc(mediaUrl);
    _player->setSource(mediaUrl);
}

void VideoHandler::setSubtitleTrack(int index)
{
    _player->setActiveSubtitleTrack(index);
}

QList<QString> VideoHandler::getSubtitleTracks()
{
    QList<QString> languages;
    const auto tracks = _player->subtitleTracks();
    for(const auto &track : tracks)
    {
        QString language = track.value(QMediaMetaData::Language).toString();
        if(!language.isEmpty())
        {
            languages.append(language);
        }
    }
    return languages;
}

bool VideoHandler::isPaused()
{
    return _player->playbackState() == QMediaPlayer::PausedState;
}

bool VideoHandler::isMute()
{
    return m_audioOutput->isMuted();
}

void VideoHandler::toggleMute()
{
    if (!isMute())
    {
        volumeBeforeMute = m_audioOutput->volume();
        m_audioOutput->setMuted(true);
    }
    else
    {
        m_audioOutput->setMuted(false);
        m_audioOutput->setVolume(volumeBeforeMute);
    }
}

void VideoHandler::setVolume(int value)
{
    float scaled = value;
    if(value > 0)
    {
        scaled = value / 100.0;
        // TODO: logrithmic scale as per Qt documentation.
        // auto xmin = 0.01;
        // auto xmax = 100.0;
        // auto X0 = 20;
        // auto X1 = 800;
        // auto b = (X1 - X0) / log(xmax / xmin) = 780/9.21;
        // a = 20 - 84.7 * (-4.6)= 410
    }
    m_audioOutput->setVolume(scaled);
}
void VideoHandler::setRepeat(int max)
{
    //_player->set(max);
}
XMediaState VideoHandler::state()
{
    return convertMediaState(_player->playbackState());
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
#if BUILD_QT5
        case QMediaPlayer::MediaStatus::UnknownMediaStatus:
            return XMediaStatus::UnknownMediaStatus;
#endif
        case QMediaPlayer::MediaStatus::StalledMedia:
            return XMediaStatus::StalledMedia;
        case QMediaPlayer::MediaStatus::InvalidMedia:
            return XMediaStatus::InvalidMedia;
    }

    return XMediaStatus::InvalidMedia;
}

#if BUILD_QT6
XMediaState VideoHandler::convertMediaState(QMediaPlayer::PlaybackState status) {
    switch(status) {
    case QMediaPlayer::PlaybackState::PausedState:
        return XMediaState::Paused;
    case QMediaPlayer::PlaybackState::PlayingState:
        return XMediaState::Playing;
    case QMediaPlayer::PlaybackState::StoppedState:
        return XMediaState::Stopped;
    }
    return XMediaState::Stopped;
}
#else
XMediaState VideoHandler::convertMediaState(QMediaPlayer::State status) {
    switch(status) {
        case QMediaPlayer::State::PausedState:
            return XMediaState::Paused;
        case QMediaPlayer::State::PlayingState:
            return XMediaState::Playing;
        case QMediaPlayer::State::StoppedState:
            return XMediaState::Stopped;
    }
    return XMediaState::Stopped;
}
#endif


void VideoHandler::hideControls()
{
    if (m_controls &&_isFullScreen)
    {
        m_controls->hide();
        if(m_libraryListFrame && m_libraryListFrame->isHidden())
            qApp->setOverrideCursor(Qt::BlankCursor);
    }
}

void VideoHandler::showControls()
{
    if (m_controls && _isFullScreen)
    {
        m_controls->show();
    }
}

void VideoHandler::hideLibrary()
{
    if (_isFullScreen && !m_libraryWindowed)
    {
        if(m_libraryListFrame)
            m_libraryListFrame->hide();
        if(m_controls && m_controls->isHidden())
            qApp->setOverrideCursor(Qt::BlankCursor);
    }
}

void VideoHandler::showLibrary()
{
    if (_isFullScreen || m_libraryWindowed)
    {
        if(m_libraryListFrame) {
            m_libraryListFrame->show();
            releaseKeyboard();
        }
    }
}

void VideoHandler::hideControlsTimeout() {
    if(m_controls) {
        auto cursorPos = _fullscreenWidget->mapFromGlobal(QCursor::pos());
        if (!m_controlsRect.contains(cursorPos)) {
            hideControls();
        }
    }
}
void VideoHandler::hideLibraryTimeout() {
    if(m_libraryList) {
        auto cursorPos = _fullscreenWidget->mapFromGlobal(QCursor::pos());
        if (!m_libraryRect.contains(cursorPos)) {
            hideLibrary();
            if(_isFullScreen)
                grabKeyboard();
        }
    }
}
//void VideoHandler::audioBufferProbed(const QAudioBuffer &buffer) {
//    LogHandler::Debug("probe0");
//    //const quint16 *data = buffer->constData<quint16>();
//}
