#include "videohandler.h"
#include <QMediaMetaData>
#include <QFileInfo>
#include <QVideoSink>
#include <QScreen>
#include <QGuiApplication>
#include <QWindow>
VideoHandler::VideoHandler(PlayerControls* controls, XLibraryList* libraryList, XVideoPreviewWidget* videoPreview, QWidget *parent) : XWidget(parent),
    m_libraryListFrame(0),
    _player(0),
    _videoView(0),
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
    setLoading(false);
    if(_videoView)
        delete _videoView;
    _videoView = new XVideoGraphicsView(this);
    _videoView->setPlayer(_player);
    _videoView->setAttribute(Qt::WA_TransparentForMouseEvents );

    // Top bar (media name) - shown only in full screen, overlaid on the video
    if(!m_topBar) {
        m_topBar = new QFrame(this);
        m_topBar->setProperty("cssClass", "fullScreenTopBar");
        QHBoxLayout* topLayout = new QHBoxLayout(m_topBar);
        topLayout->setContentsMargins(15, 5, 15, 5);
        m_topBarLabel = new QLabel(m_topBar);
        m_topBarLabel->setProperty("cssClass", "fullScreenTopBarLabel");
        topLayout->addWidget(m_topBarLabel);
        m_topBar->hide();
    }

    // Subtitle label - overlaid at the bottom of the video
    if(!m_subtitleLabel) {
        m_subtitleLabel = new QLabel(this);
        m_subtitleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_subtitleLabel->setAlignment(Qt::AlignHCenter);
        m_subtitleLabel->setProperty("cssClass", "videoSubtitle");
        m_subtitleLabel->hide();
    }

    connect(this, &XWidget::doubleClicked, this, [this](QMouseEvent* e) {emit doubleClicked(e);});
    connect(this, &XWidget::singleClicked, this, [this](QMouseEvent* e) {emit singleClicked(e);});
    connect(this, &XWidget::keyPressed, this, [this](QKeyEvent* e) {emit keyPressed(e);});
    connect(this, &XWidget::keyReleased, this, [this](QKeyEvent* e) {emit keyReleased(e);});
    connect(this, &XWidget::mouseEnter, this, [this](QEnterEvent* e) {emit mouseEnter(e);});
    connect(&m_overlayTimer, &QTimer::timeout, this, [this]() {
        hideControlsTimeout();
        hideLibraryTimeout();
    });

    // Subtitle tracking
#if BUILD_QT5
    connect(_player, &QMediaPlayer::positionChanged, this, &VideoHandler::updateSubtitle, Qt::QueuedConnection);
    connect(_player, &QMediaPlayer::subtitleFromActiveTrackChanged, this, [this]() {
        m_currentSubtitle = _player->subtitleFromActiveTrack();
        updateSubtitle();
    });
#else
    // Qt 6 pushes the active subtitle text to the video sink - render it
    // in the overlay label (same mechanism QVideoWidget uses internally).
    if(_videoView && _videoView->videoSink())
        connect(_videoView->videoSink(), &QVideoSink::subtitleTextChanged, this, [this](const QString &text) {
            if(!text.isEmpty()) {
                m_subtitleLabel->setText(text);
                m_subtitleLabel->show();
            } else {
                m_subtitleLabel->hide();
            }
        });
#endif

    setupViewLayout();
    _mediaGrid->addWidget(_videoView);
}

// Builds the layout on the video view. The video (scene) always fills the
// view; the widgets below are overlaid on top of it (like the web player's
// absolutely positioned controls).
void VideoHandler::setupViewLayout() {
    if(!_videoView)
        return;
    if(_videoView->layout())
        delete _videoView->layout();
    QGridLayout* layout = new QGridLayout(_videoView);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    _videoLoadingLabel->setParent(_videoView);
    m_subtitleLabel->setParent(_videoView);
    if(_isFullScreen) {
        m_topBar->setParent(_videoView);
        m_topBar->show();
        m_topBarLabel->setText(QFileInfo(_currentFile).fileName());
        m_controls->setParent(_videoView);
        layout->addWidget(m_topBar, 0, 0, 1, 2);
        layout->addWidget(_videoLoadingLabel, 1, 0, Qt::AlignCenter);
        layout->addWidget(m_subtitleLabel, 1, 0, Qt::AlignBottom);
        if(m_libraryListFrame)
            layout->addWidget(m_libraryListFrame, 1, 1);
        layout->addWidget(m_controls, 2, 0, 1, 2);
        layout->setRowStretch(1, 1);
        layout->setColumnStretch(0, 1);
    } else {
        m_topBar->hide();
        layout->addWidget(_videoLoadingLabel, 0, 0, Qt::AlignCenter);
        layout->addWidget(m_subtitleLabel, 0, 0, Qt::AlignBottom);
    }
}

void VideoHandler::updateSubtitle() {
    if(!m_subtitleLabel || !_player)
        return;
#if BUILD_QT5
    auto text = m_currentSubtitle.text();
    if(text.isEmpty()) {
        m_subtitleLabel->hide();
        return;
    }
    auto pos = _player->position();
    if(pos >= m_currentSubtitle.startTime() && pos < m_currentSubtitle.endTime()) {
        if(m_subtitleLabel->text() != text)
            m_subtitleLabel->setText(text);
        m_subtitleLabel->show();
    } else {
        m_subtitleLabel->hide();
    }
#else
    // Qt 6: the video sink's subtitleTextChanged signal drives the label
    // (see createLayout), so nothing to do here.
#endif
}

void VideoHandler::mouseMove( QMouseEvent* e ) {
    if (_isFullScreen) {
        // Keep the overlay hit rects in sync with the live geometry
        if(m_controls)
            m_controlsRect = m_controls->geometry();
        if(m_libraryListFrame)
            m_libraryRect = m_libraryListFrame->geometry();
        qApp->setOverrideCursor(Qt::CursorShape::ArrowCursor);
        showControls();
        showLibrary();
        m_overlayTimer.start(3000);
    }
}

bool VideoHandler::isFullScreen() {
    return _isFullScreen;
}
void VideoHandler::showNormal() {
    // The main window has already re-parented m_controls and m_libraryList back
    disconnect(_videoView, &XVideoGraphicsView::mouseMove, this, &VideoHandler::mouseMove);
    if(m_libraryListFrame) {
        delete m_libraryListFrame;
        m_libraryListFrame = 0;
    }
    m_controls->setProperty("cssClass", "windowedControls");
    m_controls->style()->unpolish(m_controls);
    m_controls->style()->polish(m_controls);
    m_libraryList->setProperty("cssClass", "windowedLibrary");
    m_libraryList->style()->unpolish(m_libraryList);
    m_libraryList->style()->polish(m_libraryList);
    m_libraryList->setMinimumSize(QSize(0, 0));
    m_libraryList->setMaximumSize(QSize(16777215, 16777215));

    _videoView->releaseKeyboard();
    _videoView->showNormal();                    // leave the full screen state first
    _videoView->setWindowFlags(Qt::Widget);      // then demote back to a child widget
    _videoView->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    // MUST be set before setupViewLayout() - otherwise it builds the full
    // screen layout and re-parents the controls back into the video view.
    _isFullScreen = false;
    setupViewLayout();
    _mediaGrid->addWidget(_videoView);

    auto flags = m_videoPreview->windowFlags();
    m_videoPreview->setParent(this->parentWidget(), flags);

    showControls();
    qApp->setOverrideCursor(Qt::CursorShape::ArrowCursor);
}

void VideoHandler::showFullscreen(QRect screenSize, bool libraryWindowed) {
    _isFullScreen = true;
    m_screenSize = screenSize;

    // The video view itself becomes the full screen window - no wrapper
    // widget, no re-parenting of the video output.
    // Determine the target screen by largest overlap with the main window's
    // frame geometry. A single-point screenAt() is unreliable here because
    // under HiDPI (dpr>1) mapToGlobal()/windowHandle()->screen() return
    // physical pixels and resolve to the wrong monitor.
    QWidget *topWindow = this->window();
    QRect winGeom = topWindow->frameGeometry();
    QScreen *target = nullptr;
    int bestArea = -1;
    for(QScreen *s : QGuiApplication::screens()) {
        QRect inter = winGeom.intersected(s->geometry());
        int area = inter.width() * inter.height();
        if(area > bestArea) {
            bestArea = area;
            target = s;
        }
    }
    if(!target)
        target = QGuiApplication::primaryScreen();

    // Clear the mouse-transparent attribute BEFORE promoting to a window:
    // setWindowFlags() recreates the QWindow, and a window created while the
    // attribute is set is click-through (events fall through to the window
    // behind).
    _videoView->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    _videoView->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Window);
    // As a top-level window the view no longer inherits the main window's
    // stylesheet, so apply it directly - otherwise the overlaid controls and
    // library list render with no styling (fully transparent).
    _videoView->setStyleSheet(this->window()->styleSheet());

    // Pin the window to the target screen.
    _videoView->setGeometry(target->geometry());

    m_controls->setProperty("cssClass", "fullScreenControls");
    m_controls->style()->unpolish(m_controls);
    m_controls->style()->polish(m_controls);

    placeLibraryList(libraryWindowed);

    // Overlay layout: top bar, video area (loading spinner, subtitles,
    // library list) and controls at the bottom - all on top of the video.
    setupViewLayout();

    // Apply the theme stylesheet directly to the overlaid widgets. The view is
    // a top-level window in fullscreen, so the main window's stylesheet does
    // not reliably cascade to re-parented children - set it on each widget.
    const QString themeSheet = this->window()->styleSheet();
    m_controls->setStyleSheet(themeSheet);
    if(m_libraryListFrame)
        m_libraryListFrame->setStyleSheet(themeSheet);
    m_libraryList->setStyleSheet(themeSheet);

    // Hover preview over the video
    auto flags = m_videoPreview->windowFlags();
    m_videoPreview->setParent(_videoView, flags);

    connect(_videoView, &XVideoGraphicsView::mouseMove, this, &VideoHandler::mouseMove);
    connect(_videoView, &XVideoGraphicsView::doubleClicked, this, [this](QMouseEvent* e) {emit doubleClicked(e);});
    connect(_videoView, &XVideoGraphicsView::singleClicked, this, [this](QMouseEvent* e) {emit singleClicked(e);});
    connect(_videoView, &XVideoGraphicsView::keyPressed, this, [this](QKeyEvent* e) {emit keyPressed(e);});
    connect(_videoView, &XVideoGraphicsView::keyReleased, this, [this](QKeyEvent* e) {emit keyReleased(e);});
    connect(_videoView, &XVideoGraphicsView::mouseEnter, this, [this](QEnterEvent* e) {emit mouseEnter(e);});

    // Place the window on the target screen, explicitly set its screen
    // association, then go true fullscreen (which covers the taskbar). Setting
    // the screen before showFullScreen() keeps the WM from dropping the window
    // onto the neighbouring monitor (the HiDPI fullscreen bug).
    _videoView->setGeometry(target->geometry());
    _videoView->show();
    if(_videoView->windowHandle())
        _videoView->windowHandle()->setScreen(target);
    _videoView->showFullScreen();
    _videoView->setFocus();
    _videoView->grabKeyboard();

    hideControls();
    hideLibrary();

    // Overlay hit rects from the actual geometry
    m_controlsRect = m_controls->geometry();
    if(m_libraryListFrame)
        m_libraryRect = m_libraryListFrame->geometry();
}

void VideoHandler::placeLibraryList(bool libraryWindowed) {
    m_libraryWindowed = libraryWindowed;
    if(_isFullScreen && !libraryWindowed && _videoView) {
        bool created = false;
        if(!m_libraryListFrame) {
            m_libraryListFrame = new QFrame(_videoView);
            m_libraryListFrame->setProperty("cssClass", "fullScreenLibrary");
            m_libraryListFrame->style()->unpolish(m_libraryListFrame);
            m_libraryListFrame->style()->polish(m_libraryListFrame);
            QGridLayout* frameLayout = new QGridLayout(m_libraryListFrame);
            frameLayout->setContentsMargins(0,0,0,0);
            frameLayout->setSpacing(0);
            m_libraryListFrame->setLayout(frameLayout);
            created = true;
        }
        auto libraryWidth = (SettingsHandler::getThumbSize() * 3) + ((SettingsHandler::getThumbSize() * 3) * 0.5);
        m_libraryListFrame->setFixedWidth(libraryWidth);
        m_libraryList->setProperty("cssClass", "fullScreenLibrary");
        m_libraryList->style()->unpolish(m_libraryList);
        m_libraryList->style()->polish(m_libraryList);
        if(created && _videoView->layout())
            ((QGridLayout*)_videoView->layout())->addWidget(m_libraryListFrame, 1, 1);
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
    if(m_topBarLabel)
        m_topBarLabel->setText(QFileInfo(file).fileName());
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

qreal VideoHandler::getSpeed()
{
    return _player->playbackRate();
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
    // LogHandler::Debug("[hideControls] Enter: _isFullScreen: "+ QString::number(_isFullScreen));
    if (m_controls && _isFullScreen)
    {
        // LogHandler::Debug("[hideControls] Hide controls");
        m_controls->hide();
        if(m_libraryListFrame && m_libraryListFrame->isHidden())
            qApp->setOverrideCursor(Qt::BlankCursor);
    }
}

void VideoHandler::showControls()
{
    if (m_controls)
    {
        bool isAvailable = true;
        if(XTPSettings::getFullScreenUIOnlyOnMouseOver() && _isFullScreen)
        {
            auto cursorPos = _videoView->mapFromGlobal(QCursor::pos());
            isAvailable = m_controlsRect.contains(cursorPos);
        }
        // LogHandler::Debug("[showControls] Enter: _isFullScreen: "+ QString::number(_isFullScreen));
        if (isAvailable)
        {
            // LogHandler::Debug("[showControls] Show controls");
            m_controls->show();
        }
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
    bool isAvailable = true;
    if(XTPSettings::getFullScreenUIOnlyOnMouseOver() && _isFullScreen)
    {
        auto cursorPos = _videoView->mapFromGlobal(QCursor::pos());
        isAvailable = m_libraryRect.contains(cursorPos);
    }
    if (_isFullScreen && isAvailable || m_libraryWindowed)
    {
        if(m_libraryListFrame) {
            m_libraryListFrame->show();
            if(_videoView)
                _videoView->releaseKeyboard();
        }
    }
}

void VideoHandler::hideControlsTimeout()
{
    if(_isFullScreen && _videoView) {
        auto cursorPos = _videoView->mapFromGlobal(QCursor::pos());
        if (!m_controlsRect.contains(cursorPos)) {
            hideControls();
        }
    }
}
void VideoHandler::hideLibraryTimeout()
{
    if(_isFullScreen && _videoView)
    {
        auto cursorPos = _videoView->mapFromGlobal(QCursor::pos());
        if (!m_libraryRect.contains(cursorPos)) {
            hideLibrary();
            if(_isFullScreen && _videoView)
                _videoView->grabKeyboard();
        }
    }
}
//void VideoHandler::audioBufferProbed(const QAudioBuffer &buffer) {
//    LogHandler::Debug("probe0");
//    //const quint16 *data = buffer->constData<quint16>();
//}
