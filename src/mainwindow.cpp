#include "mainwindow.h"
#include "ui_mainwindow.h"
MainWindow::MainWindow(QStringList arguments, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    QPixmap pixmap("://images/XTP_Splash.png");
    loadingSplash = new QSplashScreen(pixmap);
    loadingSplash->setStyleSheet("color: white");
    loadingSplash->show();

    ui->setupUi(this);

    loadingSplash->showMessage("v"+SettingsHandler::XTPVersion + "\nLoading Settings...", Qt::AlignBottom, Qt::white);
    SettingsHandler::Load();
    _xSettings = new SettingsDialog(this);
    tcodeHandler = new TCodeHandler();
    if(_xSettings->HasLaunchPass()) {
        int tries = 1;
        while(_isPasswordIncorrect != PasswordResponse::CANCEL && _isPasswordIncorrect == PasswordResponse::INCORRECT)
        {
            _isPasswordIncorrect = _xSettings->GetLaunchPass();
            if(_isPasswordIncorrect == PasswordResponse::CANCEL)
            {
                QTimer::singleShot(0, this, SLOT(onEventLoopStarted()));
                return;
            }
            else if(_isPasswordIncorrect == PasswordResponse::INCORRECT)
            {
                switch(tries) {
                    case 1:
                        LogHandler::Dialog("Wrong!", XLogLevel::Critical);
                    break;
                    case 2:
                        LogHandler::Dialog("Nope!", XLogLevel::Critical);
                    break;
                    case 3:
                        LogHandler::Dialog("K thx byyye!", XLogLevel::Critical);
                    break;
                }

                if( tries >= 3)
                {
                    QTimer::singleShot(0, this, SLOT(onEventLoopStarted()));
                    return;
                }
            }
            tries++;
        }
    }

    if(arguments.length() > 0)
    {
        foreach(QString arg, arguments)
        {
            if(arg.toLower().startsWith("-debug"))
                LogHandler::UserDebug(true);
            else if(arg.toLower().startsWith("-reset"))
                SettingsHandler::Default();
        }
    }

    loadingSplash->showMessage("v"+SettingsHandler::XTPVersion + "\nLoading UI...", Qt::AlignBottom, Qt::white);

    qRegisterMetaType<LibraryListItem>();
    qRegisterMetaTypeStreamOperators<LibraryListItem>();

    funscriptHandler = new FunscriptHandler(_axisNames.Stroke);

    textToSpeech = new QTextToSpeech(this);
    auto availableVoices = textToSpeech->availableVoices();

    const QVoice voice = boolinq::from(availableVoices).firstOrDefault([](const QVoice &x) { return x.gender() == QVoice::Female; });
    textToSpeech->setVoice(voice);

    deoConnectionStatusLabel = new QLabel(this);
    deoRetryConnectionButton = new QPushButton(this);
    deoRetryConnectionButton->hide();
    deoRetryConnectionButton->setText("DeoVR Retry");
    ui->statusbar->addPermanentWidget(deoConnectionStatusLabel);
    ui->statusbar->addPermanentWidget(deoRetryConnectionButton);

    whirligigConnectionStatusLabel = new QLabel(this);
    whirligigRetryConnectionButton = new QPushButton(this);
    whirligigRetryConnectionButton->hide();
    whirligigRetryConnectionButton->setText("Whirligig Retry");
    ui->statusbar->addPermanentWidget(whirligigConnectionStatusLabel);
    ui->statusbar->addPermanentWidget(whirligigRetryConnectionButton);

    connectionStatusLabel = new QLabel(this);
    retryConnectionButton = new QPushButton(this);
    retryConnectionButton->hide();
    retryConnectionButton->setText("TCode Retry");
    ui->statusbar->addPermanentWidget(connectionStatusLabel);
    ui->statusbar->addPermanentWidget(retryConnectionButton);

    gamepadConnectionStatusLabel = new QLabel(this);
    ui->statusbar->addPermanentWidget(gamepadConnectionStatusLabel);

    _mediaFrame = new QFrame(this);
    _mediaGrid = new QGridLayout(_mediaFrame);
    _mediaFrame->setLayout(_mediaGrid);

    videoHandler = new VideoHandler(this);
    _mediaGrid->addWidget(videoHandler, 0, 0, 3, 5);

    _controlsHomePlaceHolderFrame = new QFrame(this);
    _controlsHomePlaceHolderGrid = new QGridLayout(_controlsHomePlaceHolderFrame);
    _controlsHomePlaceHolderFrame->setLayout(_controlsHomePlaceHolderGrid);

    _playerControlsFrame = new PlayerControls(this);
    _controlsHomePlaceHolderGrid->addWidget(_playerControlsFrame, 0, 0);

    ui->mediaAndControlsGrid->addWidget(_mediaFrame, 0, 0, 19, 3);
    ui->mediaAndControlsGrid->addWidget(_controlsHomePlaceHolderFrame, 20, 0, 1, 3);

    audioSyncFilter = new AudioSyncFilter(this);
    videoHandler->installFilter(audioSyncFilter);

    _videoLoadingMovie = new QMovie("://images/Eclipse-1s-loading-200px.gif");
    _videoLoadingLabel = new QLabel(this);
    _videoLoadingLabel->setMovie(_videoLoadingMovie);
    _videoLoadingLabel->setAttribute(Qt::WA_TransparentForMouseEvents );
    _videoLoadingLabel->setMaximumSize(200,200);
    _videoLoadingLabel->setStyleSheet("* {background: transparent}");
    _videoLoadingLabel->setAlignment(Qt::AlignCenter);
    _mediaGrid->addWidget(_videoLoadingLabel, 1, 2);
    on_setLoading(false);

    _playerControlsFrame->setVolume(SettingsHandler::getPlayerVolume());

    libraryList = new QListWidget(this);
    libraryList->setUniformItemSizes(true);
    libraryList->setContextMenuPolicy(Qt::CustomContextMenu);
    libraryList->setProperty("id", "libraryList");
    libraryList->setMovement(QListView::Static);
    libraryList->setTextElideMode(Qt::TextElideMode::ElideRight);
    libraryList->setWordWrap(true);

    QScroller::grabGesture(libraryList->viewport(), QScroller::LeftMouseButtonGesture);
    auto scroller = QScroller::scroller(libraryList->viewport());
    QScrollerProperties scrollerProperties;
    QVariant overshootPolicy = QVariant::fromValue<QScrollerProperties::OvershootPolicy>(QScrollerProperties::OvershootAlwaysOff);
    scrollerProperties.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, overshootPolicy);
    QScroller::scroller(libraryList)->setScrollerProperties(scrollerProperties);
    scroller->setScrollerProperties(scrollerProperties);
    ui->libraryGrid->addWidget(libraryList, 1, 0, 20, 12);

    ui->libraryGrid->setSpacing(5);
    ui->libraryGrid->setColumnMinimumWidth(0, 0);

    backLibraryButton = new QPushButton(this);
    backLibraryButton->setProperty("id", "backLibraryButton");
    QIcon backIcon("://images/icons/back.svg");
    backLibraryButton->setIcon(backIcon);
    ui->libraryGrid->addWidget(backLibraryButton, 0, 0);
    ui->libraryFrame->setFrameShadow(QFrame::Sunken);
    backLibraryButton->hide();

    windowedLibraryButton = new QPushButton(this);
    windowedLibraryButton->setProperty("id", "windowedLibraryButton");
    QIcon windowedIcon("://images/icons/windowed.svg");
    windowedLibraryButton->setIcon(windowedIcon);
    ui->libraryGrid->addWidget(windowedLibraryButton, 0, ui->libraryGrid->columnCount() - 1);

    libraryWindow = new LibraryWindow(this);
    libraryWindow->setProperty("id", "libraryWindow");

    randomizeLibraryButton = new QPushButton(this);
    randomizeLibraryButton->setProperty("id", "randomizeLibraryButton");
    QIcon reloadIcon("://images/icons/reload.svg");
    randomizeLibraryButton->setIcon(reloadIcon);
    ui->libraryGrid->addWidget(randomizeLibraryButton, 0, 1);

    editPlaylistButton = new QPushButton(this);
    editPlaylistButton->setProperty("id", "editPlaylistButton");
    QIcon editIcon("://images/icons/edit.svg");
    editPlaylistButton->setIcon(editIcon);
    ui->libraryGrid->addWidget(editPlaylistButton, 0, ui->libraryGrid->columnCount() - 2);
    editPlaylistButton->hide();

    savePlaylistButton = new QPushButton(this);
    savePlaylistButton->setProperty("id", "savePlaylistButton");
    QIcon saveIcon("://images/icons/save.svg");
    savePlaylistButton->setIcon(saveIcon);
    ui->libraryGrid->addWidget(savePlaylistButton, 0, ui->libraryGrid->columnCount() - 3);
    savePlaylistButton->hide();

    cancelEditPlaylistButton = new QPushButton(this);
    cancelEditPlaylistButton->setProperty("id", "cancelEditPlaylistButton");
    QIcon xIcon("://images/icons/x.svg");
    cancelEditPlaylistButton->setIcon(xIcon);
    ui->libraryGrid->addWidget(cancelEditPlaylistButton, 0, ui->libraryGrid->columnCount() - 2);
    cancelEditPlaylistButton->hide();

    ui->libraryFrame->setFrameShadow(QFrame::Sunken);

    libraryLoadingMovie = new QMovie("://images/Eclipse-1s-loading-200px.gif");
    libraryLoadingMovie->setScaledSize({200,200});
    libraryLoadingLabel = new QLabel(this);
    libraryLoadingLabel->setMovie(libraryLoadingMovie);
    libraryLoadingLabel->setStyleSheet("* {background-color: rgba(128,128,128, 0.5)}");
    libraryLoadingLabel->setAlignment(Qt::AlignCenter);
    ui->libraryGrid->addWidget(libraryLoadingLabel, 0, 0, 21, 12);
    libraryLoadingLabel->hide();

    thumbCaptureTime = 35000;
    libraryViewGroup = new QActionGroup(this);
    libraryViewGroup->addAction(ui->actionList);
    libraryViewGroup->addAction(ui->actionThumbnail);

    videoPreviewWidget = new VideoPreviewWidget(this);
    videoPreviewWidget->resize(150, 90);
    videoPreviewWidget->hide();

    QMenu* submenuSize = ui->menuView->addMenu( "Size" );
    submenuSize->setObjectName("sizeMenu");
    libraryThumbSizeGroup = new QActionGroup(submenuSize);
    action75_Size = submenuSize->addAction( "75" );
    action75_Size->setCheckable(true);
    action100_Size = submenuSize->addAction( "100" );
    action100_Size->setCheckable(true);
    action125_Size = submenuSize->addAction( "125" );
    action125_Size->setCheckable(true);
    action150_Size = submenuSize->addAction( "150" );
    action150_Size->setCheckable(true);
    action175_Size = submenuSize->addAction( "175" );
    action175_Size->setCheckable(true);
    action200_Size = submenuSize->addAction( "200" );
    action200_Size->setCheckable(true);
    actionCustom_Size = submenuSize->addAction( "Custom" );
    actionCustom_Size->setCheckable(true);
    libraryThumbSizeGroup->addAction(action75_Size);
    libraryThumbSizeGroup->addAction(action100_Size);
    libraryThumbSizeGroup->addAction(action125_Size);
    libraryThumbSizeGroup->addAction(action150_Size);
    libraryThumbSizeGroup->addAction(action175_Size);
    libraryThumbSizeGroup->addAction(action200_Size);
    libraryThumbSizeGroup->addAction(actionCustom_Size);

    QMenu* submenuSort = ui->menuView->addMenu( "Sort" );
    submenuSort->setObjectName("sortMenu");
    librarySortGroup = new QActionGroup(submenuSort);
    actionNameAsc_Sort = submenuSort->addAction( "Name (Asc)" );
    actionNameAsc_Sort->setCheckable(true);
    actionNameDesc_Sort = submenuSort->addAction( "Name (Desc)" );
    actionNameDesc_Sort->setCheckable(true);
    actionRandom_Sort = submenuSort->addAction( "Random" );
    actionRandom_Sort->setCheckable(true);
    actionCreatedAsc_Sort = submenuSort->addAction( "Created (Asc)" );
    actionCreatedAsc_Sort->setCheckable(true);
    actionCreatedDesc_Sort = submenuSort->addAction( "Created (Desc)" );
    actionCreatedDesc_Sort->setCheckable(true);
    actionTypeAsc_Sort = submenuSort->addAction( "Type (Asc)" );
    actionTypeAsc_Sort->setCheckable(true);
    actionTypeDesc_Sort = submenuSort->addAction( "Type (Desc)" );
    actionTypeDesc_Sort->setCheckable(true);
    librarySortGroup->addAction(actionNameAsc_Sort);
    librarySortGroup->addAction(actionNameDesc_Sort);
    librarySortGroup->addAction(actionRandom_Sort);
    librarySortGroup->addAction(actionCreatedAsc_Sort);
    librarySortGroup->addAction(actionCreatedDesc_Sort);
    librarySortGroup->addAction(actionTypeAsc_Sort);
    librarySortGroup->addAction(actionTypeDesc_Sort);


    if (SettingsHandler::getLibraryView() == LibraryView::List)
    {
        ui->actionList->setChecked(true);
        on_actionList_triggered();
    }
    else
    {
        ui->actionThumbnail->setChecked(true);
        on_actionThumbnail_triggered();
    }


    auto splitterSizes = SettingsHandler::getMainWindowSplitterPos();
    if (splitterSizes.count() > 0)
        ui->mainFrameSplitter->setSizes(splitterSizes);

    connect(ui->mainFrameSplitter, &QSplitter::splitterMoved, this, &MainWindow::on_mainwindow_splitterMove);
    connect(backLibraryButton, &QPushButton::clicked, this, &MainWindow::backToMainLibrary);
    connect(randomizeLibraryButton, &QPushButton::clicked, this, &MainWindow::on_actionRandom_triggered);
    connect(windowedLibraryButton, &QPushButton::clicked, this, &MainWindow::onLibraryWindowed_Clicked);
    connect(savePlaylistButton, &QPushButton::clicked, this, &MainWindow::savePlaylist);
    connect(editPlaylistButton, &QPushButton::clicked, this, &MainWindow::editPlaylist);
    connect(cancelEditPlaylistButton, &QPushButton::clicked, this, &MainWindow::cancelEditPlaylist);

    connect(libraryWindow, &LibraryWindow::close, this, &MainWindow::onLibraryWindowed_Closed);

    connect(_xSettings, &SettingsDialog::deoDeviceConnectionChange, this, &MainWindow::on_deo_device_connectionChanged);
    connect(_xSettings, &SettingsDialog::deoDeviceError, this, &MainWindow::on_deo_device_error);

    connect(_xSettings, &SettingsDialog::whirligigDeviceConnectionChange, this, &MainWindow::on_whirligig_device_connectionChanged);
    connect(_xSettings, &SettingsDialog::whirligigDeviceError, this, &MainWindow::on_whirligig_device_error);
    connect(_xSettings, &SettingsDialog::deviceConnectionChange, this, &MainWindow::on_device_connectionChanged);
    connect(_xSettings, &SettingsDialog::deviceError, this, &MainWindow::on_device_error);
    connect(_xSettings, &SettingsDialog::TCodeHomeClicked, this, &MainWindow::deviceHome);
    connect(_xSettings->getDeoHandler(), &DeoHandler::messageRecieved, this, &MainWindow::onVRMessageRecieved);
    connect(_xSettings->getWhirligigHandler(), &WhirligigHandler::messageRecieved, this, &MainWindow::onVRMessageRecieved);
    connect(_xSettings, &SettingsDialog::gamepadConnectionChange, this, &MainWindow::on_gamepad_connectionChanged);
    connect(_xSettings->getGamepadHandler(), &GamepadHandler::emitTCode, this, &MainWindow::on_gamepad_sendTCode);
    connect(_xSettings->getGamepadHandler(), &GamepadHandler::emitAction, this, &MainWindow::on_gamepad_sendAction);

    _xSettings->init(videoHandler);

    connect(action75_Size, &QAction::triggered, this, &MainWindow::on_action75_triggered);
    connect(action100_Size, &QAction::triggered, this, &MainWindow::on_action100_triggered);
    connect(action125_Size, &QAction::triggered, this, &MainWindow::on_action125_triggered);
    connect(action150_Size, &QAction::triggered, this, &MainWindow::on_action150_triggered);
    connect(action175_Size, &QAction::triggered, this, &MainWindow::on_action175_triggered);
    connect(action200_Size, &QAction::triggered, this, &MainWindow::on_action200_triggered);
    connect(actionCustom_Size, &QAction::triggered, this, &MainWindow::on_actionCustom_triggered);

    connect(actionNameAsc_Sort, &QAction::triggered, this, &MainWindow::on_actionNameAsc_triggered);
    connect(actionNameDesc_Sort, &QAction::triggered, this, &MainWindow::on_actionNameDesc_triggered);
    connect(actionRandom_Sort, &QAction::triggered, this, &MainWindow::on_actionRandom_triggered);
    connect(actionCreatedAsc_Sort, &QAction::triggered, this, &MainWindow::on_actionCreatedAsc_triggered);
    connect(actionCreatedDesc_Sort, &QAction::triggered, this, &MainWindow::on_actionCreatedDesc_triggered);
    connect(actionTypeAsc_Sort, &QAction::triggered, this, &MainWindow::on_actionTypeAsc_triggered);
    connect(actionTypeDesc_Sort, &QAction::triggered, this, &MainWindow::on_actionTypeDesc_triggered);

    connect(videoHandler, &VideoHandler::positionChanged, this, &MainWindow::on_media_positionChanged, Qt::QueuedConnection);
    connect(videoHandler, &VideoHandler::mediaStatusChanged, this, &MainWindow::on_media_statusChanged, Qt::QueuedConnection);
    connect(videoHandler, &VideoHandler::started, this, &MainWindow::on_media_start, Qt::QueuedConnection);
    connect(videoHandler, &VideoHandler::stopped, this, &MainWindow::on_media_stop, Qt::QueuedConnection);
    connect(videoHandler, &VideoHandler::togglePaused, this, &MainWindow::onVideoHandler_togglePaused);

    connect(_playerControlsFrame, &PlayerControls::seekSliderMoved, this, &MainWindow::on_seekSlider_sliderMoved);
    connect(_playerControlsFrame, &PlayerControls::seekSliderHover, this, &MainWindow::on_seekslider_hover );
    connect(_playerControlsFrame, &PlayerControls::seekSliderLeave, this, &MainWindow::on_seekslider_leave );
    connect(_playerControlsFrame, &PlayerControls::volumeChanged, this, &MainWindow::on_VolumeSlider_valueChanged);
    connect(_playerControlsFrame, &PlayerControls::loopButtonToggled, this, &MainWindow::on_loopToggleButton_toggled);
    connect(_playerControlsFrame, &PlayerControls::muteChanged, this, &MainWindow::on_MuteBtn_toggled);
    connect(_playerControlsFrame, &PlayerControls::fullscreenToggled, this, &MainWindow::on_fullScreenBtn_clicked);
    connect(_playerControlsFrame, &PlayerControls::settingsClicked, this, &MainWindow::on_settingsButton_clicked);
    connect(_playerControlsFrame, &PlayerControls::playClicked, this, &MainWindow::on_PlayBtn_clicked);
    connect(_playerControlsFrame, &PlayerControls::skipForward, this, &MainWindow::on_skipForwardButton_clicked);
    connect(_playerControlsFrame, &PlayerControls::skipToMoneyShot, this, &MainWindow::skipToMoneyShot);

    connect(_playerControlsFrame, &PlayerControls::skipBack, this, &MainWindow::on_skipBackButton_clicked);
    //connect(player, static_cast<void(AVPlayer::*)(AVPlayer::Error )>(&AVPlayer::error), this, &MainWindow::on_media_error);

    connect(videoHandler, &VideoHandler::doubleClicked, this, &MainWindow::media_double_click_event);
    connect(videoHandler, &VideoHandler::rightClicked, this, &MainWindow::media_single_click_event);
    connect(this, &MainWindow::keyPressed, this, &MainWindow::on_key_press);
    connect(this, &MainWindow::change, this, &MainWindow::on_mainwindow_change);
    connect(this, &MainWindow::playVideo, this, &MainWindow::on_playVideo);
    //    connect(this, &MainWindow::setLoading, this, &MainWindow::on_setLoading);
    //    connect(this, &MainWindow::scriptNotFound, this, &MainWindow::on_scriptNotFound);
    //connect(videoHandler, &VideoHandler::mouseEnter, this, &MainWindow::on_video_mouse_enter);

    connect(libraryList, &QListWidget::customContextMenuRequested, this, &MainWindow::onLibraryList_ContextMenuRequested);
    connect(libraryList, &QListWidget::itemDoubleClicked, this, &MainWindow::on_LibraryList_itemDoubleClicked);
    connect(libraryList, &QListWidget::itemClicked, this, &MainWindow::on_LibraryList_itemClicked);

    connect(retryConnectionButton, &QPushButton::clicked, _xSettings, &SettingsDialog::initDeviceRetry);
    connect(deoRetryConnectionButton, &QPushButton::clicked, _xSettings, &SettingsDialog::initDeoRetry);
    connect(QApplication::instance(), &QCoreApplication::aboutToQuit, this, &MainWindow::dispose);

    loadingSplash->showMessage("v"+SettingsHandler::XTPVersion + "\nSetting user styles...", Qt::AlignBottom, Qt::white);
    QFile file(SettingsHandler::getSelectedTheme());
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    setStyleSheet(styleSheet);

    setFocus();
    _defaultAppSize = this->size();
    _appSize = _defaultAppSize;
    _appPos = this->pos();

    loadingSplash->showMessage("v"+SettingsHandler::XTPVersion + "\nLoading Library...", Qt::AlignBottom, Qt::white);
    on_load_library(SettingsHandler::getSelectedLibrary());

//    QScreen *screen = this->screen();
//    QSize screenSize = screen->size();
//    auto minHeight = round(screenSize.height() * .06f);
//    _playerControlsFrame->setMinimumHeight(minHeight);
//    _controlsHomePlaceHolderFrame->setMinimumHeight(minHeight);
//    _playerControlsFrame->setMaximumHeight(minHeight);
//    _controlsHomePlaceHolderFrame->setMaximumHeight(minHeight);

    loadingSplash->showMessage("v"+SettingsHandler::XTPVersion + "\nStarting Application...", Qt::AlignBottom, Qt::white);
    loadingSplash->finish(this);

}
MainWindow::~MainWindow()
{

}

void MainWindow::onEventLoopStarted()
{
    if(_isPasswordIncorrect)
        QApplication::quit();
}
void MainWindow::dispose()
{
    deviceSwitchedHome();
    if(playingLibraryListItem != nullptr)
        updateMetaData(playingLibraryListItem->getLibraryListItem());

    SettingsHandler::Save();
    _xSettings->dispose();

    if (videoHandler->isPlaying())
    {
        videoHandler->stop();
    }
    if(funscriptFuture.isRunning())
    {
        funscriptFuture.cancel();
        funscriptFuture.waitForFinished();
    }
    if(funscriptVRSyncFuture.isRunning())
    {
        funscriptVRSyncFuture.cancel();
        funscriptVRSyncFuture.waitForFinished();
    }
    if(loadingLibraryFuture.isRunning())
    {
        loadingLibraryFuture.cancel();
        loadingLibraryFuture.waitForFinished();
    }
    qDeleteAll(funscriptHandlers);
    delete tcodeHandler;
    delete videoHandler;
    delete connectionStatusLabel;
    delete retryConnectionButton;
    delete videoPreviewWidget;
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // This function repeatedly call for those QObjects
    // which have installed eventFilter (Step 2)
    if (obj == (QObject*)playerControlsPlaceHolder) {
        if (event->type() == QEvent::Enter)
        {
            showControls();
        }
        else if(event->type() == QEvent::Leave)
        {
            hideControls();
        }
        return true;
    }
    else if (obj == (QObject*)playerLibraryPlaceHolder) {
        if (event->type() == QEvent::Enter)
        {
            showLibrary();
        }
        else if(event->type() == QEvent::Leave)
        {
            hideLibrary();
        }
        return true;
    }
    else
    {
        // pass the event on to the parent class
        return QWidget::eventFilter(obj, event);
    }
}
void MainWindow::on_key_press(QKeyEvent * event)
{
    MediaActions actions;
     switch(event->key())
    {
        case Qt::Key_Space:
        case Qt::Key_Enter:
        case Qt::Key_MediaPause:
        case Qt::Key_MediaTogglePlayPause:
            mediaAction(actions.TogglePause);
            break;
        case Qt::Key_1:
        case Qt::Key_F11:
            mediaAction(actions.FullScreen);
            break;
        case Qt::Key_M:
        case Qt::Key_2:
            mediaAction(actions.Mute);
            break;
        case Qt::Key_MediaStop:
        case Qt::Key_Escape:
            mediaAction(actions.Stop);
            break;
        case Qt::Key_MediaNext:
        case Qt::Key_E:
            mediaAction(actions.Next);
            break;
        case Qt::Key_MediaPrevious:
        case Qt::Key_Q:
            mediaAction(actions.Back);
            break;
        case Qt::Key_VolumeUp:
        case Qt::Key_W:
            mediaAction(actions.VolumeUp);
            break;
        case Qt::Key_VolumeDown:
        case Qt::Key_S:
            mediaAction(actions.VolumeDown);
            break;
        case Qt::Key_L:
        case Qt::Key_C:
            mediaAction(actions.Loop);
            break;
        case Qt::Key_A:
            mediaAction(actions.Rewind);
            break;
        case Qt::Key_D:
            mediaAction(actions.FastForward);
            break;
        case Qt::Key_X:
            mediaAction(actions.IncreaseXRange);
            break;
        case Qt::Key_Z:
            mediaAction(actions.DecreaseXRange);
            break;
        case Qt::Key_F:
            mediaAction(actions.DecreaseXUpperRange);
            break;
        case Qt::Key_R:
            mediaAction(actions.IncreaseXUpperRange);
            break;
        case Qt::Key_G:
            mediaAction(actions.DecreaseXLowerRange);
            break;
        case Qt::Key_T:
            mediaAction(actions.IncreaseXLowerRange);
            break;
        case Qt::Key_Control:
            mediaAction(actions.ResetLiveXRange);
            break;
        case Qt::Key_I:
            mediaAction(actions.ToggleFunscriptInvert);
            break;
        case Qt::Key_V:
            mediaAction(actions.ToggleAxisMultiplier);
            break;
         case Qt::Key_P:
             mediaAction(actions.TogglePauseAllDeviceActions);
             break;
         case Qt::Key_J:
             mediaAction(actions.SkipToMoneyShot);
             break;
    }
}
void MainWindow::mediaAction(QString action)
{
    MediaActions actions;
     if (action == actions.TogglePause)
    {
        if (videoHandler->isPaused() || videoHandler->isPlaying())
        {
            videoHandler->togglePause();
        }
        else if(SettingsHandler::getDeoEnabled() && _xSettings->getDeoHandler()->isConnected())
        {
            //_xSettings->getDeoHandler()->togglePause();
        }
        else
        {
            on_PlayBtn_clicked();
        }
    }
    else if(action == actions.FullScreen)
    {
        toggleFullScreen();
    }
    else if(action == actions.Mute)
    {
        on_MuteBtn_toggled(!videoHandler->isMute());
    }
    else if(action == actions.Stop)
    {
        stopMedia();
    }
     else if(action == actions.Next)
    {
        skipForward();
    }
    else if(action == actions.Back)
    {
        skipBack();
    }
    else if(action == actions.VolumeUp)
    {
        _playerControlsFrame->IncreaseVolume();
    }
    else if(action == actions.VolumeDown)
    {
        _playerControlsFrame->DecreaseVolume();
    }
    else if(action == actions.Loop)
    {
        _playerControlsFrame->toggleLoop(videoHandler->duration(), videoHandler->position());
    }
    else if(action == actions.Rewind)
    {
        if (videoHandler->isPlaying())
        {
            rewind();
        }
    }
    else if(action == actions.FastForward)
    {
        if (videoHandler->isPlaying())
        {
            fastForward();
        }
    }
    else if(action == actions.TCodeSpeedUp)
    {
        int newGamepadSpeed = SettingsHandler::getLiveGamepadSpeed() + SettingsHandler::getGamepadSpeedIncrement();
        SettingsHandler::setLiveGamepadSpeed(newGamepadSpeed);
        if(!SettingsHandler::getDisableSpeechToText())
            textToSpeech->say("Raise speed "+ QString::number(newGamepadSpeed));
    }
    else if(action == actions.TCodeSpeedDown)
    {
        int newGamepadSpeed = SettingsHandler::getLiveGamepadSpeed() - SettingsHandler::getGamepadSpeedIncrement();
        if (newGamepadSpeed > 0)
        {
            SettingsHandler::setLiveGamepadSpeed(newGamepadSpeed);
            if(!SettingsHandler::getDisableSpeechToText())
                textToSpeech->say("Lower speed "+ QString::number(newGamepadSpeed));
        } else if(!SettingsHandler::getDisableSpeechToText())
                textToSpeech->say("Lower speed at minimum");
    }
    else if(action == actions.IncreaseXLowerRange)
    {
        TCodeChannels axisNames;
        int xRangeStep = SettingsHandler::getXRangeStep();
        int newLiveRange = SettingsHandler::getLiveXRangeMin() + SettingsHandler::getXRangeStep();
        int xRangeMax = SettingsHandler::getLiveXRangeMax();
        if(newLiveRange < xRangeMax - xRangeStep)
        {
            SettingsHandler::setLiveXRangeMin(newLiveRange);
            if(!SettingsHandler::getDisableSpeechToText())
                textToSpeech->say("Raise X min to "+ QString::number(newLiveRange));
        }
        else
        {
            SettingsHandler::setLiveXRangeMin(xRangeMax - xRangeStep);
            if(!SettingsHandler::getDisableSpeechToText())
                textToSpeech->say("X Min limit reached");
        }
    }
    else if(action == actions.DecreaseXLowerRange)
    {
        TCodeChannels axisNames;
        int newLiveRange = SettingsHandler::getLiveXRangeMin() - SettingsHandler::getXRangeStep();
        int axisMin = SettingsHandler::getAxis(axisNames.Stroke).Min;
        if(newLiveRange > axisMin)
        {
            SettingsHandler::setLiveXRangeMin(newLiveRange);
            if(!SettingsHandler::getDisableSpeechToText())
                textToSpeech->say("Lower X min to "+ QString::number(newLiveRange));
        }
        else
        {
            SettingsHandler::setLiveXRangeMin(axisMin);
            if(!SettingsHandler::getDisableSpeechToText())
                textToSpeech->say("Low X min limit reached");
        }
    }
    else if(action == actions.IncreaseXUpperRange)
    {
        TCodeChannels axisNames;
        int newLiveRange = SettingsHandler::getLiveXRangeMax() + SettingsHandler::getXRangeStep();
        int axisMax = SettingsHandler::getAxis(axisNames.Stroke).Max;
        if(newLiveRange < axisMax)
        {
            SettingsHandler::setLiveXRangeMax(newLiveRange);
            if(!SettingsHandler::getDisableSpeechToText())
                textToSpeech->say("Raise X max to "+ QString::number(newLiveRange));
        }
        else
        {
            SettingsHandler::setLiveXRangeMax(axisMax);
            if(!SettingsHandler::getDisableSpeechToText())
                textToSpeech->say("High X max limit reached");
        }
    }
    else if(action == actions.DecreaseXUpperRange)
    {
        TCodeChannels axisNames;
        int xRangeStep = SettingsHandler::getXRangeStep();
        int newLiveRange = SettingsHandler::getLiveXRangeMax() - xRangeStep;
        int xRangeMin = SettingsHandler::getLiveXRangeMin();
        if(newLiveRange > xRangeMin + xRangeStep)
        {
            SettingsHandler::setLiveXRangeMax(newLiveRange);
            if(!SettingsHandler::getDisableSpeechToText())
                textToSpeech->say("Lower X max to "+ QString::number(newLiveRange));
        }
        else
        {
            SettingsHandler::setLiveXRangeMax(xRangeMin + xRangeStep);
            if(!SettingsHandler::getDisableSpeechToText())
                textToSpeech->say("Low X max limit reached");
        }
    }
    else if (action == actions.IncreaseXRange)
    {
        TCodeChannels axisNames;
        int xRangeMax = SettingsHandler::getLiveXRangeMax();
        int xRangeMin = SettingsHandler::getLiveXRangeMin();
        int xRangeStep = SettingsHandler::getXRangeStep();
        int newLiveMaxRange = xRangeMax + xRangeStep;
        int limitXMax = 0;
        int axisMax = SettingsHandler::getAxis(axisNames.Stroke).Max;
        if(newLiveMaxRange < axisMax)
        {
            SettingsHandler::setLiveXRangeMax(newLiveMaxRange);
        }
        else
        {
            limitXMax = axisMax;
            SettingsHandler::setLiveXRangeMax(axisMax);
        }

        int newLiveMinRange = xRangeMin - xRangeStep;
        int axisMin = SettingsHandler::getAxis(axisNames.Stroke).Min;
        int limitXMin = 0;
        if(newLiveMinRange > axisMin)
        {
            SettingsHandler::setLiveXRangeMin(newLiveMinRange);
        }
        else
        {
            limitXMin = axisMin;
            SettingsHandler::setLiveXRangeMin(axisMin);
        }

        if(!SettingsHandler::getDisableSpeechToText())
        {
            if (limitXMax && limitXMin)
                textToSpeech->say("Increase X at limit");
            else if (limitXMax)
                textToSpeech->say("Increase X, "+ QString::number(newLiveMinRange) + ", max at limit");
            else if (limitXMin)
                textToSpeech->say("Increase X, "+ QString::number(newLiveMaxRange) + ", min at limit");
            else
                textToSpeech->say("Increase X, "+ QString::number(newLiveMaxRange) + ", "+ QString::number(newLiveMinRange));
        }

    }
    else if (action == actions.DecreaseXRange)
    {
        TCodeChannels axisNames;
        int xRangeMax = SettingsHandler::getLiveXRangeMax();
        int xRangeMin = SettingsHandler::getLiveXRangeMin();
        int xRangeStep = SettingsHandler::getXRangeStep();
        int newLiveMaxRange = xRangeMax - xRangeStep;
        int limitXMax = 0;
        if(newLiveMaxRange > xRangeMin + xRangeStep)
        {
            SettingsHandler::setLiveXRangeMax(newLiveMaxRange);
        }
        else
        {
            limitXMax = xRangeMin + xRangeStep;
            SettingsHandler::setLiveXRangeMax(xRangeMin + xRangeStep);
        }

        int newLiveMinRange = xRangeMin + xRangeStep;
        int limitXMin = 0;
        if(newLiveMinRange < xRangeMax - xRangeStep)
        {
            SettingsHandler::setLiveXRangeMin(newLiveMinRange);
        }
        else
        {
            limitXMin = xRangeMax - xRangeStep;
            SettingsHandler::setLiveXRangeMin(xRangeMax - xRangeStep);
        }
        if(!SettingsHandler::getDisableSpeechToText())
        {
            if (limitXMax && limitXMin)
                textToSpeech->say("Decrease X at limit");
            else if (limitXMax)
                textToSpeech->say("Decrease X, "+ QString::number(newLiveMinRange) + ", max at limit");
            else if (limitXMin)
                textToSpeech->say("Decrease X, "+ QString::number(newLiveMaxRange) + ", min at limit");
            else
                textToSpeech->say("Decrease X, "+ QString::number(newLiveMaxRange) + ", "+ QString::number(newLiveMinRange));
        }
    }
    else if (action == actions.ResetLiveXRange)
    {
        if(!SettingsHandler::getDisableSpeechToText())
            textToSpeech->say("Resetting X range");
        SettingsHandler::resetLiveXRange();
    }
    else if (action == actions.ToggleAxisMultiplier)
    {
        bool multiplier = SettingsHandler::getMultiplierEnabled();
        textToSpeech->say(multiplier ? "Disable multiplier" : "Enable multiplier");
        SettingsHandler::setLiveMultiplierEnabled(!multiplier);
    }
    else if (action == actions.ToggleFunscriptInvert)
    {
        bool inverted = FunscriptHandler::getInverted();
        textToSpeech->say(inverted ? "Funscript normal" : "Funscript inverted");
        FunscriptHandler::setInverted(!inverted);
    }
     else if(action == actions.TogglePauseAllDeviceActions)
     {
         bool paused = SettingsHandler::getLiveActionPaused();
         textToSpeech->say(paused ? "Resume action" : "Pause action");
         SettingsHandler::setLiveActionPaused(!paused);
     }
     else if (action == actions.SkipToMoneyShot)
     {
         skipToMoneyShot();
     }
}

void MainWindow::deviceHome()
{
    auto deviceHandler = _xSettings->getSelectedDeviceHandler();
    if(deviceHandler != nullptr && _xSettings->isConnected())
        deviceHandler->sendTCode(tcodeHandler->getAllHome());
}
void MainWindow::deviceSwitchedHome()
{
    auto deviceHandler = _xSettings->getSelectedDeviceHandler();
    if(deviceHandler != nullptr && _xSettings->isConnected())
        deviceHandler->sendTCode(tcodeHandler->getSwitchedHome());
}

void MainWindow::on_mainwindow_splitterMove(int pos, int index)
{
    SettingsHandler::setMainWindowSplitterPos(ui->mainFrameSplitter->sizes());
}

//qint64 strokerUpdateMillis = 50;
qint64 strokerLastUpdate;
int lastAction = 500;
int minAmplitude = 0;
int maxAmplitude = 0;
//QElapsedTimer mSecTimer;
//qint64 timer1 = 0;
//qint64 timer2 = 0;
void MainWindow::on_audioLevel_Change(int decibelL,int decibelR)
{
//    if (timer2 - timer1 >= 1)
//    {
//    timer1 = timer2;
        if(_xSettings->isConnected())
        {
    //        strokerLastUpdate = time;
            auto availibleAxis = SettingsHandler::getAvailableAxis();
            auto decibelLInverse = -decibelL;
            auto decibelRInverse = -decibelR;
            auto difference = decibelLInverse > decibelRInverse ? decibelLInverse - decibelRInverse : decibelRInverse - decibelLInverse;
            int average = round(difference / 2);
            auto amplitude = decibelLInverse > decibelRInverse ? decibelLInverse - average : decibelRInverse - average;
            if(amplitude > minAmplitude || minAmplitude - amplitude > 25)
                minAmplitude = amplitude;
    //        if(amplitude > 0 && amplitude > maxAmplitude)
    //            maxAmplitude = amplitude;
            //auto delta = decibelLInverse > decibelRInverse ? amplitude / average : decibelRInverse - average;
            QString tcode;
            foreach(auto axis, availibleAxis->keys())
            {
                ChannelModel channel = availibleAxis->value(axis);
                if (channel.AxisName == _axisNames.Stroke  || SettingsHandler::getMultiplierChecked(axis))
                {
                    if (channel.Type == AxisType::HalfRange || channel.Type == AxisType::None)
                        continue;
                    auto multiplierValue = SettingsHandler::getMultiplierValue(axis);
                    if (channel.AxisName == _axisNames.Stroke)
                        multiplierValue = 1.0f;
                    auto angle = XMath::mapRange(amplitude * 2, minAmplitude, maxAmplitude, 0, 180);
                    auto magnifiedAmplitude = XMath::mapRange(amplitude * 2, minAmplitude, maxAmplitude, 0, 100);
                    auto value = XMath::constrain(XMath::randSine(angle * multiplierValue, magnifiedAmplitude), 0, 100);
                    //auto value = int(XMath::mapRange(amplitude, minAmplitude, maxAmplitude, 1, 100) * multiplierValue);

                    int distance = value >= lastAction ? value - lastAction : lastAction - value;
                    if(distance > 25)
                    {
                        lastAction = value;
    //                    LogHandler::Debug("value: "+QString::number(value));
    //                    LogHandler::Debug("distance: "+QString::number(distance));
    //                    LogHandler::Debug("amplitude: "+QString::number(amplitude));
    //                    LogHandler::Debug("minAmplitude: "+QString::number(minAmplitude));
    //                    LogHandler::Debug("decibelLInverse: "+QString::number(decibelLInverse));
    //                    LogHandler::Debug("decibelRInverse: "+QString::number(decibelRInverse));
    //                    LogHandler::Debug("difference: "+QString::number(difference));
                        auto time = QTime::currentTime().msecsSinceStartOfDay();
                        int speed = time - strokerLastUpdate;
                        strokerLastUpdate = time;
                        //LogHandler::Debug("speed: "+QString::number(speed));

                        char tcodeValueString[4];
                        sprintf(tcodeValueString, "%03d", tcodeHandler->calculateRange(axis.toUtf8(), value));
                        tcode += " ";
                        tcode += axis;
                        tcode += tcodeValueString;
                        tcode += "I";
                        float speedModifierValue = SettingsHandler::getDamperValue(axis);
                        if (SettingsHandler::getDamperChecked(axis) && speedModifierValue > 0.0 && speed > 1000 && distance > 50)
                        {
                            tcode += QString::number(round(speed * speedModifierValue));
                        }
                        else
                        {
                            tcode += QString::number(speed > 0 ? speed : 1000);
                        }
                    }
                }
            }
            if(!tcode.isEmpty())
                _xSettings->getSelectedDeviceHandler()->sendTCode(tcode);
        }
        //timer2 = (round(mSecTimer.nsecsElapsed() / 1000000));
    //}
}


void MainWindow::turnOffAudioSync()
{
    disconnect(audioSyncFilter, &AudioSyncFilter::levelChanged, this, &MainWindow::on_audioLevel_Change);
    minAmplitude = 0;
    maxAmplitude = 0;
//    strokerUpdateMillis = 1000;
//    strokerLastUpdate = 500;
}

void MainWindow::onLibraryWindowed_Clicked()
{
    _libraryDockMode = true;
    ui->libraryGrid->removeWidget(libraryList);
    ui->libraryGrid->removeWidget(randomizeLibraryButton);
    ui->libraryGrid->removeWidget(windowedLibraryButton);
    ui->libraryGrid->removeWidget(backLibraryButton);
    ui->libraryGrid->removeWidget(cancelEditPlaylistButton);
    ui->libraryGrid->removeWidget(editPlaylistButton);
    ui->libraryGrid->removeWidget(savePlaylistButton);
    ui->libraryGrid->removeWidget(libraryLoadingLabel);
    ((QGridLayout*)libraryWindow->layout())->addWidget(libraryList, 1, 0, 20, 12);
    ((QGridLayout*)libraryWindow->layout())->addWidget(backLibraryButton, 0, 0);
    ((QGridLayout*)libraryWindow->layout())->addWidget(randomizeLibraryButton, 0, 1);
    ((QGridLayout*)libraryWindow->layout())->addWidget(windowedLibraryButton, 0, ui->libraryGrid->columnCount() - 1);
    ((QGridLayout*)libraryWindow->layout())->addWidget(cancelEditPlaylistButton, 0, ui->libraryGrid->columnCount() - 2);
    ((QGridLayout*)libraryWindow->layout())->addWidget(editPlaylistButton, 0, ui->libraryGrid->columnCount() - 2);
    ((QGridLayout*)libraryWindow->layout())->addWidget(savePlaylistButton, 0, ui->libraryGrid->columnCount() - 3);
    ((QGridLayout*)libraryWindow->layout())->addWidget(libraryLoadingLabel, 0, 0, 21, 12);
    windowedLibraryButton->hide();
    ui->libraryFrame->hide();
    libraryWindow->show();
    if(SettingsHandler::getLibrarySortMode() != LibrarySortMode::RANDOM)
        randomizeLibraryButton->hide();
    else
        randomizeLibraryButton->show();

    if(selectedPlaylistItems.length() > 0)
    {
        backLibraryButton->show();
        if(_editPlaylistMode)
        {
            savePlaylistButton->show();
            editPlaylistButton->hide();
            cancelEditPlaylistButton->show();
        }
        else
        {
            savePlaylistButton->hide();
            editPlaylistButton->show();
            cancelEditPlaylistButton->hide();
        }
    }
    else
    {
        backLibraryButton->hide();
        savePlaylistButton->hide();
        editPlaylistButton->hide();
    }
}

void MainWindow::onLibraryWindowed_Closed()
{
    _libraryDockMode = false;
    libraryWindow->layout()->removeWidget(libraryList);
    ui->libraryGrid->addWidget(libraryList, 1, 0, 20, 12);
    ui->libraryGrid->addWidget(backLibraryButton, 0, 0);
    ui->libraryGrid->addWidget(randomizeLibraryButton, 0, 1);
    ui->libraryGrid->addWidget(windowedLibraryButton, 0, ui->libraryGrid->columnCount() - 1);
    ui->libraryGrid->addWidget(cancelEditPlaylistButton, 0, ui->libraryGrid->columnCount() - 2);
    ui->libraryGrid->addWidget(editPlaylistButton, 0, ui->libraryGrid->columnCount() - 2);
    ui->libraryGrid->addWidget(savePlaylistButton, 0, ui->libraryGrid->columnCount() - 3);
    ui->libraryGrid->addWidget(libraryLoadingLabel, 0, 0, 21, 12);
    windowedLibraryButton->show();
    ui->libraryFrame->show();
    if(SettingsHandler::getLibrarySortMode() != LibrarySortMode::RANDOM)
        randomizeLibraryButton->hide();
    else
        randomizeLibraryButton->show();

    if(selectedPlaylistItems.length() > 0)
    {
        backLibraryButton->show();
        if(_editPlaylistMode)
        {
            savePlaylistButton->show();
            editPlaylistButton->hide();
            cancelEditPlaylistButton->show();
        }
        else
        {
            savePlaylistButton->hide();
            editPlaylistButton->show();
            cancelEditPlaylistButton->hide();
        }
    }
    else
        backLibraryButton->hide();
}

void MainWindow::onLibraryList_ContextMenuRequested(const QPoint &pos)
{
    // Handle global position
    QPoint globalPos = libraryList->mapToGlobal(pos);

    // Create menu and insert some actions
    QMenu myMenu;

    QListWidgetItem* selectedItem = libraryList->selectedItems().first();
    LibraryListItem selectedFileListItem = ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();

    myMenu.addAction(tr("Play"), this, &MainWindow::playFileFromContextMenu);
    if(selectedFileListItem.type == LibraryListItemType::PlaylistInternal)
    {
        myMenu.addAction(tr("Open"), this, [this]()
        {
            QListWidgetItem* selectedItem = libraryList->selectedItems().first();
            LibraryListItem selectedFileListItem = ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();
            loadPlaylistIntoLibrary(selectedFileListItem.nameNoExtension);
        });
        myMenu.addAction(tr("Rename..."), this, &MainWindow::renamePlaylist);
        myMenu.addAction(tr("Delete..."), this, [this, selectedFileListItem]() {

            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, tr("WARNING!"), tr("Are you sure you want to delete the playlist: ") + selectedFileListItem.nameNoExtension,
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes)
            {
                deleteSelectedPlaylist();
            }
        });
    }
    if(selectedFileListItem.type != LibraryListItemType::PlaylistInternal)
    {
        if(selectedPlaylistItems.length() > 0)
        {
            myMenu.addAction(tr("Remove from playlist"), this, &MainWindow::removeFromPlaylist);
        }
        myMenu.addAction(tr("Play with funscript..."), this, &MainWindow::playFileWithCustomScript);
        // Experimental
        //myMenu.addAction("Play with audio sync (Experimental)", this, &MainWindow::playFileWithAudioSync);
        if(selectedPlaylistItems.length() == 0)
        {
            QMenu* subMenu = myMenu.addMenu(tr("Add to playlist"));
            subMenu->addAction(tr("New playlist..."), this, [this]()
            {
                QString playlist = getPlaylistName();
                if(!playlist.isEmpty())
                    addSelectedLibraryItemToPlaylist(playlist);
            });
            subMenu->addSeparator();
            auto playlists = SettingsHandler::getPlaylists();
            foreach(auto playlist, playlists.keys())
            {
                subMenu->addAction(playlist, this, [this, playlist]()
                {
                    addSelectedLibraryItemToPlaylist(playlist);
                });
            }
        }

        if(selectedFileListItem.type != LibraryListItemType::Audio)
        {
            myMenu.addAction(tr("Regenerate thumbnail"), this, &MainWindow::regenerateThumbNail);
            myMenu.addAction(tr("Set thumbnail from current"), this, &MainWindow::setThumbNailFromCurrent);
        }
        myMenu.addAction(tr("Set moneyshot from current"), this, [this, selectedFileListItem] () {
            onSetMoneyShot(selectedFileListItem, videoHandler->position());
        });
//        myMenu.addAction("Add bookmark from current", this, [this, selectedFileListItem] () {
//            onAddBookmark(selectedFileListItem, "Book mark 1", videoHandler->position());
//        });
        myMenu.addAction(tr("Reveal in directory"), this, [this, selectedFileListItem] () {
            showInGraphicalShell(selectedFileListItem.path);
        });
        myMenu.addAction(tr("Edit media settings..."), this, [this, selectedFileListItem] () {
            LibraryItemSettingsDialog::getSettings(this, selectedFileListItem.path);
        });
    }

    // Show context menu at handling position
    myMenu.exec(globalPos);
}

void MainWindow::changeDeoFunscript()
{
    VRPacket playingPacket = _xSettings->getDeoHandler()->getCurrentPacket();
    if (playingPacket.path != nullptr)
    {
        QFileInfo videoFile(playingPacket.path);
        funscriptFileSelectorOpen = true;
        QString funscriptPath = QFileDialog::getOpenFileName(this, tr("Choose script for video: ") + videoFile.fileName(), SettingsHandler::getSelectedLibrary(), "Script Files (*.funscript)");
        funscriptFileSelectorOpen = false;
        if (!funscriptPath.isEmpty())
        {
            SettingsHandler::setDeoDnlaFunscript(playingPacket.path, funscriptPath);
            funscriptHandler->setLoaded(false);
        }
    }
    else
    {
        LogHandler::Dialog(tr("No packet for current video or no video playing"), XLogLevel::Information);
    }
}

void MainWindow::on_load_library(QString path)
{
    if (!path.isNull() && !path.isEmpty())
    {
        QString thumbPath = SettingsHandler::getSelectedThumbsDir();
        QDir thumbDir(thumbPath);
        if (!thumbDir.exists())
        {
            thumbDir.mkdir(thumbPath);
        }
        QDir directory(path);
        if (directory.exists())
        {
            QStringList videoTypes = QStringList()
                    << "*.mp4"
                    << "*.avi"
                    << "*.mpg"
                    << "*.wmv"
                    << "*.mkv"
                    << "*.webm"
                    << "*.mp2"
                    << "*.mpeg"
                    << "*.mpv"
                    << "*.ogg"
                    << "*.m4p"
                    << "*.m4v"
                    << "*.mov"
                    << "*.qt"
                    << "*.flv"
                    << "*.swf"
                    << "*.avchd";

            QStringList audioTypes = QStringList()
                    << "*.m4a"
                    << "*.mp3"
                    << "*.aac"
                    << "*.flac"
                    << "*.wav"
                    << "*.wma";
            QStringList playlistTypes = QStringList()
                    << "*.m3u";

            QStringList mediaTypes;
            mediaTypes.append(videoTypes);
            mediaTypes.append(audioTypes);
            QDirIterator library(path, mediaTypes, QDir::Files, QDirIterator::Subdirectories);

            stopThumbProcess();
            cachedLibraryItems.clear();
            libraryList->clear();
            auto playlists = SettingsHandler::getPlaylists();
            foreach(auto playlist, playlists.keys())
            {
                setupPlaylistItem(playlist);
            }
            while (library.hasNext())
            {
                QFileInfo fileinfo(library.next());
                QString fileDir = fileinfo.dir().path();
                QList<QString> excludedLibraryPaths = SettingsHandler::getLibraryExclusions();
                bool isExcluded = false;
                foreach(QString dir, excludedLibraryPaths)
                {
                    if(dir != path && (fileDir.startsWith(dir, Qt::CaseInsensitive)))
                        isExcluded = true;
                }
                if (isExcluded)
                    continue;
                QString videoPath = fileinfo.filePath();
                QString videoPathTemp = fileinfo.filePath();
                QString fileName = fileinfo.fileName();
                QString fileNameTemp = fileinfo.fileName();
                QString fileNameNoExtension = fileNameTemp.remove(fileNameTemp.lastIndexOf('.'), fileNameTemp.length() -  1);
                QString scriptFile = fileNameNoExtension + ".funscript";
                QString scriptPath;
                QString scriptNoExtension = videoPathTemp.remove(videoPathTemp.lastIndexOf('.'), videoPathTemp.length() - 1);
                fileNameTemp = fileinfo.fileName();
                QString mediaExtension = "*" + fileNameTemp.remove(0, fileNameTemp.length() - (fileNameTemp.length() - fileNameTemp.lastIndexOf('.')));

                if (SettingsHandler::getSelectedFunscriptLibrary() == Q_NULLPTR)
                {
                    scriptPath = scriptNoExtension + ".funscript";
                }
                else
                {
                    scriptNoExtension = SettingsHandler::getSelectedFunscriptLibrary() + QDir::separator() + fileNameNoExtension;
                    scriptPath = SettingsHandler::getSelectedFunscriptLibrary() + QDir::separator() + scriptFile;
                }
                if (!funscriptHandler->exists(scriptPath))
                {
                    scriptPath = nullptr;
                }

                QFileInfo scriptZip(scriptNoExtension + ".zip");
                QString zipFile;
                if(scriptZip.exists())
                    zipFile = scriptNoExtension + ".zip";
                bool audioOnly = false;
                if(audioTypes.contains(mediaExtension))
                {
                    audioOnly = true;
                }
                LibraryListItem item
                {
                    audioOnly ? LibraryListItemType::Audio : LibraryListItemType::Video,
                    videoPath, // path
                    fileName, // name
                    fileNameNoExtension, //nameNoExtension
                    scriptPath, // script
                    scriptNoExtension,
                    mediaExtension,
                    nullptr,
                    zipFile,
                    fileinfo.birthTime().date(),
                    0
                };
                LibraryListWidgetItem* qListWidgetItem = new LibraryListWidgetItem(item);
                libraryList->addItem(qListWidgetItem);
                cachedLibraryItems.push_back((LibraryListWidgetItem*)qListWidgetItem->clone());
            }
            updateLibrarySortUI();
            startThumbProcess();
        }
        else
        {
           LogHandler::Dialog(tr("Library path '") + path + tr("' does not exist anymore!"), XLogLevel::Critical);
           on_actionSelect_library_triggered();
        }
    }
    else
    {
        on_actionSelect_library_triggered();
    }
}

void MainWindow::startThumbProcess()
{
    stopThumbProcess();
    thumbProcessIsRunning = true;
    extractor = new VideoFrameExtractor;
    thumbNailPlayer = new AVPlayer;
    thumbNailPlayer->setInterruptOnTimeout(true);
    thumbNailPlayer->setInterruptTimeout(10000);
    thumbNailPlayer->setAsyncLoad(true);
    extractor->setAsync(true);
    saveNewThumbs();
}

void MainWindow::stopThumbProcess()
{
    if(thumbProcessIsRunning)
    {
        disconnect(extractor, nullptr,  nullptr, nullptr);
        disconnect(thumbNailPlayer, nullptr,  nullptr, nullptr);
        thumbNailSearchIterator = 0;
        thumbProcessIsRunning = false;
        delete extractor;
        delete thumbNailPlayer;
    }
}

void MainWindow::saveSingleThumb(const QString& videoFile, const QString& thumbFile, LibraryListWidgetItem* qListWidgetItem, qint64 position)
{
    if(!thumbProcessIsRunning)
    {
        extractor = new VideoFrameExtractor;
        thumbNailPlayer = new AVPlayer;
        thumbNailPlayer->setInterruptOnTimeout(true);
        thumbNailPlayer->setInterruptTimeout(10000);
        thumbNailPlayer->setAsyncLoad(true);
        extractor->setAsync(true);
    }
    saveThumb(videoFile, thumbFile, qListWidgetItem, position);
}

void MainWindow::saveNewThumbs()
{
    if (thumbProcessIsRunning && thumbNailSearchIterator < cachedLibraryItems.count())
    {
        //Use a non user modifiable list incase they sort random when getting thumbs.
        LibraryListWidgetItem* listWidgetItem = cachedLibraryItems.at(thumbNailSearchIterator);
        LibraryListItem item = listWidgetItem->getLibraryListItem();
        thumbNailSearchIterator++;
        QFileInfo thumbInfo(item.thumbFile);
        if (item.type == LibraryListItemType::Video && !thumbInfo.exists())
        {
            disconnect(extractor, nullptr,  nullptr, nullptr);
            disconnect(thumbNailPlayer, nullptr,  nullptr, nullptr);
            saveThumb(item.path, item.thumbFile, listWidgetItem);
        }
        else
        {

            saveNewThumbs();
        }
    }
    else
    {
        stopThumbProcess();
    }
}
void MainWindow::saveThumb(const QString& videoFile, const QString& thumbFile, LibraryListWidgetItem* cachedListWidgetItem, qint64 position)
{
//    QIcon thumb;
//    QPixmap bgPixmap(QApplication::applicationDirPath() + "/themes/loading.png");
//    QPixmap scaled = bgPixmap.scaled(SettingsHandler::getThumbSize(), Qt::AspectRatioMode::KeepAspectRatio);
//    thumb.addPixmap(scaled);
//    qListWidgetItem->setIcon(thumb);
    LibraryListItem cachedListItem = cachedListWidgetItem->getLibraryListItem();
    auto text = cachedListWidgetItem->text();
    auto cachedLibraryListWidgetItem = cachedListWidgetItem->getLibraryListItem();
    auto libraryListItems = libraryList->findItems(cachedLibraryListWidgetItem.nameNoExtension, Qt::MatchFlag::MatchEndsWith);
    auto libraryListWidgetItem = ((LibraryListWidgetItem*)libraryListItems.first());
    QIcon thumb;
    QPixmap bgPixmap("://images/icons/loading_current.png");
    int thumbSize = SettingsHandler::getThumbSize();
    QSize size = {thumbSize, thumbSize};
    QPixmap scaled = bgPixmap.scaled(SettingsHandler::getMaxThumbnailSize(), Qt::AspectRatioMode::KeepAspectRatio);
    thumb.addPixmap(scaled);
    libraryListWidgetItem->setIcon(thumb);


    auto libraryListItem = libraryListWidgetItem->getLibraryListItem();

    //://images/icons/loading_current.png
    if(cachedListItem.type == LibraryListItemType::Audio)
    {
        int thumbSize = SettingsHandler::getThumbSize();
        QSize size = {thumbSize, thumbSize};
        cachedListWidgetItem->updateThumbSize(size);
        libraryList->removeItemWidget(libraryListWidgetItem);
        libraryListWidgetItem->updateThumbSize(size);

        saveNewThumbs();
    }
    else
    {
        connect(extractor,
           &QtAV::VideoFrameExtractor::frameExtracted,
           extractor,
           [this, videoFile, thumbFile, cachedListWidgetItem, libraryListWidgetItem](const QtAV::VideoFrame& frame)
            {
                if(frame.isValid())
                {
                    bool error = false;
                    QImage img;
                    try{
                        LogHandler::Debug(tr("Saving thumbnail: ") + thumbFile + tr(" for video: ") + videoFile);
                        img = frame.toImage();
//                        auto vf = VideoFormat::pixelFormatFromImageFormat(QImage::Format_ARGB32);
//                        auto vf2 = VideoFormat(vf);
//                        VideoFrame f = frame.to(vf2, {frame.width(), frame.height()}, QRect(0,0,frame.width(), frame.height()));
//                        QImage img(f.frameDataPtr(), f.width(), f.height(), f.bytesPerLine(0), QImage::Format_ARGB32);
                    }
                    catch (...) {
                        error = true;
                    }
                    QString thumbFileTemp = thumbFile;
                    if (error || img.isNull() || !img.save(thumbFile, nullptr, 15))
                    {
                       LogHandler::Debug(tr("Error saving thumbnail: ") + thumbFile + tr(" for video: ") + videoFile);
                       thumbFileTemp = "://images/icons/error.png";
                    }

                    int thumbSize = SettingsHandler::getThumbSize();
                    QSize size = {thumbSize, thumbSize};
                    cachedListWidgetItem->updateThumbSize(size, thumbFileTemp);
                    libraryList->removeItemWidget(libraryListWidgetItem);
                    libraryListWidgetItem->updateThumbSize(size, thumbFileTemp);
                }
               saveNewThumbs();
            });
        connect(extractor,
           &QtAV::VideoFrameExtractor::error,
           extractor,
           [this, videoFile, cachedListWidgetItem, libraryListWidgetItem](const QString &errorMessage)
            {

               LogHandler::Debug(tr("Error extracting image from: ") + videoFile + tr(" Error: ") + errorMessage);
               auto errorMsg = cachedListWidgetItem->toolTip() + tr("\n\nError: ") + errorMessage;

               auto thumbError = "://images/icons/error.png";
               cachedListWidgetItem->setToolTip(errorMsg);
               int thumbSize = SettingsHandler::getThumbSize();
               QSize size = {thumbSize,thumbSize};
               cachedListWidgetItem->updateThumbSize(size, thumbError);

               libraryList->removeItemWidget(libraryListWidgetItem);
               libraryListWidgetItem->setToolTip(errorMsg);
               libraryListWidgetItem->updateThumbSize(size, thumbError);

               saveNewThumbs();
            });

        extractor->setSource(videoFile);

        thumbNailPlayer->setFile(videoFile);
        thumbNailPlayer->load();

        connect(thumbNailPlayer,
           &AVPlayer::loaded,
           thumbNailPlayer,
           [this, position]()
            {
               LogHandler::Debug(tr("Loaded video for thumb duration: ") + QString::number(thumbNailPlayer->duration()));
               qint64 randomPosition = position > 0 ? position : XMath::rand((qint64)1, thumbNailPlayer->duration());
               //LogHandler::Debug("randomPosition: " + QString::number(randomPosition));
               extractor->setPosition(randomPosition);
            });

        connect(thumbNailPlayer,
           &AVPlayer::error,
           thumbNailPlayer,
           [this, cachedListWidgetItem, libraryListWidgetItem](QtAV::AVError er)
            {
               LogHandler::Debug(tr("Video load error"));
               auto errorMsg = cachedListWidgetItem->toolTip() + tr("\n\nError: ") + er.ffmpegErrorString();
               auto thumbError = "://images/icons/error.png";
               cachedListWidgetItem->setToolTip(errorMsg);
               int thumbSize = SettingsHandler::getThumbSize();
               QSize size = {thumbSize,thumbSize};
               cachedListWidgetItem->updateThumbSize(size, thumbError);

               libraryList->removeItemWidget(libraryListWidgetItem);
               libraryListWidgetItem->setToolTip(errorMsg);
               libraryListWidgetItem->updateThumbSize(size, thumbError);

               disconnect(extractor, nullptr,  nullptr, nullptr);
               saveNewThumbs();
            });
    }
}

void MainWindow::on_libray_path_select(QString path)
{
    on_load_library(path);
}

void MainWindow::on_actionSelect_library_triggered()
{
    QString currentPath = SettingsHandler::getSelectedLibrary();
    QDir currentDir(currentPath);
    QString defaultPath = !currentPath.isEmpty() && currentDir.exists() ? currentPath : ".";
    QString selectedLibrary = QFileDialog::getExistingDirectory(this, tr("Choose media library"), defaultPath, QFileDialog::ReadOnly);
    if (selectedLibrary != Q_NULLPTR)
    {
        on_libray_path_select(selectedLibrary);
        SettingsHandler::setSelectedLibrary(selectedLibrary);
    }
}

void MainWindow::on_LibraryList_itemClicked(QListWidgetItem *item)
{
    auto selectedItem = item->data(Qt::UserRole).value<LibraryListItem>();
    if(videoHandler->isPlaying() && !videoHandler->isPaused())
    {
        auto playingFile = videoHandler->file();
        _playerControlsFrame->setPlayIcon(playingFile == selectedItem.path);
    }
    ui->statusbar->showMessage(selectedItem.nameNoExtension);
    selectedLibraryListItem = (LibraryListWidgetItem*)item;
    selectedLibraryListIndex = libraryList->currentRow();
}

void MainWindow::regenerateThumbNail()
{
    QListWidgetItem* selectedItem = libraryList->selectedItems().first();
    LibraryListItem selectedFileListItem = ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();
    saveSingleThumb(selectedFileListItem.path, selectedFileListItem.thumbFile, (LibraryListWidgetItem*)selectedItem);
}

void MainWindow::setThumbNailFromCurrent()
{
    QListWidgetItem* selectedItem = libraryList->selectedItems().first();
    LibraryListItem selectedFileListItem = ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();
    saveSingleThumb(selectedFileListItem.path, selectedFileListItem.thumbFile, (LibraryListWidgetItem*)selectedItem, videoHandler->position());
}

void MainWindow::on_LibraryList_itemDoubleClicked(QListWidgetItem *item)
{
    auto libraryListItem = item->data(Qt::UserRole).value<LibraryListItem>();
    if(libraryListItem.type == LibraryListItemType::Audio || libraryListItem.type == LibraryListItemType::Video)
    {
        stopAndPlayVideo(item->data(Qt::UserRole).value<LibraryListItem>());
    }
    else if(libraryListItem.type == LibraryListItemType::PlaylistInternal)
    {
        loadPlaylistIntoLibrary(libraryListItem.nameNoExtension);
    }
}

void MainWindow::playFileFromContextMenu()
{
    LibraryListItem libraryListItem = ((LibraryListWidgetItem*)libraryList->selectedItems().first())->getLibraryListItem();
    if(libraryListItem.type == LibraryListItemType::Audio || libraryListItem.type == LibraryListItemType::Video)
    {
        stopAndPlayVideo(libraryListItem);
    }
    else if(libraryListItem.type == LibraryListItemType::PlaylistInternal)
    {
        loadPlaylistIntoLibrary(libraryListItem.nameNoExtension);
        if(selectedPlaylistItems.length() > 0)
        {
            LibraryListItem libraryListItem = setCurrentLibraryRow(0)->getLibraryListItem();
            stopAndPlayVideo(libraryListItem);
        }
    }
}

void MainWindow::playFileWithAudioSync()
{
    LibraryListItem selectedFileListItem = ((LibraryListWidgetItem*)libraryList->selectedItems().first())->getLibraryListItem();
    stopAndPlayVideo(selectedFileListItem, nullptr, true);
}

void MainWindow::playFileWithCustomScript()
{
    QString selectedScript = QFileDialog::getOpenFileName(this, tr("Choose script"), SettingsHandler::getSelectedLibrary(), tr("Scripts (*.funscript *.zip)"));
    if (selectedScript != Q_NULLPTR)
    {
        LibraryListItem selectedFileListItem = ((LibraryListWidgetItem*)libraryList->selectedItems().first())->getLibraryListItem();
        stopAndPlayVideo(selectedFileListItem, selectedScript);
    }
}
//Hack because QTAV calls stopped and start out of order
void MainWindow::stopAndPlayVideo(LibraryListItem selectedFileListItem, QString customScript, bool audioSync)
{
    QFile file(selectedFileListItem.path);
    if (file.exists())
    {
        if (videoHandler->file() != selectedFileListItem.path || !customScript.isEmpty() || audioSync)
        {
            if(playingLibraryListItem != nullptr)
                updateMetaData(playingLibraryListItem->getLibraryListItem());
            _playerControlsFrame->SetLoop(false);
            on_setLoading(true);
            if(videoHandler->isPlaying())
            {
                stopMedia();
                if(_waitForStopFuture.isRunning())
                {
                    _waitForStopFuture.cancel();
                    _waitForStopFutureCancel = true;
                    _waitForStopFuture.waitForFinished();
                }

                _waitForStopFuture = QtConcurrent::run([this, selectedFileListItem, customScript, audioSync]()
                {
                    while(!_mediaStopped)
                    {
                        LogHandler::Debug(tr("Waiting for media stop..."));
                        if(!_waitForStopFutureCancel)
                            QThread::msleep(500);
                        else {
                            _waitForStopFutureCancel = false;
                            return;
                        }
                    }
                    emit playVideo(selectedFileListItem, customScript, audioSync);
                });
            }
            else
            {
                on_playVideo(selectedFileListItem, customScript, audioSync);
            }
        }
    }

}
void MainWindow::on_playVideo(LibraryListItem selectedFileListItem, QString customScript, bool audioSync)
{
    QFile file(selectedFileListItem.path);
    if (file.exists())
    {
        if (videoHandler->file() != selectedFileListItem.path || !customScript.isEmpty())
        {
            deviceHome();
            _playerControlsFrame->SetLoop(false);
            funscriptHandler->setLoaded(false);
            on_setLoading(true);
            videoHandler->setFile(selectedFileListItem.path);
            videoPreviewWidget->setFile(selectedFileListItem.path);
            videoHandler->load();
            if(!audioSync)
            {
                turnOffAudioSync();
                QString customScriptNoextension = nullptr;
                if (!customScript.isEmpty())
                {
                    QString customScriptTemp = customScript;
                    customScriptNoextension = customScriptTemp.remove(customScriptTemp.lastIndexOf('.'), customScriptTemp.length() -  1);
                }
                QString scriptFile = customScript.isEmpty() ? selectedFileListItem.script : customScript;
                QString scriptFileNoExtension = customScript.isEmpty() ? selectedFileListItem.scriptNoExtension : customScriptNoextension;
                QFileInfo scriptFileNoExtensionInfo(scriptFileNoExtension);
                QString scriptNameNoextension = nullptr;
                scriptNameNoextension = scriptFileNoExtensionInfo.fileName();
                QZipReader zipFile(selectedFileListItem.zipFile, QIODevice::ReadOnly);
                QZipReader customZipFile(scriptFile, QIODevice::ReadOnly);
                QFileInfo scriptFileInfo(scriptFile);
                if(scriptFile.endsWith(".funscript") && scriptFileInfo.exists())
                {
                    funscriptHandler->load(scriptFile);
                }
                else if(scriptFile.endsWith(".zip") && customZipFile.isReadable())
                {
                   QByteArray data = customZipFile.fileData(scriptNameNoextension + ".funscript");
                   if (!data.isEmpty())
                   {
                       funscriptHandler->load(data);
                   }
                   else
                   {
                       LogHandler::Dialog(tr("Custom zip file missing main funscript."), XLogLevel::Warning);
                   }
                }
                else if(zipFile.isReadable())
                {
                    QByteArray data = zipFile.fileData(selectedFileListItem.nameNoExtension + ".funscript");
                    if (!data.isEmpty())
                    {
                        funscriptHandler->load(data);
                    }
                    else
                    {
                        LogHandler::Dialog(tr("Zip file missing main funscript."), XLogLevel::Warning);
                    }
                }

                if(funscriptHandlers.length() > 0)
                {
                    qDeleteAll(funscriptHandlers);
                    funscriptHandlers.clear();
                }

                TCodeChannels axisNames;
                auto availibleAxis = SettingsHandler::getAvailableAxis();
                foreach(auto axisName, availibleAxis->keys())
                {
                    auto trackName = availibleAxis->value(axisName).TrackName;
                    if(axisName == axisNames.Stroke || trackName.isEmpty())
                        continue;

                    QFileInfo fileInfo(scriptFileNoExtension + "." + trackName + ".funscript");
                    if(scriptFile.endsWith(".zip") && customZipFile.isReadable())
                    {
                       QByteArray data = customZipFile.fileData(scriptNameNoextension + "." + availibleAxis->value(axisName).TrackName  + ".funscript");
                       if (!data.isEmpty())
                       {
                           FunscriptHandler* otherFunscript = new FunscriptHandler(axisName);
                           otherFunscript->load(data);
                           funscriptHandlers.append(otherFunscript);
                       }
                    }
                    else if(fileInfo.exists())
                    {
                        FunscriptHandler* otherFunscript = new FunscriptHandler(axisName);
                        otherFunscript->load(fileInfo.absoluteFilePath());
                        funscriptHandlers.append(otherFunscript);
                    }
                    else if(zipFile.isReadable())
                    {
                       auto trackName = availibleAxis->value(axisName).TrackName;
                       if(trackName.isEmpty())
                           continue;
                       QByteArray data = zipFile.fileData(selectedFileListItem.nameNoExtension + "." + trackName + ".funscript");
                       if (!data.isEmpty())
                       {
                           FunscriptHandler* otherFunscript = new FunscriptHandler(axisName);
                           otherFunscript->load(data);
                           funscriptHandlers.append(otherFunscript);
                       }
                    }
                }
            }
            else
            {
                turnOffAudioSync();
                strokerLastUpdate = QTime::currentTime().msecsSinceStartOfDay();
                connect(audioSyncFilter, &AudioSyncFilter::levelChanged, this, &MainWindow::on_audioLevel_Change);
            }
            if(!SettingsHandler::getDisableNoScriptFound() && !audioSync && !funscriptHandler->isLoaded())
            {
                on_scriptNotFound(customScript);
            }
            videoHandler->play();
            playingLibraryListIndex = libraryList->currentRow();
            playingLibraryListItem = (LibraryListWidgetItem*)libraryList->item(playingLibraryListIndex);

            processMetaData(selectedFileListItem);

        }

    }
    else
    {
        LogHandler::Dialog(tr("File '") + selectedFileListItem.path + tr("' does not exist!"), XLogLevel::Critical);
    }
}

void MainWindow::processMetaData(LibraryListItem libraryListItem)
{
    auto libraryListItemMetaData = SettingsHandler::getLibraryListItemMetaData(libraryListItem.path);
    if(libraryListItemMetaData.lastLoopEnabled && libraryListItemMetaData.lastLoopStart > -1 && libraryListItemMetaData.lastLoopEnd > libraryListItemMetaData.lastLoopStart)
    {
        _playerControlsFrame->SetLoop(true);
    }
    SettingsHandler::setLiveOffset(libraryListItemMetaData.offset);
}

void MainWindow::updateMetaData(LibraryListItem libraryListItem)
{
    auto libraryListItemMetaData = SettingsHandler::getLibraryListItemMetaData(libraryListItem.path);
    libraryListItemMetaData.lastPlayPosition = videoHandler->position();
    libraryListItemMetaData.lastLoopEnabled = _playerControlsFrame->getAutoLoop();
    if(libraryListItemMetaData.lastLoopEnabled)
    {
        libraryListItemMetaData.lastLoopStart = _playerControlsFrame->getSeekSliderLowerValue();
        libraryListItemMetaData.lastLoopEnd = _playerControlsFrame->getSeekSliderUpperValue();
    }
    SettingsHandler::updateLibraryListItemMetaData(libraryListItemMetaData);
}

void MainWindow::on_mainwindow_change(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        QWindowStateChangeEvent* stateEvent = (QWindowStateChangeEvent*)event;
        if(stateEvent->oldState() == Qt::WindowState::WindowNoState)
        {
            LogHandler::Debug("oldState No state: "+QString::number(stateEvent->oldState()));
        }
        else if(stateEvent->oldState() == Qt::WindowState::WindowMaximized && !_isFullScreen && !_isMaximized)
        {
            LogHandler::Debug("oldState WindowMaximized: "+QString::number(stateEvent->oldState()));
            //QMainWindow::resize(_appSize);
            //QMainWindow::move(_appPos.x() < 10 ? 10 : _appPos.x(), _appPos.y() < 10 ? 10 : _appPos.y());
        }
        else if(stateEvent->oldState() == Qt::WindowState::WindowFullScreen && !_isMaximized)
        {
            LogHandler::Debug("WindowFullScreen to normal: "+QString::number(stateEvent->oldState()));
            QTimer::singleShot(200, [this]{
                QMainWindow::resize(_appSize);
                QMainWindow::move(_appPos.x() < 10 ? 10 : _appPos.x(), _appPos.y() < 10 ? 10 : _appPos.y());
               // videoHandler->resize(_videoSize);
            });
        }
        else if(stateEvent->oldState() == Qt::WindowState::WindowMaximized && !_isFullScreen && !QMainWindow::isMaximized())
        {
            LogHandler::Debug("WindowMaximized to normal");

            _isMaximized = false;
        }
    }
}
QPoint _mainStackedWidgetPos;
void MainWindow::toggleFullScreen()
{
    if(!_isFullScreen)
    {
        LogHandler::Debug("Before full VideoSize: width: "+QString::number(videoHandler->size().width()) + " height: " + QString::number(videoHandler->size().height()));
        QScreen *screen = this->window()->windowHandle()->screen();
        QSize screenSize = screen->size();
        _videoSize = videoHandler->size();
        _appSize = this->size();
        _appPos = this->pos();
        _mainStackedWidgetPos = ui->mainStackedWidget->pos();
        _isMaximized = this->isMaximized();
        _isFullScreen = true;
        //QMainWindow::setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        _mediaGrid->removeWidget(videoHandler);
        _mediaGrid->removeWidget(_videoLoadingLabel);
        _controlsHomePlaceHolderGrid->removeWidget(_playerControlsFrame);

        placeHolderControlsGrid = new QGridLayout(this);
        placeHolderControlsGrid->setContentsMargins(0,0,0,0);
        placeHolderControlsGrid->setSpacing(0);
        placeHolderControlsGrid->addWidget(_playerControlsFrame);

        playerControlsPlaceHolder = new QFrame(this);
        playerControlsPlaceHolder->setLayout(placeHolderControlsGrid);
        playerControlsPlaceHolder->setContentsMargins(0,0,0,0);
        playerControlsPlaceHolder->installEventFilter(this);
        playerControlsPlaceHolder->move(QPoint(0, screenSize.height() - _playerControlsFrame->height()));
        playerControlsPlaceHolder->setFixedWidth(screenSize.width());
        playerControlsPlaceHolder->setFixedHeight(_playerControlsFrame->height());
        playerControlsPlaceHolder->setFocusPolicy(Qt::StrongFocus);
        int rows = screenSize.height() / _playerControlsFrame->height();
        ui->fullScreenGrid->addWidget(videoHandler, 0, 0, rows, 5);
        ui->fullScreenGrid->addWidget(_videoLoadingLabel, (rows / 2) - 1, 2, 2, 1);
        ui->fullScreenGrid->addWidget(playerControlsPlaceHolder, rows - 1, 0, 1, 5);

        if(libraryWindow == nullptr || libraryWindow->isHidden())
        {
            libraryOverlay = true;
            placeHolderLibraryGrid = new QGridLayout(this);
            placeHolderLibraryGrid->setContentsMargins(0,0,0,0);
            placeHolderLibraryGrid->setSpacing(0);
            placeHolderLibraryGrid->addWidget(libraryList);

            playerLibraryPlaceHolder = new QFrame(this);
            playerLibraryPlaceHolder->setLayout(placeHolderLibraryGrid);
            playerLibraryPlaceHolder->setContentsMargins(0,0,0,0);
            playerLibraryPlaceHolder->installEventFilter(this);
            playerLibraryPlaceHolder->move(QPoint(0, screenSize.height()));
            playerLibraryPlaceHolder->setFixedWidth(ui->libraryFrame->width());
            playerLibraryPlaceHolder->setFixedHeight(screenSize.height() - _playerControlsFrame->height());
            ui->fullScreenGrid->addWidget(playerLibraryPlaceHolder, 0, 0, rows - 1, 2);
            libraryList->setProperty("cssClass", "fullScreenLibrary");
            _playerControlsFrame->style()->unpolish(libraryList);
            _playerControlsFrame->style()->polish(libraryList);
            hideLibrary();
        }

        _videoLoadingLabel->raise();
        _playerControlsFrame->setProperty("cssClass", "fullScreenControls");
        _playerControlsFrame->style()->unpolish(_playerControlsFrame);
        _playerControlsFrame->style()->polish(_playerControlsFrame);
        ui->mainStackedWidget->setCurrentIndex(1);
        QMainWindow::centralWidget()->layout()->setMargin(0);
        QMainWindow::showFullScreen();
        LogHandler::Debug("After full VideoSize: width: "+QString::number(videoHandler->size().width()) + " height: " + QString::number(videoHandler->size().height()));
        videoHandler->layout()->setMargin(0);
        //ui->mainStackedWidget->move(QPoint(0, 0));
        hideControls();
        ui->menubar->hide();
        ui->statusbar->hide();
        QMainWindow::setFocus();
    }
    else
    {
        ui->mainStackedWidget->setCurrentIndex(0);
        LogHandler::Debug("Before Normal VideoSize: width: "+QString::number(videoHandler->size().width()) + " height: " + QString::number(videoHandler->size().height()));
        ui->fullScreenGrid->removeWidget(videoHandler);
        _mediaGrid->addWidget(videoHandler, 0, 0, 3, 5);
        _mediaGrid->addWidget(_videoLoadingLabel, 1, 2);
        playerControlsPlaceHolder->layout()->removeWidget(_playerControlsFrame);
        ui->fullScreenGrid->removeWidget(playerControlsPlaceHolder);
        _videoLoadingLabel->raise();
        _playerControlsFrame->setWindowFlags(Qt::Widget);
        _controlsHomePlaceHolderGrid->addWidget(_playerControlsFrame);
        _playerControlsFrame->setProperty("cssClass", "windowedControls");
        _playerControlsFrame->style()->unpolish(_playerControlsFrame);
        _playerControlsFrame->style()->polish(_playerControlsFrame);
        libraryList->setProperty("cssClass", "windowedLibrary");
        libraryList->style()->unpolish(libraryList);
        libraryList->style()->polish(libraryList);

        if(libraryOverlay)
        {
            placeHolderLibraryGrid->removeWidget(libraryList);
            ui->fullScreenGrid->removeWidget(playerLibraryPlaceHolder);
            libraryList->setMinimumSize(QSize(0, 0));
            libraryList->setMaximumSize(QSize(16777215, 16777215));
            ui->libraryGrid->addWidget(libraryList, 0, 0, 20, 12);
            windowedLibraryButton->raise();
            randomizeLibraryButton->raise();
            libraryOverlay = false;
            delete placeHolderLibraryGrid;
            delete playerLibraryPlaceHolder;
        }

        videoHandler->layout()->setMargin(9);
        QMainWindow::centralWidget()->layout()->setMargin(9);


        if(_isMaximized)
        {
            QMainWindow::showMaximized();
        }
        else
        {
//            QMainWindow::showMaximized();
            QMainWindow::showNormal();
            //videoHandler->resize(_videoSize);
            //QMainWindow::resize(_appSize);
//            QMainWindow::move(_appPos.x() < 10 ? 10 : _appPos.x(), _appPos.y() < 10 ? 10 : _appPos.y());
//            this->repaint();
        }
        LogHandler::Debug("After Normal VideoSize: width: "+QString::number(videoHandler->size().width()) + " height: " + QString::number(videoHandler->size().height()));
        ui->menubar->show();
        ui->statusbar->show();
        _playerControlsFrame->show();
        libraryList->show();
        _isFullScreen = false;
        //QMainWindow::setWindowFlags(Qt::WindowFlags());
        delete placeHolderControlsGrid;
        delete playerControlsPlaceHolder;
    }
}

void MainWindow::hideControls()
{
    if (_isFullScreen)
    {
        _playerControlsFrame->hide();
    }
}

void MainWindow::showControls()
{
    if (_isFullScreen)
    {
        _playerControlsFrame->show();
    }
}

void MainWindow::hideLibrary()
{
    if (_isFullScreen)
    {
        libraryList->hide();
    }
}

void MainWindow::showLibrary()
{
    if (_isFullScreen)
    {
        libraryList->show();
    }
}

void MainWindow::on_VolumeSlider_valueChanged(int value)
{
    videoHandler->setVolume(value);
    SettingsHandler::setPlayerVolume(value);
}

void MainWindow::on_PlayBtn_clicked()
{
    if (libraryList->count() > 0)
    {
        if(libraryList->selectedItems().length() == 0)
        {
            LibraryListWidgetItem* selectedItem = setCurrentLibraryRow(0);
            stopAndPlayVideo(selectedItem->getLibraryListItem());
            return;
        }
        LibraryListWidgetItem* selectedItem = (LibraryListWidgetItem*)libraryList->selectedItems().first();
        LibraryListItem selectedFileListItem = selectedItem->getLibraryListItem();
        if(selectedFileListItem.path != videoHandler->file() || !videoHandler->isPlaying())
        {
            stopAndPlayVideo(selectedFileListItem);
        }
        else if(videoHandler->isPaused() || videoHandler->isPlaying())
        {
            videoHandler->togglePause();
        }
    }
}

LibraryListWidgetItem* MainWindow::setCurrentLibraryRow(int row)
{
    libraryList->setCurrentRow(row);
    on_LibraryList_itemClicked(libraryList->item(row));
    return (LibraryListWidgetItem*)libraryList->item(row);
}

void MainWindow::onVideoHandler_togglePaused(bool paused)
{
    _playerControlsFrame->setPlayIcon(!paused);
    if(paused)
        deviceSwitchedHome();
}

void MainWindow::stopMedia()
{
    if(videoHandler->isPlaying())
    {
        deviceHome();
        videoHandler->stop();
    }
}

void MainWindow::on_MuteBtn_toggled(bool checked)
{
    videoHandler->toggleMute();
}

void MainWindow::on_fullScreenBtn_clicked()
{
    MainWindow::toggleFullScreen();
}

void MainWindow::on_seekslider_hover(int position, int sliderValue)
{
//    if(!isFullScreen())
//    {
        qint64 sliderValueTime = XMath::mapRange(static_cast<qint64>(sliderValue), (qint64)0, (qint64)100, (qint64)0, videoHandler->duration());
    //    if (!videoPreviewWidget)
    //        videoPreviewWidget = new VideoPreviewWidget();
        videoPreviewWidget->setTimestamp(sliderValueTime);
        videoPreviewWidget->preview();
//        LogHandler::Debug("sliderValue: "+QString::number(sliderValue));
//        LogHandler::Debug("time: "+QString::number(sliderValueTime));
        //LogHandler::Debug("position: "+QString::number(position));
        QPoint gpos;
        if(_isFullScreen)
        {
            gpos = mapToGlobal(playerControlsPlaceHolder->pos() + _playerControlsFrame->getTimeSliderPosition() + QPoint(position, 0));
            QToolTip::showText(gpos, QTime(0, 0, 0).addMSecs(sliderValueTime).toString(QString::fromLatin1("HH:mm:ss")));
        }
        else
        {
            auto tootipPos = mapToGlobal(QPoint(ui->medialAndControlsFrame->pos().x(), 0) + _controlsHomePlaceHolderFrame->pos() + _playerControlsFrame->getTimeSliderPosition() + QPoint(position, 0));
            QToolTip::showText(tootipPos, QTime(0, 0, 0).addMSecs(sliderValueTime).toString(QString::fromLatin1("HH:mm:ss")));
            gpos = QPoint(ui->medialAndControlsFrame->pos().x(), 0) + _controlsHomePlaceHolderFrame->pos() + _playerControlsFrame->getTimeSliderPosition() + QPoint(position, 0);
        }

//        LogHandler::Debug("medialAndControlsFrame x: " + QString::number(ui->medialAndControlsFrame->pos().x()));
//        LogHandler::Debug("SeekSlider x: " + QString::number(ui->SeekSlider->pos().x()));
//        LogHandler::Debug("SeekSlider y: " + QString::number(ui->SeekSlider->pos().y()));
//        LogHandler::Debug("controlsHomePlaceHolder x: " + QString::number(ui->controlsHomePlaceHolder->pos().x()));
//        LogHandler::Debug("controlsHomePlaceHolder y: " + QString::number(ui->controlsHomePlaceHolder->pos().y()));
//        LogHandler::Debug("gpos x: " + QString::number(gpos.x()));
//        LogHandler::Debug("gpos y: " + QString::number(gpos.y()));
//        LogHandler::Debug("gpos - QPoint(176/2, 250) x: " + QString::number((gpos - QPoint(176/2, 250)).x()));
//        LogHandler::Debug("gpos - QPoint(176/2, 250) y: " + QString::number((gpos - QPoint(176/2, 250)).y()));

    //    if (!Config::instance().previewEnabled())
    //        return;

        //const int w = Config::instance().previewWidth();
        //const int h = Config::instance().previewHeight();
        //videoPreviewWidget->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        videoPreviewWidget->resize(176, 100);
        videoPreviewWidget->move(gpos - QPoint(176/2, 100));
        videoPreviewWidget->show();
        //videoPreviewWidget->raise();
        //videoPreviewWidget->activateWindow();
    //}
}

void MainWindow::on_seekslider_leave()
{
    if (!videoPreviewWidget)
    {
        return;
    }
    if (videoPreviewWidget->isVisible())
    {
        videoPreviewWidget->close();
    }
//    delete videoPreviewWidget;
//    videoPreviewWidget = NULL;
}

void MainWindow::on_seekSlider_sliderMoved(int position)
{
    LogHandler::Debug("position: "+ QString::number(position));
    if (!_playerControlsFrame->getAutoLoop())
    {
        qint64 playerPosition = XMath::mapRange(static_cast<qint64>(position), (qint64)0, (qint64)100, (qint64)0, videoHandler->duration());

        LogHandler::Debug("playerPosition: "+ QString::number(playerPosition));
        if(playerPosition <= 0)
            playerPosition = 50;
        videoHandler->seek(playerPosition);
    }
}


void MainWindow::onLoopRange_valueChanged(int position, int startLoop, int endLoop)
{
    if(endLoop >= 100)
        endLoop = 99;
    qint64 duration = videoHandler->duration();

    qint64 currentVideoPositionPercentage = XMath::mapRange(videoHandler->position(),  (qint64)0, duration, (qint64)0, (qint64)100);
    qint64 destinationVideoPosition = XMath::mapRange((qint64)position, (qint64)0, (qint64)100,  (qint64)0, duration);

    QString timeCurrent = mSecondFormat(destinationVideoPosition);
    _playerControlsFrame->setSeekSliderToolTip(timeCurrent);

    if(currentVideoPositionPercentage < startLoop)
    {
        videoHandler->setPosition(destinationVideoPosition);
    }
    else if (currentVideoPositionPercentage >= endLoop)
    {
        qint64 startLoopVideoPosition = XMath::mapRange((qint64)startLoop, (qint64)0, (qint64)100,  (qint64)0, duration);
        if(startLoopVideoPosition <= 0)
            startLoopVideoPosition = 50;
        if (videoHandler->position() != startLoopVideoPosition)
            videoHandler->setPosition(startLoopVideoPosition);
    }
}

void MainWindow::on_media_positionChanged(qint64 position)
{
    qint64 duration = videoHandler->duration();
    qint64 videoToSliderPosition = XMath::mapRange(position,  (qint64)0, duration, (qint64)0, (qint64)100);
    if (!_playerControlsFrame->getAutoLoop())
    {
        if (duration > 0)
        {
            _playerControlsFrame->setSeekSliderUpperValue(static_cast<int>(videoToSliderPosition));
        }
    }
    else
    {
        int endLoop = _playerControlsFrame->getSeekSliderUpperValue();
        qint64 endLoopToVideoPosition = XMath::mapRange((qint64)endLoop, (qint64)0, (qint64)100,  (qint64)0, duration);
        if (position >= endLoopToVideoPosition || (endLoop == 100 && position >= duration - 500))
        {
            int startLoop = _playerControlsFrame->getSeekSliderLowerValue();
            qint64 startLoopVideoPosition = XMath::mapRange((qint64)startLoop, (qint64)0, (qint64)100,  (qint64)0, duration);
            if(startLoopVideoPosition <= 0)
                startLoopVideoPosition = 50;
            if (videoHandler->position() != startLoopVideoPosition)
                videoHandler->setPosition(startLoopVideoPosition);
        }
        QPoint gpos;
        qint64 videoToSliderPosition = XMath::mapRange(position,  (qint64)0, duration, (qint64)0, (qint64)100);
        int hoverposition = XMath::mapRange((int)videoToSliderPosition,  (int)0, (int)100, (int)0, _playerControlsFrame->getSeekSliderWidth()) - 15;
        if(_isFullScreen)
        {
            gpos = mapToGlobal(playerControlsPlaceHolder->pos() + _playerControlsFrame->getTimeSliderPosition() + QPoint(hoverposition, 0));
            QToolTip::showText(gpos, QTime(0, 0, 0).addMSecs(position).toString(QString::fromLatin1("HH:mm:ss")), this);
        }
        else
        {
            auto tootipPos = mapToGlobal(QPoint(ui->medialAndControlsFrame->pos().x(), 0) + _controlsHomePlaceHolderFrame->pos() + _playerControlsFrame->getTimeSliderPosition() + QPoint(hoverposition, 0));
            QToolTip::showText(tootipPos, QTime(0, 0, 0).addMSecs(position).toString(QString::fromLatin1("HH:mm:ss")), this);
            gpos = QPoint(ui->medialAndControlsFrame->pos().x(), 0) + _controlsHomePlaceHolderFrame->pos() + _playerControlsFrame->getTimeSliderPosition() + QPoint(hoverposition, 0);
        }
    }
    _playerControlsFrame->setTimeDuration(mSecondFormat(position), mSecondFormat(duration));
    //    QString timeCurrent = QTime(0, 0, 0).addMSecs(position).toString(QString::fromLatin1("HH:mm:ss"));
    //    QString timeDuration = QTime(0, 0, 0).addMSecs(duration).toString(QString::fromLatin1("HH:mm:ss"));
    //    QString timeStamp = timeCurrent.append("/").append(timeDuration);
    //    ui->lblCurrentDuration->setText(timeStamp);
}

QString MainWindow::mSecondFormat(int mSecs)
{
    int seconds = mSecs / 1000;
    mSecs %= 1000;

    int minutes = seconds / 60;
    seconds %= 60;

    int hours = minutes / 60;
    minutes %= 60;
    QString hr = QString::number(hours);
    QString mn = QString::number(minutes);
    QString sc = QString::number(seconds);

    return (hr.length() == 1 ? "0" + hr : hr ) + ":" + (mn.length() == 1 ? "0" + mn : mn ) + ":" + (sc.length() == 1 ? "0" + sc : sc);
}

void MainWindow::on_media_start()
{
    LogHandler::Debug("Enter on_media_start");
    if(SettingsHandler::getDeoEnabled())
        _xSettings->getDeoHandler()->dispose();
    if(SettingsHandler::getWhirligigEnabled())
        _xSettings->getWhirligigHandler()->dispose();
    if(funscriptFuture.isRunning())
    {
        funscriptFuture.cancel();
        funscriptFuture.waitForFinished();
    }
    if (funscriptHandler->isLoaded())
    {
        funscriptFuture = QtConcurrent::run(syncFunscript, videoHandler, _xSettings, tcodeHandler, funscriptHandler, funscriptHandlers);
    }
    on_setLoading(false);
    _playerControlsFrame->resetMediaControlStatus(true);
    _mediaStopped = false;
}

void MainWindow::on_media_stop()
{
    LogHandler::Debug("Enter on_media_stop");
    on_setLoading(false);
    _playerControlsFrame->resetMediaControlStatus(false);
    if(funscriptFuture.isRunning())
    {
        funscriptFuture.cancel();
        funscriptFuture.waitForFinished();
    }
    _mediaStopped = true;
}
void MainWindow::on_setLoading(bool loading)
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

void MainWindow::on_scriptNotFound(QString message)
{
    NoMatchingScriptDialog::show(this, message);
}

void MainWindow::setLibraryLoading(bool loading)
{
    if(loading)
    {
        //libraryList->hide();
        libraryLoadingLabel->show();
        libraryLoadingMovie->start();
    }
    else
    {
        libraryLoadingLabel->hide();
        libraryLoadingMovie->stop();
        //libraryList->show();
    }
    randomizeLibraryButton->setDisabled(loading);
    savePlaylistButton->setDisabled(loading);
    editPlaylistButton->setDisabled(loading);
    cancelEditPlaylistButton->setDisabled(loading);
    windowedLibraryButton->setDisabled(loading);
    ui->actionSelect_library->setDisabled(loading);
    ui->actionReload_library->setDisabled(loading);
}

void MainWindow::onVRMessageRecieved(VRPacket packet)
{
        //LogHandler::Debug("VR path: "+packet.path);
//LogHandler::Debug("VR duration: "+QString::number(packet.duration));
        //LogHandler::Debug("VR currentTime: "+QString::number(packet.currentTime));
//        LogHandler::Debug("VR playbackSpeed: "+QString::number(packet.playbackSpeed));
//        LogHandler::Debug("VR playing: "+QString::number(packet.playing));

    if(!packet.path.isEmpty() && packet.path != vrScriptSelectedCanceledPath)
        vrScriptSelectorCanceled = false;

    if (!vrScriptSelectorCanceled && !packet.path.isEmpty() && !funscriptFileSelectorOpen && packet.duration > 0)
    {
        QString videoPath = packet.path;
        QFileInfo videoFile(videoPath);
        QString funscriptPath = SettingsHandler::getDeoDnlaFunscript(videoPath);
        QFileInfo funscriptFile(funscriptPath);
        if (funscriptPath == nullptr || !funscriptFile.exists())
        {
            funscriptHandler->setLoaded(false);
            //Check the deo device local video directory for funscript.
            int indexOfSuffix = packet.path.lastIndexOf(".");
            QString packetPath = packet.path;
            QString localFunscriptPath = packetPath.replace(indexOfSuffix, packet.path.length() - indexOfSuffix, ".funscript");
            QFile localFile(localFunscriptPath);
            LogHandler::Debug("Searching local path: "+localFunscriptPath);
            if(localFile.exists())
            {
                funscriptPath = localFunscriptPath;
            }
            else
            {
                //Check the user selected library location.
                QFileInfo fileinfo(packet.path);
                QString libraryScriptFile = fileinfo.fileName().remove(fileinfo.fileName().lastIndexOf('.'), fileinfo.fileName().length() -  1) + ".funscript";
                QString libraryScriptPath = SettingsHandler::getSelectedLibrary() + QDir::separator() + libraryScriptFile;
                QFile libraryFile(libraryScriptPath);
                if(libraryFile.exists())
                {
                    funscriptPath = libraryScriptPath;
                }
            }
            //If the above locations fail ask the user to select a file manually.
            if (funscriptPath.isEmpty())
            {
                if(SettingsHandler::getDisableVRScriptSelect())
                {
                    if (!SettingsHandler::getDisableSpeechToText())
                        textToSpeech->say("Script for video playing in Deeo VR not found. Please check your computer to select a script.");
                    funscriptFileSelectorOpen = true;
                    funscriptPath = QFileDialog::getOpenFileName(this, "Choose script for video: " + videoFile.fileName(), SettingsHandler::getSelectedLibrary(), "Script Files (*.funscript)");
                    funscriptFileSelectorOpen = false;
                    //LogHandler::Debug("funscriptPath: "+funscriptPath);
                }
                if(funscriptPath.isEmpty())
                {
                    vrScriptSelectorCanceled = true;
                    vrScriptSelectedCanceledPath = packet.path;
                }
            }
            //Store the location of the file so the above check doesnt happen again.
            if(!funscriptPath.isEmpty())
                SettingsHandler::setDeoDnlaFunscript(videoPath, funscriptPath);
        }
    }
}

void syncVRFunscript(VRDeviceHandler* vrPlayer, VideoHandler* xPlayer, SettingsDialog* xSettings, TCodeHandler* tcodeHandler, FunscriptHandler* funscriptHandler)
{
    if(xPlayer->isPlaying())
    {
        xPlayer->stop();
        funscriptHandler->setLoaded(false);
    }
    TCodeChannels axisNames;
    DeviceHandler* device = xSettings->getSelectedDeviceHandler();
    QList<FunscriptHandler*> funscriptHandlers;
    std::shared_ptr<FunscriptAction> actionPosition;
    QMap<QString, std::shared_ptr<FunscriptAction>> otherActions;
    VRPacket currentVRPacket = vrPlayer->getCurrentPacket();
    QString currentVideo;
    qint64 timeTracker = 0;
    qint64 lastVRTime = 0;
    QElapsedTimer mSecTimer;
    qint64 timer1 = 0;
    qint64 timer2 = 0;
    bool deviceHomed = false;
    //qint64 elapsedTracker = 0;
//    QElapsedTimer timer;
//    timer.start();
    mSecTimer.start();
    while (vrPlayer->isConnected() && !xPlayer->isPlaying())
    {
        //timer.start();
        if(!SettingsHandler::getLiveActionPaused() && xSettings->isConnected() && funscriptHandler->isLoaded() && !currentVRPacket.path.isNull() && currentVRPacket.duration > 0 && currentVRPacket.playing)
        {
            //execute once every millisecond
            if (timer2 - timer1 >= 1)
            {
//                LogHandler::Debug("timer1: "+QString::number(timer1));
//                LogHandler::Debug("timer2: "+QString::number(timer2));
//                LogHandler::Debug("timer2 - timer1 "+QString::number(timer2-timer1));
//                LogHandler::Debug("Out timeTracker: "+QString::number(timeTracker));
                timer1 = timer2;
                qint64 currentTime = currentVRPacket.currentTime;
                //LogHandler::Debug("VR time reset: "+QString::number(currentTime));
                bool hasRewind = lastVRTime > currentTime;
                if (currentTime > timeTracker + 100 || hasRewind)
                {
                    lastVRTime = currentTime;
                    LogHandler::Debug("current time reset: " + QString::number(currentTime));
                    LogHandler::Debug("timeTracker: " + QString::number(timeTracker));
                    timeTracker = currentTime;
                }
                else
                {
                    timeTracker++;
                    currentTime = timeTracker;
                }
                //LogHandler::Debug("funscriptHandler->getPosition: "+QString::number(currentTime));
                actionPosition = funscriptHandler->getPosition(currentTime);
                if(actionPosition != nullptr)
                    xSettings->setAxisProgressBar(axisNames.Stroke, actionPosition->pos);
                foreach(auto funscriptHandlerOther, funscriptHandlers)
                {
                    auto otherAction = funscriptHandlerOther->getPosition(currentTime);
                    if(otherAction != nullptr)
                    {
                        otherActions.insert(funscriptHandlerOther->channel(), otherAction);
                        xSettings->setAxisProgressBar(funscriptHandlerOther->channel(), otherAction->pos);
                    }
                }
                QString tcode = tcodeHandler->funscriptToTCode(actionPosition, otherActions);
                if(tcode != nullptr)
                    device->sendTCode(tcode);
                otherActions.clear();
           /*     LogHandler::Debug("timer "+QString::number((round(timer.nsecsElapsed()) / 1000000)));
                timer.start()*/;
            }
            timer2 = (round(mSecTimer.nsecsElapsed() / 1000000));
            //LogHandler::Debug("timer nsecsElapsed: "+QString::number(timer2));
        }
        else if(!funscriptHandler->isLoaded() && xSettings->isConnected() && currentVRPacket.duration > 0 && currentVRPacket.playing)
        {
            if (!currentVRPacket.path.isEmpty() && !currentVRPacket.path.isNull())
            {
                QString funscriptPath = SettingsHandler::getDeoDnlaFunscript(currentVRPacket.path);
                currentVideo = currentVRPacket.path;
                funscriptHandler->load(funscriptPath);

                qDeleteAll(funscriptHandlers);
                funscriptHandlers.clear();
                if(!deviceHomed)
                {
                    deviceHomed = true;
                    xSettings->getSelectedDeviceHandler()->sendTCode(tcodeHandler->getRunningHome());
                }

                auto availibleAxis = SettingsHandler::getAvailableAxis();
                foreach(auto axisName, availibleAxis->keys())
                {
                    auto trackName = availibleAxis->value(axisName).TrackName;
                    if(axisName == axisNames.Stroke || trackName.isEmpty())
                        continue;
                    QString funscriptPathTemp = funscriptPath;
                    auto funscriptNoExtension = funscriptPathTemp.remove(funscriptPathTemp.lastIndexOf('.'), funscriptPathTemp.length() -  1);
                    QFileInfo fileInfo(funscriptNoExtension + "." + trackName + ".funscript");
                    if(fileInfo.exists())
                    {
                        FunscriptHandler* otherFunscript = new FunscriptHandler(axisName);
                        otherFunscript->load(fileInfo.absoluteFilePath());
                        funscriptHandlers.append(otherFunscript);
                    }
                }
            }
        }

        if(currentVideo != currentVRPacket.path)
        {
            currentVideo = currentVRPacket.path;
            funscriptHandler->setLoaded(false);
        }

        //LogHandler::Debug("Get deo packet: "+QString::number((round(timer.nsecsElapsed()) / 1000000)));
        currentVRPacket = vrPlayer->getCurrentPacket();
        //QThread::currentThread()->usleep(10);
        //LogHandler::Debug("After get deo packet: "+QString::number((round(timer.nsecsElapsed()) / 1000000)));
        //QThread::currentThread()->msleep(1);
    }
    xSettings->resetAxisProgressBars();
    LogHandler::Debug("exit syncDeoFunscript");
}

void syncFunscript(VideoHandler* player, SettingsDialog* xSettings, TCodeHandler* tcodeHandler, FunscriptHandler* funscriptHandler, QList<FunscriptHandler*> funscriptHandlers)
{
    TCodeChannels axisNames;
    std::shared_ptr<FunscriptAction> actionPosition;
    QMap<QString, std::shared_ptr<FunscriptAction>> otherActions;
    DeviceHandler* device = xSettings->getSelectedDeviceHandler();
    QElapsedTimer mSecTimer;
    qint64 timer1 = 0;
    qint64 timer2 = 0;
    mSecTimer.start();
    while (player->isPlaying())
    {
        if (timer2 - timer1 >= 1)
        {
            timer1 = timer2;
            if(!SettingsHandler::getLiveActionPaused() && xSettings->isConnected())
            {
                qint64 currentTime = player->position();
                actionPosition = funscriptHandler->getPosition(currentTime);
                if(actionPosition != nullptr)
                    xSettings->setAxisProgressBar(axisNames.Stroke, actionPosition->pos);
                foreach(auto funscriptHandlerOther, funscriptHandlers)
                {
                    auto otherAction = funscriptHandlerOther->getPosition(currentTime);
                    if(otherAction != nullptr)
                    {
                        otherActions.insert(funscriptHandlerOther->channel(), otherAction);
                        xSettings->setAxisProgressBar(funscriptHandlerOther->channel(), otherAction->pos);
                    }
                }
                QString tcode = tcodeHandler->funscriptToTCode(actionPosition, otherActions);
                if(tcode != nullptr)
                    device->sendTCode(tcode);
                otherActions.clear();
            }
        }
        timer2 = (round(mSecTimer.nsecsElapsed() / 1000000));
        QThread::currentThread()->usleep(500);
    }
    xSettings->resetAxisProgressBars();
    LogHandler::Debug("exit syncFunscript");
}

void MainWindow::on_gamepad_sendTCode(QString value)
{
    if(_xSettings->isConnected())
    {
        if(SettingsHandler::getFunscriptLoaded(_axisNames.Stroke) && (videoHandler->isPlaying() || _xSettings->getDeoHandler()->isPlaying()))
        {
            QRegularExpression rx("L0[^\\s]*\\s?");
            value = value.remove(rx);
        }
        _xSettings->getSelectedDeviceHandler()->sendTCode(value);
    }
}

void MainWindow::on_gamepad_sendAction(QString action)
{
    mediaAction(action);
}

void MainWindow::on_skipForwardButton_clicked()
{
    skipForward();
}

void MainWindow::on_skipBackButton_clicked()
{
    skipBack();
}

void MainWindow::on_media_statusChanged(MediaStatus status)
{
    switch (status) {
    case EndOfMedia:
        if (!_playerControlsFrame->getAutoLoop())
            skipForward();
    break;
    case NoMedia:
        //status = tr("No media");
        break;
    case InvalidMedia:
        //status = tr("Invalid meida");
        break;
    case BufferingMedia:
        on_setLoading(true);
        break;
    case BufferedMedia:
        on_setLoading(false);
        break;
    case LoadingMedia:
        on_setLoading(true);
        break;
    case LoadedMedia:
        on_setLoading(false);
        break;
    case StalledMedia:

        break;
    default:
        //status = QString();
        //onStopPlay();
        break;
    }
}

void MainWindow::skipForward()
{
    if (libraryList->count() > 0)
    {
        LibraryListWidgetItem* item;
        int index = libraryList->currentRow() + 1;
        if(index < libraryList->count())
        {
            item = setCurrentLibraryRow(index);
        }
        else
        {
            item = setCurrentLibraryRow(0);
        }

        auto libraryListItem = item->getLibraryListItem();
        if(libraryListItem.type == LibraryListItemType::PlaylistInternal)
        {
            skipForward();
        }
        else
            stopAndPlayVideo(libraryListItem);
    }
}

void MainWindow::skipBack()
{
    if (libraryList->count() > 0)
    {
        LibraryListWidgetItem* item;
        int index = libraryList->currentRow() - 1;
        if(index >= 0)
        {
            item = setCurrentLibraryRow(index);
        }
        else
        {
            item = setCurrentLibraryRow(libraryList->count() - 1);
        }

        auto libraryListItem = item->getLibraryListItem();
        if(libraryListItem.type == LibraryListItemType::PlaylistInternal)
        {
            skipBack();
        }
        else
            stopAndPlayVideo(libraryListItem);
    }
}

void MainWindow::rewind()
{
    qint64 position = videoHandler->position();
    qint64 videoIncrement = SettingsHandler::getVideoIncrement() * 1000;
    if (position > videoIncrement)
        videoHandler->seek(videoHandler->position() - videoIncrement);
    else
        skipBack();
}

void MainWindow::fastForward()
{
    qint64 position = videoHandler->position();
    qint64 videoIncrement = SettingsHandler::getVideoIncrement() * 1000;
    if (position < videoHandler->duration() - videoIncrement)
        videoHandler->seek(videoHandler->position() + videoIncrement);
    else
        skipForward();
}

void MainWindow::media_double_click_event(QMouseEvent * event)
{
    if ( event->button() == Qt::LeftButton )
    {
        MainWindow::toggleFullScreen();
    }
}

void MainWindow::media_single_click_event(QMouseEvent * event)
{
    if (event->button() == Qt::MouseButton::RightButton)
    {
        videoHandler->togglePause();
    }
}

void MainWindow::on_device_connectionChanged(ConnectionChangedSignal event)
{
    deviceConnected = event.status == ConnectionStatus::Connected;
    QString message = "";
    if (event.deviceType == DeviceType::Serial)
    {
        message += "Serial: ";
    }
    else if(event.deviceType == DeviceType::Network)
    {
        message += "Network: ";
    }
    message += " " + event.message;
    connectionStatusLabel->setText(message);
    if(event.status == ConnectionStatus::Error || event.status == ConnectionStatus::Disconnected)
    {
        retryConnectionButton->show();
    }
    else
    {
        retryConnectionButton->hide();
    }
}

void MainWindow::on_gamepad_connectionChanged(ConnectionChangedSignal event)
{
    QString message = "";
    message += "Gamepad: ";
    message += " " + event.message;
    QPixmap bgPixmap;
    if(event.status == ConnectionStatus::Connected)
    {
        bgPixmap.load(("://images/gamepad-icon.png"));
    }
    else if(event.status == ConnectionStatus::Disconnected)
    {

        bgPixmap.load(("://images/gamepad-icon-disconnected.png"));
    }
    else if(event.status == ConnectionStatus::Connecting)
    {
        bgPixmap.load(("://images/gamepad-icon-disconnected.png"));
    }
    QPixmap scaled = bgPixmap.scaled({40, 30}, Qt::AspectRatioMode::KeepAspectRatio);
    gamepadConnectionStatusLabel->setPixmap(scaled);
    gamepadConnectionStatusLabel->setToolTip(message);
}

void MainWindow::on_device_error(QString error)
{
    LogHandler::Dialog(error, XLogLevel::Critical);
}

void MainWindow::on_deo_device_connectionChanged(ConnectionChangedSignal event)
{
    QString message = "";
    message += "DeoVR: ";
    message += " " + event.message;
    deoConnectionStatusLabel->setText(message);
    if(SettingsHandler::getDeoEnabled() && (event.status == ConnectionStatus::Error || event.status == ConnectionStatus::Disconnected))
    {
        ui->actionChange_current_deo_script->setEnabled(false);
        deoRetryConnectionButton->show();
        if(funscriptVRSyncFuture.isRunning())
        {
            funscriptVRSyncFuture.cancel();
            funscriptVRSyncFuture.waitForFinished();
        }
    }
    else if(event.status == ConnectionStatus::Connected)
    {
        ui->actionChange_current_deo_script->setEnabled(true);
        deoRetryConnectionButton->hide();
        if(funscriptVRSyncFuture.isRunning())
        {
            funscriptVRSyncFuture.cancel();
            funscriptVRSyncFuture.waitForFinished();
        }
        funscriptVRSyncFuture = QtConcurrent::run(syncVRFunscript, _xSettings->getDeoHandler(), videoHandler, _xSettings, tcodeHandler, funscriptHandler);

    }
    else if(event.status == ConnectionStatus::Connecting)
    {
        deoConnectionStatusLabel->show();
    }
    else
    {
        ui->actionChange_current_deo_script->setEnabled(false);
        deoConnectionStatusLabel->hide();
        deoRetryConnectionButton->hide();
    }
}

void MainWindow::on_deo_device_error(QString error)
{
    LogHandler::Dialog("Deo error: "+error, XLogLevel::Critical);
}

void MainWindow::on_whirligig_device_connectionChanged(ConnectionChangedSignal event)
{
    QString message = "";
    message += "Whirligig: ";
    message += " " + event.message;
    whirligigConnectionStatusLabel->setText(message);
    if(SettingsHandler::getWhirligigEnabled() && (event.status == ConnectionStatus::Error || event.status == ConnectionStatus::Disconnected))
    {
        ui->actionChange_current_deo_script->setEnabled(false);
        whirligigRetryConnectionButton->show();
        if(funscriptVRSyncFuture.isRunning())
        {
            funscriptVRSyncFuture.cancel();
            funscriptVRSyncFuture.waitForFinished();
        }
    }
    else if(event.status == ConnectionStatus::Connected)
    {
        ui->actionChange_current_deo_script->setEnabled(true);
        whirligigRetryConnectionButton->hide();
        if(funscriptVRSyncFuture.isRunning())
        {
            funscriptVRSyncFuture.cancel();
            funscriptVRSyncFuture.waitForFinished();
        }
        funscriptVRSyncFuture = QtConcurrent::run(syncVRFunscript, _xSettings->getWhirligigHandler(), videoHandler, _xSettings, tcodeHandler, funscriptHandler);

    }
    else if(event.status == ConnectionStatus::Connecting)
    {
        whirligigConnectionStatusLabel->show();
    }
    else
    {
        ui->actionChange_current_deo_script->setEnabled(false);
        whirligigConnectionStatusLabel->hide();
        whirligigRetryConnectionButton->hide();
    }
}

void MainWindow::on_whirligig_device_error(QString error)
{
    LogHandler::Dialog("Whirligig error: "+error, XLogLevel::Critical);
}


void MainWindow::on_actionAbout_triggered()
{
    QDialog aboutWindow;
    QGridLayout layout;
    QRect windowRect;
    windowRect.setSize({300, 200});
    layout.setGeometry(windowRect);
    QLabel copyright;
    copyright.setText("<b>XTPlayer v"+SettingsHandler::XTPVersion + "</b><br>"
                       + SettingsHandler::TCodeVersion + "<br>"
                                                "Copyright 2020 Jason C. Fain<br>"
                                                "Donate: <a href='https://www.patreon.com/Khrull'>https://www.patreon.com/Khrull</a><br>"
                                                "THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND.");
    copyright.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&copyright);
    QLabel sources;
    sources.setText("This software uses libraries from:");
    sources.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&sources);
    QLabel qtInfo;
    qtInfo.setFrameStyle(QFrame::Panel | QFrame::Sunken);
    qtInfo.setText("<b>Qt v5.15.0</b><br>"
                   "Distributed under the terms of LGPLv3 or later.<br>"
                   "Source: <a href='https://github.com/qt/qt5/releases/tag/v5.15.0'>https://github.com/qt/qt5/releases/tag/v5.15.0</a>");
    qtInfo.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&qtInfo);
    QLabel qtAVInfo;
    qtAVInfo.setFrameStyle(QFrame::Panel | QFrame::Sunken);
    qtAVInfo.setText("<b>QtAV 1.12.0(Aug 17 2020, 22:01:37)</b><br>"
                     "Multimedia framework base on Qt and FFmpeg.<br>"
                     "Distributed under the terms of LGPLv2.1 or later.<br>"
                     "Shanghai University->S3 Graphics->Deepin->PPTV<br>"
                     "Shanghai, China<br>"
                     "Copyright (C) 2012-2016 Wang Bin (aka. Lucas Wang)<br>"
                     "Email: <a href='mailto:wbsecg1@gmail.com'>wbsecg1@gmail.com</a><br>"
                     "Donate: <a href='http://qtav.org/donate.html'>http://qtav.org/donate.html</a><br>"
                     "Source: <a href='https://github.com/wang-bin/QtAV'>https://github.com/wang-bin/QtAV</a><br>"
                     "Home page: <a href='http://qtav.org'>http://qtav.org</a>");
    qtAVInfo.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&qtAVInfo);
    QLabel libAVInfo;
    libAVInfo.setFrameStyle(QFrame::Panel | QFrame::Sunken);
    libAVInfo.setText("<b>Libav 12.3</b><br>"
                      "the Libav project under the LGPLv2.1<br>"
                      "<a href='https://libav.org/download/'>https://libav.org/download/</a>");
    libAVInfo.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&libAVInfo);
    QLabel rangeSliderInfo;
    rangeSliderInfo.setFrameStyle(QFrame::Panel | QFrame::Sunken);
    rangeSliderInfo.setText("<b>Qt-RangeSlider</b><br>"
                      "Copyright (c) 2019 ThisIsClark (MIT)<br>"
                      "Modifications Copyright (c) 2020 Jason C. Fain<br>"
                      "<a href='https://github.com/ThisIsClark/Qt-RangeSlider'>https://github.com/ThisIsClark/Qt-RangeSlider</a>");
    rangeSliderInfo.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&rangeSliderInfo);
    QLabel boolinqInfo;
    boolinqInfo.setFrameStyle(QFrame::Panel | QFrame::Sunken);
    boolinqInfo.setText("<b>boolinq</b><br>"
                      "Copyright (C) 2019 by Anton Bukov (MIT)<br>"
                      "<a href='https://github.com/k06a/boolinq'>https://github.com/k06a/boolinq</a>");
    boolinqInfo.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&boolinqInfo);
    QLabel qtcompressInfo;
    qtcompressInfo.setFrameStyle(QFrame::Panel | QFrame::Sunken);
    qtcompressInfo.setText("<b>qtcompress</b><br>"
                      "Copyright (C) 2013 Digia Plc (LGPL)<br>"
                      "<a href='https://github.com/nezticle/qtcompress'>https://github.com/nezticle/qtcompress</a>");
    qtcompressInfo.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&qtcompressInfo);
    aboutWindow.setLayout(&layout);
    aboutWindow.exec();
}
//<a href="https://www.vecteezy.com/free-vector/media-player-icons">Media Player Icons Vectors by Vecteezy</a>

void MainWindow::on_actionDonate_triggered()
{
    QDesktopServices::openUrl(QUrl(QString::fromLatin1("https://www.patreon.com/Khrull")));
}

void MainWindow::on_actionSettings_triggered()
{
    _xSettings->initLive();
    _xSettings->show();
}

void MainWindow::on_actionThumbnail_triggered()
{
    SettingsHandler::setLibraryView(LibraryView::Thumb);
    changelibraryDisplayMode(LibraryView::Thumb);
}

void MainWindow::on_actionList_triggered()
{
    SettingsHandler::setLibraryView(LibraryView::List);
    changelibraryDisplayMode(LibraryView::List);
}

void MainWindow::changelibraryDisplayMode(LibraryView value)
{
    switch(value)
    {
        case LibraryView::List:
            libraryList->setResizeMode(QListView::Fixed);
            libraryList->setFlow(QListView::TopToBottom);
            libraryList->setViewMode(QListView::ListMode);
            libraryList->setTextElideMode(Qt::ElideRight);
            libraryList->setSpacing(0);
        break;
        case LibraryView::Thumb:
            libraryList->setResizeMode(QListView::Adjust);
            libraryList->setFlow(QListView::LeftToRight);
            libraryList->setViewMode(QListView::IconMode);
            libraryList->setTextElideMode(Qt::ElideMiddle);
            libraryList->setSpacing(2);
        break;
    }
    if(selectedPlaylistItems.length() > 0)
    {
        libraryList->setDragEnabled(true);
        libraryList->setDragDropMode(QAbstractItemView::DragDrop);
        libraryList->setDefaultDropAction(Qt::MoveAction);
        libraryList->setMovement(QListView::Movement::Snap);
    }
    updateThumbSizeUI(SettingsHandler::getThumbSize());
}

void MainWindow::updateThumbSizeUI(int size)
{
    switch(size)
    {
        case 75:
            action75_Size->setChecked(true);
            on_action75_triggered();
        break;
        case 100:
            action100_Size->setChecked(true);
            on_action100_triggered();
        break;
        case 125:
            action125_Size->setChecked(true);
            on_action125_triggered();
        break;
        case 150:
            action150_Size->setChecked(true);
            on_action150_triggered();
        break;
        case 175:
            action175_Size->setChecked(true);
            on_action175_triggered();
        break;
        case 200:
            action200_Size->setChecked(true);
            on_action200_triggered();
        break;
        default:
            actionCustom_Size->setChecked(true);
            setThumbSize(size);
        break;
    }
}

void MainWindow::on_action75_triggered()
{
    setThumbSize(75);
}

void MainWindow::on_action100_triggered()
{
    setThumbSize(100);
}

void MainWindow::on_action125_triggered()
{
    setThumbSize(125);
}

void MainWindow::on_action150_triggered()
{
    setThumbSize(150);
}

void MainWindow::on_action175_triggered()
{
    setThumbSize(175);
}

void MainWindow::on_action200_triggered()
{
    setThumbSize(200);
}

void MainWindow::on_actionCustom_triggered()
{
    bool ok;
    int size = QInputDialog::getInt(this, tr("Custom size"), "Size (Max:"+QString::number(SettingsHandler::getMaxThumbnailSize().height()) + ")",
                                         SettingsHandler::getThumbSize(), 1, SettingsHandler::getMaxThumbnailSize().height(), 50, &ok);
    if (ok && size > 0)
        setThumbSize(size);
}

void MainWindow::setThumbSize(int size)
{
    SettingsHandler::setThumbSize(size);
    resizeThumbs(size);

    videoHandler->setMinimumHeight(SettingsHandler::getThumbSize());
    videoHandler->setMinimumWidth(SettingsHandler::getThumbSize());
//    if(SettingsHandler::getLibraryView() == LibraryView::List)
//        libraryList->setViewMode(QListView::ListMode);
//    else
//        libraryList->setViewMode(QListView::IconMode);

}

void MainWindow::resizeThumbs(int size)
{
    QSize newSize = {size, size};
    for(int i = 0; i < libraryList->count(); i++)
    {
        ((LibraryListWidgetItem*)libraryList->item(i))->updateThumbSize(newSize);
    }
    libraryList->setIconSize(newSize);
}

void MainWindow::updateLibrarySortUI()
{
    updateLibrarySortUI(SettingsHandler::getLibrarySortMode());
}

void MainWindow::updateLibrarySortUI(LibrarySortMode mode)
{
    switch(mode)
    {
        case LibrarySortMode::NAME_ASC:
            actionNameAsc_Sort->setChecked(true);
            on_actionNameAsc_triggered();
        break;
        case LibrarySortMode::NAME_DESC:
            actionNameDesc_Sort->setChecked(true);
            on_actionNameDesc_triggered();
        break;
        case LibrarySortMode::CREATED_ASC:
            actionCreatedAsc_Sort->setChecked(true);
            on_actionCreatedAsc_triggered();
        break;
        case LibrarySortMode::CREATED_DESC:
            actionCreatedDesc_Sort->setChecked(true);
            on_actionCreatedDesc_triggered();
        break;
        case LibrarySortMode::RANDOM:
            actionRandom_Sort->setChecked(true);
            on_actionRandom_triggered();
        break;
        case LibrarySortMode::TYPE_ASC:
            actionTypeAsc_Sort->setChecked(true);
            on_actionTypeAsc_triggered();
        break;
        case LibrarySortMode::TYPE_DESC:
            actionTypeDesc_Sort->setChecked(true);
            on_actionTypeDesc_triggered();
        break;
    }

    libraryList->setCurrentRow(0);
}

void MainWindow::on_actionNameAsc_triggered()
{
    randomizeLibraryButton->hide();
    LibraryListWidgetItem::setSortMode(LibrarySortMode::NAME_ASC);
    SettingsHandler::setLibrarySortMode(LibrarySortMode::NAME_ASC);
    libraryList->sortItems();
}
void MainWindow::on_actionNameDesc_triggered()
{
    randomizeLibraryButton->hide();
    LibraryListWidgetItem::setSortMode(LibrarySortMode::NAME_DESC);
    SettingsHandler::setLibrarySortMode(LibrarySortMode::NAME_DESC);
    libraryList->sortItems();
}
void MainWindow::on_actionRandom_triggered()
{
    if(!loadingLibraryFuture.isRunning())
    {
        setLibraryLoading(true);
        LibraryListWidgetItem::setSortMode(LibrarySortMode::RANDOM);
        SettingsHandler::setLibrarySortMode(LibrarySortMode::RANDOM);

        loadingLibraryFuture = QtConcurrent::run([this]() {
            //Fisher and Yates algorithm
            int n = libraryList->count();

            QList<LibraryListWidgetItem*> arr, arr1;
            int index_arr[n];
            int index;

            for (int i = 0; i < n; i++)
                index_arr[i] = 0;

            for (int i = 0; i < n; i++)
            {
              do
              {
                 index = XMath::rand(0, n);
              }
              while (index_arr[index] != 0);
              index_arr[index] = 1;
              arr1.push_back(((LibraryListWidgetItem*)libraryList->item(index)));
            }
            while(libraryList->count()>0)
            {
              libraryList->takeItem(0);
            }
            foreach(auto item, arr1)
            {
                libraryList->addItem(item);
            }
            randomizeLibraryButton->show();
            setLibraryLoading(false);
        });
    }
}
void MainWindow::on_actionCreatedAsc_triggered()
{
    randomizeLibraryButton->hide();
    LibraryListWidgetItem::setSortMode(LibrarySortMode::CREATED_ASC);
    SettingsHandler::setLibrarySortMode(LibrarySortMode::CREATED_ASC);
    libraryList->sortItems();
}
void MainWindow::on_actionCreatedDesc_triggered()
{
    randomizeLibraryButton->hide();
    LibraryListWidgetItem::setSortMode(LibrarySortMode::CREATED_DESC);
    SettingsHandler::setLibrarySortMode(LibrarySortMode::CREATED_DESC);
    libraryList->sortItems();
}
void MainWindow::on_actionTypeAsc_triggered()
{
    randomizeLibraryButton->hide();
    LibraryListWidgetItem::setSortMode(LibrarySortMode::TYPE_ASC);
    SettingsHandler::setLibrarySortMode(LibrarySortMode::TYPE_ASC);
    libraryList->sortItems();
}
void MainWindow::on_actionTypeDesc_triggered()
{
    randomizeLibraryButton->hide();
    LibraryListWidgetItem::setSortMode(LibrarySortMode::TYPE_DESC);
    SettingsHandler::setLibrarySortMode(LibrarySortMode::TYPE_DESC);
    libraryList->sortItems();
}

void MainWindow::on_actionChange_theme_triggered()
{
    QFileInfo selectedThemeInfo(SettingsHandler::getSelectedTheme());
    QString selectedTheme = QFileDialog::getOpenFileName(this, "Choose XTP theme", selectedThemeInfo.absoluteDir().absolutePath(), "CSS Files (*.css)");
    if(!selectedTheme.isEmpty())
    {
        SettingsHandler::setSelectedTheme(selectedTheme);
        QFile file(selectedTheme);
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());
        setStyleSheet(styleSheet);
    }
}

void MainWindow::on_actionChange_current_deo_script_triggered()
{
    changeDeoFunscript();
}

void MainWindow::on_settingsButton_clicked()
{
    on_actionSettings_triggered();
}

void MainWindow::on_loopToggleButton_toggled(bool checked)
{
    if (checked)
    {
        connect(_playerControlsFrame, &PlayerControls::loopRangeChanged, this, &MainWindow::onLoopRange_valueChanged);
        videoHandler->setRepeat(-1);
        auto libraryListItemMetaData = SettingsHandler::getLibraryListItemMetaData(playingLibraryListItem->getLibraryListItem().path);
        if(libraryListItemMetaData.lastLoopStart > -1 && libraryListItemMetaData.lastLoopEnd > libraryListItemMetaData.lastLoopStart)
        {
            QTimer::singleShot(250, this, [this, libraryListItemMetaData]() {
                _playerControlsFrame->setSeekSliderLowerValue(libraryListItemMetaData.lastLoopStart);
                _playerControlsFrame->setSeekSliderUpperValue(libraryListItemMetaData.lastLoopEnd);
            });
//            qint64 sliderToVideoPosition = XMath::mapRange(libraryListItemMetaData.lastLoopStart,  (qint64)0, (qint64)100, (qint64)0, videoHandler->duration());
//            videoHandler->setPosition(sliderToVideoPosition +100);
        }
        else
        {
            qint64 videoToSliderPosition = XMath::mapRange(videoHandler->position(),  (qint64)0, videoHandler->duration(), (qint64)0, (qint64)100);
            _playerControlsFrame->setSeekSliderLowerValue(videoToSliderPosition);
        }
        _playerControlsFrame->setSeekSliderMinimumRange(1);
    }
    else
    {
        on_media_positionChanged(videoHandler->position());
        disconnect(_playerControlsFrame, &PlayerControls::loopRangeChanged, this, &MainWindow::onLoopRange_valueChanged);
        qint64 position = videoHandler->position();
        videoHandler->setRepeat();
        videoHandler->setPosition(position);
        _playerControlsFrame->setSeekSliderMinimumRange(0);
    }
}

QString MainWindow::getPlaylistName(bool newPlaylist)
{
    bool ok;
    QString playlistName = nullptr;
    if(newPlaylist)
        playlistName = PlaylistDialog::getNewPlaylist(this, &ok);
    else
    {
        QListWidgetItem* selectedItem = libraryList->selectedItems().first();
        LibraryListItem selectedFileListItem = ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();
        playlistName = PlaylistDialog::renamePlaylist(this, selectedFileListItem.nameNoExtension, &ok);
    }

    if(newPlaylist && ok)
    {
        auto items = libraryList->findItems(playlistName, Qt::MatchFixedString);
        if(items.empty())
        {
            SettingsHandler::addNewPlaylist(playlistName);
            setupPlaylistItem(playlistName);
        }
        else
        {
            LogHandler::Dialog((tr("Playlist '") + playlistName + tr("' already exists.\nPlease choose another name.")), XLogLevel::Critical);
        }
    }
    return ok ? playlistName : nullptr;
}

LibraryListItem MainWindow::setupPlaylistItem(QString playlistName)
{
    LibraryListItem item
    {
        LibraryListItemType::PlaylistInternal,
        nullptr, // path
        nullptr, // name
        playlistName, //nameNoExtension
        nullptr, // script
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        QDate::currentDate(),
        0
    };
    LibraryListWidgetItem* qListWidgetItem = new LibraryListWidgetItem(item);
    libraryList->insertItem(0, qListWidgetItem);
    cachedLibraryItems.push_front(new LibraryListWidgetItem(item));
    return item;
}

void MainWindow::addSelectedLibraryItemToPlaylist(QString playlistName)
{
    LibraryListWidgetItem* qListWidgetItem = (LibraryListWidgetItem*)(libraryList->findItems(playlistName, Qt::MatchExactly).first());
    QListWidgetItem* selectedItem = libraryList->selectedItems().first();
    LibraryListItem selectedFileListItem = ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();
    SettingsHandler::addToPlaylist(playlistName, selectedFileListItem);
    qListWidgetItem->updateToolTip(selectedFileListItem);
}

void MainWindow::loadPlaylistIntoLibrary(QString playlistName)
{
    if(!thumbProcessIsRunning)
    {
        selectedPlaylistName = playlistName;
        libraryList->clear();
        auto playlists = SettingsHandler::getPlaylists();
        auto playlist = playlists.value(playlistName);
        foreach(auto item, playlist)
        {
            LibraryListWidgetItem* qListWidgetItem = new LibraryListWidgetItem(item);
            libraryList->addItem(qListWidgetItem);
            selectedPlaylistItems.push_back((LibraryListWidgetItem*)qListWidgetItem->clone());
        }
        backLibraryButton->show();
        editPlaylistButton->show();
        librarySortGroup->setEnabled(false);
    }
    else
        LogHandler::Dialog(tr("Please wait for thumbnails to fully load!"), XLogLevel::Warning);
}

void MainWindow::backToMainLibrary()
{
    librarySortGroup->setEnabled(true);
    selectedPlaylistName = nullptr;
    selectedPlaylistItems.clear();
    if(cachedLibraryItems.length() == 0)
        on_load_library(SettingsHandler::getSelectedLibrary());
    else
    {
        libraryList->clear();
        foreach(auto item, cachedLibraryItems)
        {
            auto name = item->text();
            libraryList->addItem(item->clone());
        }
    }
    backLibraryButton->hide();
    editPlaylistButton->hide();
    savePlaylistButton->hide();
    cancelEditPlaylistButton->hide();
    libraryList->setDragEnabled(false);
    disconnect(libraryList, &QListWidget::itemChanged, 0, 0);

    changelibraryDisplayMode(SettingsHandler::getLibraryView());
    resizeThumbs(SettingsHandler::getThumbSize());
    updateLibrarySortUI();
}

void MainWindow::savePlaylist()
{
    QScroller::grabGesture(libraryList->viewport(), QScroller::LeftMouseButtonGesture);
    QList<LibraryListItem> libraryItems;
    for(int i=0;i<libraryList->count();i++)
    {
        LibraryListItem libraryListItem = ((LibraryListWidgetItem*)libraryList->item(i))->getLibraryListItem();
        if(!libraryListItem.nameNoExtension.isEmpty())
            libraryItems.push_back(libraryListItem);
    }
    SettingsHandler::updatePlaylist(selectedPlaylistName, libraryItems);
    savePlaylistButton->hide();
    editPlaylistButton->show();
    cancelEditPlaylistButton->hide();
    _editPlaylistMode = false;
    changelibraryDisplayMode(SettingsHandler::getLibraryView());
    resizeThumbs(SettingsHandler::getThumbSize());
}
void MainWindow::editPlaylist()
{

    QScroller::grabGesture(libraryList->viewport(), QScroller::MiddleMouseButtonGesture);
    _editPlaylistMode = true;
    savePlaylistButton->show();
    cancelEditPlaylistButton->show();
    editPlaylistButton->hide();
    changelibraryDisplayMode(LibraryView::List);
    if(SettingsHandler::getThumbSize() > 75)
        resizeThumbs(75);
    libraryList->setDragEnabled(true);
    libraryList->setDragDropMode(QAbstractItemView::InternalMove);
    libraryList->setDefaultDropAction(Qt::MoveAction);
    libraryList->setMovement(QListWidget::Movement::Snap);
    libraryList->setDragDropOverwriteMode(false);
    libraryList->setDropIndicatorShown(true);

}
void MainWindow::cancelEditPlaylist()
{
    QScroller::grabGesture(libraryList->viewport(), QScroller::LeftMouseButtonGesture);
    savePlaylistButton->hide();
    cancelEditPlaylistButton->hide();
    editPlaylistButton->show();
    loadPlaylistIntoLibrary(selectedPlaylistName);
    libraryList->setDragEnabled(false);
    changelibraryDisplayMode(SettingsHandler::getLibraryView());
    if(SettingsHandler::getThumbSize() > 75)
        resizeThumbs(SettingsHandler::getThumbSize());
}
void MainWindow::removeFromPlaylist()
{
    QListWidgetItem* selectedItem = libraryList->selectedItems().first();
    delete selectedItem;
    savePlaylistButton->show();
    editPlaylistButton->hide();
    cancelEditPlaylistButton->show();
}
void MainWindow::renamePlaylist()
{
    QString renamedPlaylistName = getPlaylistName(false);
    if(renamedPlaylistName != nullptr)
    {
        auto items = libraryList->findItems(renamedPlaylistName, Qt::MatchFixedString);
        LibraryListItem playlist = ((LibraryListWidgetItem*)libraryList->selectedItems().first())->getLibraryListItem();
        if(items.empty())
        {
            auto playlists = SettingsHandler::getPlaylists();
            auto storedPlaylist = playlists.value(playlist.nameNoExtension);
            deleteSelectedPlaylist();
            SettingsHandler::updatePlaylist(renamedPlaylistName, storedPlaylist);
            setupPlaylistItem(renamedPlaylistName);
        }
        else if(playlist.nameNoExtension != renamedPlaylistName)
        {
            LogHandler::Dialog((tr("Playlist '") + renamedPlaylistName + tr("' already exists.\nPlease choose another name.")), XLogLevel::Critical);
        }
    }

}
void MainWindow::deleteSelectedPlaylist()
{
    LibraryListWidgetItem* selectedItem = (LibraryListWidgetItem*)libraryList->selectedItems().first();
    LibraryListItem selectedFileListItem = selectedItem->getLibraryListItem();
    SettingsHandler::deletePlaylist(selectedFileListItem.nameNoExtension);
    libraryList->removeItemWidget(selectedItem);
    LibraryListWidgetItem* deleteCache = boolinq::from(cachedLibraryItems).firstOrDefault([selectedItem](const LibraryListWidgetItem* x) { return x->text() == selectedItem->text(); });
    cachedLibraryItems.removeOne(deleteCache);
    delete deleteCache;
    delete selectedItem;
}


LibraryListItem MainWindow::getSelectedLibraryListItem()
{
    QListWidgetItem* selectedItem = libraryList->selectedItems().first();
    return ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();
}

void MainWindow::on_actionReload_library_triggered()
{
    if(!loadingLibraryFuture.isRunning())
    {
        setLibraryLoading(true);
        loadingLibraryFuture = QtConcurrent::run([this]() {
            on_load_library(SettingsHandler::getSelectedLibrary());
            setLibraryLoading(false);
        });
    }
}

void MainWindow::showInGraphicalShell(QString path)
{
    const QFileInfo fileInfo(path);
    // Mac, Window, linux support folder or file.
#if defined(Q_OS_WIN)
    QStringList args;
    if (!fileInfo.isDir())
        args << "/select,";
    args << QDir::toNativeSeparators(path);
    if (QProcess::startDetached("explorer", args))
        return;
#elif defined(Q_OS_MAC)
    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e")
               << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                                     .arg(fileInfo.canonicalFilePath());
    QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
    scriptArgs.clear();
    scriptArgs << QLatin1String("-e")
               << QLatin1String("tell application \"Finder\" to activate");
    QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
#elif defined(Q_OS_LINUX)
    // we cannot select a file here, because no file browser really supports it...
    const QString folder = fileInfo.isDir() ? fileInfo.absoluteFilePath() : fileInfo.filePath();
     QDesktopServices::openUrl(fileInfo.path());
//    QProcess browserProc;
//    bool success = browserProc.startDetached("gtk-launch \"$(xdg-mime query default inode/directory)\"", QStringList() << folder );
//    const QString error = QString::fromLocal8Bit(browserProc.readAllStandardError());
//    success = success && error.isEmpty();
//    if (!success)
//        LogHandler::Dialog(error, XLogLevel::Critical);
#endif
}

void MainWindow::onSetMoneyShot(LibraryListItem libraryListItem, qint64 currentPosition)
{
    auto libraryListItemMetaData = SettingsHandler::getLibraryListItemMetaData(libraryListItem.path);
    libraryListItemMetaData.moneyShotMillis = currentPosition;
    SettingsHandler::updateLibraryListItemMetaData(libraryListItemMetaData);
}
void MainWindow::onAddBookmark(LibraryListItem libraryListItem, QString name, qint64 currentPosition)
{
    auto libraryListItemMetaData = SettingsHandler::getLibraryListItemMetaData(libraryListItem.path);
    libraryListItemMetaData.bookmarks.append({name, currentPosition});
    SettingsHandler::updateLibraryListItemMetaData(libraryListItemMetaData);
}
void MainWindow::skipToMoneyShot()
{
    if(_playerControlsFrame->getAutoLoop())
        _playerControlsFrame->SetLoop(false);
    LibraryListItem selectedLibraryListItem = playingLibraryListItem->getLibraryListItem();
    auto libraryListItemMetaData = SettingsHandler::getLibraryListItemMetaData(selectedLibraryListItem.path);
    if (libraryListItemMetaData.moneyShotMillis > -1 && libraryListItemMetaData.moneyShotMillis < videoHandler->duration())
    {
        videoHandler->setPosition(libraryListItemMetaData.moneyShotMillis);
    }
    else
    {
        qint64 last30PercentOfduration = videoHandler->duration() - videoHandler->duration() * .1;
        videoHandler->setPosition(last30PercentOfduration);
    }
}
