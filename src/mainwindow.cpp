#include "mainwindow.h"
#include "ui_mainwindow.h"
MainWindow::MainWindow(QStringList arguments, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    QCoreApplication::setOrganizationName("cUrbSide prOd");
    QCoreApplication::setApplicationName("XTPlayer");

    QPixmap pixmap("://images/XTP_Splash.png");
    loadingSplash = new QSplashScreen(pixmap);
    loadingSplash->setStyleSheet("color: white");
    loadingSplash->show();

    ui->setupUi(this);
    loadingSplash->showMessage("v"+SettingsHandler::XTPVersion + "\nLoading Settings...", Qt::AlignBottom, Qt::white);
    SettingsHandler::Load();
    _xSettings = new SettingsDialog(this);
    _dlnaScriptLinksDialog = new DLNAScriptLinks(this);
    tcodeHandler = new TCodeHandler();
    if(_xSettings->HasLaunchPass()) {
        int tries = 1;
        while(_isPasswordIncorrect != PasswordResponse::CANCEL && _isPasswordIncorrect == PasswordResponse::INCORRECT)
        {
            _isPasswordIncorrect = _xSettings->GetLaunchPass();
            if(_isPasswordIncorrect == PasswordResponse::CANCEL)
            {
                QTimer::singleShot(0, this, SLOT(onPasswordIncorrect()));
                return;
            }
            else if(_isPasswordIncorrect == PasswordResponse::INCORRECT)
            {
                switch(tries) {
                    case 1:
                        LogHandler::Dialog(this, "Wrong!", XLogLevel::Critical);
                    break;
                    case 2:
                        LogHandler::Dialog(this, "Nope!", XLogLevel::Critical);
                    break;
                    case 3:
                        LogHandler::Dialog(this, "K thx byyye!", XLogLevel::Critical);
                    break;
                }

                if( tries >= 3)
                {
                    QTimer::singleShot(0, this, SLOT(onPasswordIncorrect()));
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
                LogHandler::setUserDebug(true);
            else if(arg.toLower().startsWith("-reset"))
                SettingsHandler::Default();
        }
    }

    loadingSplash->showMessage("v"+SettingsHandler::XTPVersion + "\nLoading UI...", Qt::AlignBottom, Qt::white);


    textToSpeech = new QTextToSpeech(this);
    auto availableVoices = textToSpeech->availableVoices();

    const QVoice voice = boolinq::from(availableVoices).firstOrDefault([](const QVoice &x) { return x.gender() == QVoice::Female; });
    textToSpeech->setVoice(voice);

    deoConnectionStatusLabel = new QLabel(this);
    deoRetryConnectionButton = new QPushButton(this);
    deoRetryConnectionButton->hide();
    deoRetryConnectionButton->setProperty("cssClass", "retryButton");
    deoRetryConnectionButton->setText("DeoVR Retry");
    ui->statusbar->addPermanentWidget(deoConnectionStatusLabel);
    ui->statusbar->addPermanentWidget(deoRetryConnectionButton);

    vrConnectionStatusLabel = new QLabel(this);
    vrRetryConnectionButton = new QPushButton(this);
    vrRetryConnectionButton->hide();
    vrRetryConnectionButton->setProperty("cssClass", "retryButton");
    vrRetryConnectionButton->setText("Whirligig Retry");
    ui->statusbar->addPermanentWidget(vrConnectionStatusLabel);
    ui->statusbar->addPermanentWidget(vrRetryConnectionButton);

    xtpWebStatusLabel = new QLabel(this);
    ui->statusbar->addPermanentWidget(xtpWebStatusLabel);

    connectionStatusLabel = new QLabel(this);
    connectionStatusLabel->setObjectName("connectionStatusLabel");
    retryConnectionButton = new QPushButton(this);
    retryConnectionButton->hide();
    retryConnectionButton->setProperty("cssClass", "retryButton");
    retryConnectionButton->setText("TCode Retry");
    ui->statusbar->addPermanentWidget(connectionStatusLabel);
    ui->statusbar->addPermanentWidget(retryConnectionButton);


    gamepadConnectionStatusLabel = new QLabel(this);
    ui->statusbar->addPermanentWidget(gamepadConnectionStatusLabel);

    _mediaFrame = new QFrame(this);
    _mediaGrid = new QGridLayout(_mediaFrame);
    _mediaFrame->setLayout(_mediaGrid);
    _mediaFrame->setContentsMargins(0,0,0,0);

    videoHandler = new VideoHandler(this);
    _mediaGrid->addWidget(videoHandler, 0, 0, 3, 5);
    _mediaGrid->setMargin(0);
    _mediaGrid->setContentsMargins(0,0,0,0);


    _controlsHomePlaceHolderFrame = new QFrame(this);
    _controlsHomePlaceHolderGrid = new QGridLayout(_controlsHomePlaceHolderFrame);
    _controlsHomePlaceHolderFrame->setLayout(_controlsHomePlaceHolderGrid);

    _playerControlsFrame = new PlayerControls(this);
    _controlsHomePlaceHolderGrid->addWidget(_playerControlsFrame, 0, 0);

    ui->mediaAndControlsGrid->addWidget(_mediaFrame, 0, 0, 19, 3);
    ui->mediaAndControlsGrid->addWidget(_controlsHomePlaceHolderFrame, 20, 0, 1, 3);

    _syncHandler = new SyncHandler(_xSettings, tcodeHandler, videoHandler, this);
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
    libraryWindow->hide();

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
    libraryLoadingInfoLabel = new QLabel(this);
    //libraryLoadingLabel->setStyleSheet("* {background-color: rgba(128,128,128, 0.5)}");
    libraryLoadingLabel->setProperty("cssClass", "libraryLoadingSpinner");
    libraryLoadingLabel->setAlignment(Qt::AlignCenter);
    libraryLoadingInfoLabel->setProperty("cssClass", "libraryLoadingSpinnerText");
    libraryLoadingInfoLabel->setAlignment(Qt::AlignCenter);
    ui->libraryGrid->addWidget(libraryLoadingLabel, 0, 0, 21, 12);
    ui->libraryGrid->addWidget(libraryLoadingInfoLabel, 0, 0, 21, 12);
    libraryLoadingLabel->hide();
    libraryLoadingInfoLabel->hide();

    thumbCaptureTime = 35000;
    libraryViewGroup = new QActionGroup(this);
    libraryViewGroup->addAction(ui->actionList);
    libraryViewGroup->addAction(ui->actionThumbnail);

    _videoPreviewWidget = new XVideoPreviewWidget(this);
    _videoPreviewWidget->hide();

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
    }
    else
    {
        ui->actionThumbnail->setChecked(true);
    }

    auto splitterSizes = SettingsHandler::getMainWindowSplitterPos();
    if (splitterSizes.count() > 0)
        ui->mainFrameSplitter->setSizes(splitterSizes);

    connect(&SettingsHandler::instance(), &SettingsHandler::messageSend, this, &MainWindow::on_settingsMessageRecieve);

    connect(ui->mainFrameSplitter, &QSplitter::splitterMoved, this, &MainWindow::on_mainwindow_splitterMove);
    connect(backLibraryButton, &QPushButton::clicked, this, &MainWindow::backToMainLibrary);
    connect(randomizeLibraryButton, &QPushButton::clicked, this, &MainWindow::on_actionRandom_triggered);
    connect(windowedLibraryButton, &QPushButton::clicked, this, &MainWindow::onLibraryWindowed_Clicked);
    connect(savePlaylistButton, &QPushButton::clicked, this, &MainWindow::savePlaylist);
    connect(editPlaylistButton, &QPushButton::clicked, this, &MainWindow::editPlaylist);
    connect(cancelEditPlaylistButton, &QPushButton::clicked, this, &MainWindow::cancelEditPlaylist);

    connect(libraryWindow, &LibraryWindow::close, this, &MainWindow::onLibraryWindowed_Closed);

    connect(_xSettings, &SettingsDialog::deoDeviceConnectionChange, this, &MainWindow::on_vr_device_connectionChanged);
    connect(_xSettings, &SettingsDialog::deoDeviceError, this, &MainWindow::on_vr_device_error);
    connect(_xSettings, &SettingsDialog::whirligigDeviceConnectionChange, this, &MainWindow::on_vr_device_connectionChanged);
    connect(_xSettings, &SettingsDialog::whirligigDeviceError, this, &MainWindow::on_vr_device_error);
    connect(_xSettings, &SettingsDialog::xtpWebDeviceConnectionChange, this, &MainWindow::on_xtpWeb_device_connectionChanged);
    connect(_xSettings, &SettingsDialog::xtpWebDeviceError, this, &MainWindow::on_xtpWeb_device_error);

    connect(_xSettings, &SettingsDialog::deviceConnectionChange, this, &MainWindow::on_device_connectionChanged);
    connect(_xSettings, &SettingsDialog::deviceError, this, &MainWindow::on_device_error);
    connect(_xSettings, &SettingsDialog::TCodeHomeClicked, this, &MainWindow::deviceHome);
    connect(_xSettings->getDeoHandler(), &DeoHandler::messageRecieved, this, &MainWindow::onVRMessageRecieved);
    connect(_xSettings->getWhirligigHandler(), &WhirligigHandler::messageRecieved, this, &MainWindow::onVRMessageRecieved);
    connect(_xSettings->getXTPWebHandler(), &XTPWebHandler::messageRecieved, this, &MainWindow::onVRMessageRecieved);
    connect(_xSettings, &SettingsDialog::gamepadConnectionChange, this, &MainWindow::on_gamepad_connectionChanged);
    connect(_xSettings->getGamepadHandler(), &GamepadHandler::emitTCode, this, &MainWindow::on_gamepad_sendTCode);
    connect(_xSettings->getGamepadHandler(), &GamepadHandler::emitAction, this, &MainWindow::on_gamepad_sendAction);
    connect(_xSettings, &SettingsDialog::onOpenWelcomeDialog, this, &MainWindow::openWelcomeDialog);
    _mediaLibraryHandler = new MediaLibraryHandler(this);
    _xSettings->init(videoHandler, _mediaLibraryHandler);

    connect(this, &MainWindow::libraryIconResized, this, &MainWindow::libraryListSetIconSize);

    connect(ui->actionReload_library, &QAction::triggered, _mediaLibraryHandler, &MediaLibraryHandler::loadLibraryAsync);
    connect(_mediaLibraryHandler, &MediaLibraryHandler::libraryNotFound, this, &MainWindow::onLibraryNotFound);
    connect(_mediaLibraryHandler, &MediaLibraryHandler::libraryLoaded, this, &MainWindow::onSetLibraryLoaded);
    connect(_mediaLibraryHandler, &MediaLibraryHandler::libraryLoadingStatus, this, &MainWindow::onLibraryLoadingStatusChange);
    connect(_mediaLibraryHandler, &MediaLibraryHandler::libraryLoading, this, &MainWindow::onSetLibraryLoading);
    connect(_mediaLibraryHandler, &MediaLibraryHandler::libraryItemFound, this, &MainWindow::onLibraryItemFound);
    connect(_mediaLibraryHandler, &MediaLibraryHandler::playListItem, this, &MainWindow::onSetupPlaylistItem);
    connect(_mediaLibraryHandler, &MediaLibraryHandler::prepareLibraryLoad, this, &MainWindow::onPrepareLibraryLoad);
    connect(_mediaLibraryHandler, &MediaLibraryHandler::saveThumbError, this, &MainWindow::onSaveThumbError);
    connect(_mediaLibraryHandler, &MediaLibraryHandler::saveNewThumb, this, &MainWindow::onSaveNewThumb);
    connect(_mediaLibraryHandler, &MediaLibraryHandler::saveNewThumbLoading, this, &MainWindow::onSaveNewThumbLoading);


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

    connect(_syncHandler, &SyncHandler::funscriptPositionChanged, this, &MainWindow::on_media_positionChanged, Qt::QueuedConnection);
    connect(_syncHandler, &SyncHandler::funscriptStatusChanged, this, &MainWindow::on_media_statusChanged, Qt::QueuedConnection);
    connect(_syncHandler, &SyncHandler::funscriptStarted, this, &MainWindow::on_standaloneFunscript_start, Qt::QueuedConnection);
    connect(_syncHandler, &SyncHandler::funscriptStopped, this, &MainWindow::on_standaloneFunscript_stop, Qt::QueuedConnection);

    connect(videoHandler, &VideoHandler::positionChanged, this, &MainWindow::on_media_positionChanged, Qt::QueuedConnection);
    connect(videoHandler, &VideoHandler::mediaStatusChanged, this, &MainWindow::on_media_statusChanged, Qt::QueuedConnection);
    connect(videoHandler, &VideoHandler::started, this, &MainWindow::on_media_start, Qt::QueuedConnection);
    connect(videoHandler, &VideoHandler::stopped, this, &MainWindow::on_media_stop, Qt::QueuedConnection);
    connect(videoHandler, &VideoHandler::togglePaused, this, &MainWindow::on_togglePaused);
    connect(videoHandler, &VideoHandler::doubleClicked, this, &MainWindow::media_double_click_event);
    connect(videoHandler, &VideoHandler::singleClicked, this, &MainWindow::media_single_click_event);
    connect(videoHandler, &VideoHandler::keyPressed, this, &MainWindow::on_key_press);

    connect(_playerControlsFrame, &PlayerControls::seekSliderMoved, this, &MainWindow::on_seekSlider_sliderMoved);
    connect(_playerControlsFrame, &PlayerControls::seekSliderHover, this, &MainWindow::on_seekslider_hover );
    connect(_playerControlsFrame, &PlayerControls::seekSliderLeave, this, &MainWindow::on_seekslider_leave );
    connect(_playerControlsFrame, &PlayerControls::volumeChanged, this, &MainWindow::on_VolumeSlider_valueChanged);
    connect(_playerControlsFrame, &PlayerControls::loopButtonToggled, this, &MainWindow::on_loopToggleButton_toggled);
    connect(_playerControlsFrame, &PlayerControls::muteChanged, this, &MainWindow::on_MuteBtn_toggled);
    connect(_playerControlsFrame, &PlayerControls::fullscreenToggled, this, &MainWindow::on_fullScreenBtn_clicked);
    connect(_playerControlsFrame, &PlayerControls::settingsClicked, this, &MainWindow::on_settingsButton_clicked);
    connect(_playerControlsFrame, &PlayerControls::playClicked, this, &MainWindow::on_PlayBtn_clicked);
    connect(_playerControlsFrame, &PlayerControls::stopClicked, this, &MainWindow::stopMedia);
    connect(_playerControlsFrame, &PlayerControls::skipForward, this, &MainWindow::on_skipForwardButton_clicked);
    connect(_playerControlsFrame, &PlayerControls::skipToMoneyShot, this, &MainWindow::skipToMoneyShot);
    connect(_playerControlsFrame, &PlayerControls::skipActionBegin, this, &MainWindow::skipToActionBegin);
    connect(_playerControlsFrame, &PlayerControls::skipBack, this, &MainWindow::on_skipBackButton_clicked);

    connect(this, &MainWindow::keyPressed, this, &MainWindow::on_key_press);
    connect(this, &MainWindow::change, this, &MainWindow::on_mainwindow_change);
    connect(this, &MainWindow::playVideo, this, &MainWindow::on_playVideo);
    connect(this, &MainWindow::stopAndPlayVideo, this, &MainWindow::stopAndPlayMedia);
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
    loadTheme(SettingsHandler::getSelectedTheme());

    setFocus();
    _defaultAppSize = this->size();
    _appSize = _defaultAppSize;
    _appPos = this->pos();

    changeLibraryDisplayMode(SettingsHandler::getLibraryView());
    loadingSplash->showMessage("v"+SettingsHandler::XTPVersion + "\nLoading Library...", Qt::AlignBottom, Qt::white);
    _mediaLibraryHandler->loadLibraryAsync();

//    QScreen *screen = this->screen();
//    QSize screenSize = screen->size();
//    auto minHeight = round(screenSize.height() * .06f);
//    _playerControlsFrame->setMinimumHeight(minHeight);
//    _controlsHomePlaceHolderFrame->setMinimumHeight(minHeight);
//    _playerControlsFrame->setMaximumHeight(minHeight);
//    _controlsHomePlaceHolderFrame->setMaximumHeight(minHeight);

    loadingSplash->showMessage("v"+SettingsHandler::XTPVersion + "\nStarting Application...", Qt::AlignBottom, Qt::white);
    loadingSplash->finish(this);
    if(!SettingsHandler::getHideWelcomeScreen())
    {
        openWelcomeDialog();
    }
}
MainWindow::~MainWindow()
{

}

void MainWindow::on_settingsMessageRecieve(QString message, XLogLevel logLevel) {
    LogHandler::Dialog(this, message, logLevel);
}
void MainWindow::onPasswordIncorrect()
{
    if(_isPasswordIncorrect)
        QApplication::quit();
}
void MainWindow::dispose()
{
    if(playingLibraryListItem != nullptr)
        updateMetaData(playingLibraryListItem->getLibraryListItem());
    SettingsHandler::Save();

    if (videoHandler->isPlaying())
    {
        videoHandler->stop();
    }
    _syncHandler->stopAll();
    _xSettings->dispose();
    //qDeleteAll(funscriptHandlers);
    delete _mediaLibraryHandler;
    delete tcodeHandler;
    delete videoHandler;
    delete connectionStatusLabel;
    delete retryConnectionButton;
    delete _videoPreviewWidget;
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
         case Qt::Key_K:
             mediaAction(actions.SkipToAction);
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
//        else if(SettingsHandler::getDeoEnabled() && _xSettings->getDeoHandler()->isConnected())
//        {
//            //_xSettings->getDeoHandler()->togglePause();
//        }
        else if(_syncHandler->isPlayingStandAlone())
        {
            _syncHandler->togglePause();
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
        if (videoHandler->isPaused() || videoHandler->isPlaying() || _syncHandler->isPlayingStandAlone())
            stopMedia();
    }
     else if(action == actions.Next)
    {
        if (videoHandler->isPaused() || videoHandler->isPlaying() || _syncHandler->isPlayingStandAlone())
            skipForward();
    }
    else if(action == actions.Back)
    {
        if (videoHandler->isPaused() || videoHandler->isPlaying() || _syncHandler->isPlayingStandAlone())
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
        if (videoHandler->isPlaying() || _syncHandler->isPlayingStandAlone())
            rewind();
    }
    else if(action == actions.FastForward )
    {
        if (videoHandler->isPlaying() || _syncHandler->isPlayingStandAlone())
            fastForward();
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
        int newLiveRange = SettingsHandler::getLiveXRangeMin() - SettingsHandler::getXRangeStep();
        int axisMin = SettingsHandler::getAxis(TCodeChannelLookup::Stroke()).Min;
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
        int newLiveRange = SettingsHandler::getLiveXRangeMax() + SettingsHandler::getXRangeStep();
        int axisMax = SettingsHandler::getAxis(TCodeChannelLookup::Stroke()).Max;
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
        int xRangeMax = SettingsHandler::getLiveXRangeMax();
        int xRangeMin = SettingsHandler::getLiveXRangeMin();
        int xRangeStep = SettingsHandler::getXRangeStep();
        int newLiveMaxRange = xRangeMax + xRangeStep;
        int axisMax = SettingsHandler::getAxis(TCodeChannelLookup::Stroke()).Max;
        bool atMax = false;
        if(newLiveMaxRange > axisMax)
        {
            atMax = true;
            newLiveMaxRange = axisMax;
        }
        SettingsHandler::setLiveXRangeMax(newLiveMaxRange);

        int newLiveMinRange = xRangeMin - xRangeStep;
        int axisMin = SettingsHandler::getAxis(TCodeChannelLookup::Stroke()).Min;
        bool atMin = false;
        if(newLiveMinRange < axisMin)
        {
            atMin = true;
            newLiveMinRange = axisMin;
        }
        SettingsHandler::setLiveXRangeMin(newLiveMinRange);

        if(!SettingsHandler::getDisableSpeechToText())
        {
            if (atMin && atMax)
                textToSpeech->say("Increase X at limit");
            else if (atMax)
                textToSpeech->say("Increase X, max at limit, min"+ QString::number(newLiveMinRange));
            else if (atMin)
                textToSpeech->say("Increase X, max "+ QString::number(newLiveMaxRange) + ", min at limit");
            else
                textToSpeech->say("Increase X, max "+ QString::number(newLiveMaxRange) + ", min "+ QString::number(newLiveMinRange));
        }

    }
    else if (action == actions.DecreaseXRange)
    {
        int xRangeMax = SettingsHandler::getLiveXRangeMax();
        int xRangeMin = SettingsHandler::getLiveXRangeMin();
        int xRangeStep = SettingsHandler::getXRangeStep();
        int newLiveMaxRange = xRangeMax - xRangeStep;
        bool maxLessThanMin = false;
        if(newLiveMaxRange < xRangeMin)
        {
            maxLessThanMin = true;
            newLiveMaxRange = xRangeMin + 1;
        }
        SettingsHandler::setLiveXRangeMax(newLiveMaxRange);

        int newLiveMinRange = xRangeMin + xRangeStep;
        bool minGreaterThanMax = false;
        if(newLiveMinRange > xRangeMax)
        {
            minGreaterThanMax = true;
            newLiveMinRange = xRangeMax - 1;
        }
        SettingsHandler::setLiveXRangeMin(newLiveMinRange);
        if(!SettingsHandler::getDisableSpeechToText())
        {
            if (maxLessThanMin && minGreaterThanMax)
                textToSpeech->say("Decrease X at limit");
            else if (maxLessThanMin)
                textToSpeech->say("Decrease X, max at limit, min "+ QString::number(newLiveMinRange));
            else if (minGreaterThanMax)
                textToSpeech->say("Decrease X, max "+ QString::number(newLiveMaxRange) + ", min at limit");
            else
                textToSpeech->say("Decrease X, max "+ QString::number(newLiveMaxRange) + ", min "+ QString::number(newLiveMinRange));
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
        if(!SettingsHandler::getDisableSpeechToText())
            textToSpeech->say(multiplier ? "Disable multiplier" : "Enable multiplier");
        SettingsHandler::setLiveMultiplierEnabled(!multiplier);
    }
    else if (action == actions.ToggleFunscriptInvert)
    {
        bool inverted = FunscriptHandler::getInverted();
        if(!SettingsHandler::getDisableSpeechToText())
            textToSpeech->say(inverted ? "Funscript normal" : "Funscript inverted");
        FunscriptHandler::setInverted(!inverted);
    }
     else if(action == actions.TogglePauseAllDeviceActions)
     {
         bool paused = SettingsHandler::getLiveActionPaused();
         if(!SettingsHandler::getDisableSpeechToText())
            textToSpeech->say(paused ? "Resume action" : "Pause action");
         SettingsHandler::setLiveActionPaused(!paused);
     }
     else if (action == actions.SkipToMoneyShot)
     {
        if (videoHandler->isPaused() || videoHandler->isPlaying() || _syncHandler->isPlayingStandAlone() || _syncHandler->isPlayingVR())
            skipToMoneyShot();
     }
    else if (action == actions.SkipToAction)
    {
       if (videoHandler->isPaused() || videoHandler->isPlaying() || _syncHandler->isPlayingStandAlone())
           skipToActionBegin();
    }
    else if (action == actions.IncreaseFunscriptModifier || action == actions.DecreaseFunscriptModifier)
    {
        bool increase = action == actions.IncreaseFunscriptModifier;
        QString verb = increase ? "Increase" : "Decrease";
        int modifier = FunscriptHandler::getModifier();
        int modedModifier = increase ? modifier + SettingsHandler::getFunscriptModifierStep() : modifier - SettingsHandler::getFunscriptModifierStep();
        if(modedModifier > 0)
        {
            FunscriptHandler::setModifier(modedModifier);
            if(!SettingsHandler::getDisableSpeechToText())
                textToSpeech->say(verb + " funscript modifier to "+ QString::number(modedModifier) + "percent");
        }
        else
            if(!SettingsHandler::getDisableSpeechToText())
                textToSpeech->say("Funscript modifier at minimum "+ QString::number(modedModifier) + "percent");
    }
    else if (action == actions.IncreaseOffset || action == actions.DecreaseOffset)
    {
        bool increase = action == actions.IncreaseOffset;
        QString verb = increase ? "Increase" : "Decrease";
        if (_syncHandler->isPlaying())
        {
           QString path = playingLibraryListItem->getLibraryListItem().path;
           if(!path.isEmpty())
           {
               auto libraryListItemMetaData = SettingsHandler::getLibraryListItemMetaData(path);
               int newOffset = increase ? libraryListItemMetaData.offset + SettingsHandler::getFunscriptOffsetStep() : libraryListItemMetaData.offset - SettingsHandler::getFunscriptOffsetStep();
               SettingsHandler::setLiveOffset(newOffset);
               libraryListItemMetaData.offset = newOffset;
               SettingsHandler::updateLibraryListItemMetaData(libraryListItemMetaData);
               if(!SettingsHandler::getDisableSpeechToText())
                   textToSpeech->say(verb + " offset to " + QString::number(newOffset));
           }
        }
        else if(!SettingsHandler::getDisableSpeechToText())
                textToSpeech->say("No script playing to " + verb + " offset.");
    }
}

void MainWindow::deviceHome()
{
    _xSettings->sendTCode(tcodeHandler->getAllHome());
}
void MainWindow::deviceSwitchedHome()
{
    _xSettings->sendTCode(tcodeHandler->getSwitchedHome());
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
        if(_xSettings->isDeviceConnected())
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
                if (channel.AxisName == TCodeChannelLookup::Stroke()  || SettingsHandler::getMultiplierChecked(axis))
                {
                    if (channel.Type == AxisType::HalfRange || channel.Type == AxisType::None)
                        continue;
                    auto multiplierValue = SettingsHandler::getMultiplierValue(axis);
                    if (channel.AxisName == TCodeChannelLookup::Stroke())
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
    // disconnect(audioSyncFilter, &AudioSyncFilter::levelChanged, this, &MainWindow::on_audioLevel_Change);
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
    if (libraryList->count() > 0)
    {
        // Handle global position
        QPoint globalPos = libraryList->mapToGlobal(pos);

        // Create menu and insert some actions
        QMenu myMenu;

        QListWidgetItem* selectedItem = libraryList->selectedItems().first();
        LibraryListItem27 selectedFileListItem = ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();

        myMenu.addAction(tr("Play"), this, &MainWindow::playFileFromContextMenu);
        if(selectedFileListItem.type == LibraryListItemType::PlaylistInternal)
        {
            myMenu.addAction(tr("Open"), this, [this]()
            {
                QListWidgetItem* selectedItem = libraryList->selectedItems().first();
                LibraryListItem27 selectedFileListItem = ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();
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
            if(selectedFileListItem.type != LibraryListItemType::FunscriptType)
            {
                myMenu.addAction(tr("Play with funscript..."), this, &MainWindow::playFileWithCustomScript);
            }
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

            if(selectedFileListItem.type == LibraryListItemType::Video && !selectedFileListItem.thumbFile.contains(".lock.") && selectedPlaylistItems.length() == 0)
            {
                myMenu.addAction(tr("Regenerate thumbnail"), this, &MainWindow::regenerateThumbNail);

                if(playingLibraryListItem && (videoHandler->isPlaying() || videoHandler->isPaused()))
                {
                    LibraryListItem27 playingFileListItem = playingLibraryListItem->getLibraryListItem();
                    if(playingFileListItem.ID == selectedFileListItem.ID)
                        myMenu.addAction(tr("Set thumbnail from current"), this, &MainWindow::setThumbNailFromCurrent);
                }
                if(selectedFileListItem.thumbFileExists)
                    myMenu.addAction(tr("Lock thumb"), this, &MainWindow::lockThumb);
            }
            else if(selectedFileListItem.type == LibraryListItemType::Video &&  selectedPlaylistItems.length() == 0)
            {
                if(selectedFileListItem.thumbFileExists)
                    myMenu.addAction(tr("Unlock thumb"), this, &MainWindow::unlockThumb);
            }
            if(playingLibraryListItem && (videoHandler->isPlaying() || videoHandler->isPaused()))
            {
                LibraryListItem27 playingFileListItem = playingLibraryListItem->getLibraryListItem();
                if(playingFileListItem.ID == selectedFileListItem.ID)
                    myMenu.addAction(tr("Set moneyshot from current"), this, [this, selectedFileListItem] () {
                        onSetMoneyShot(selectedFileListItem, videoHandler->position());
                    });
            }
    //        myMenu.addAction("Add bookmark from current", this, [this, selectedFileListItem] () {
    //            onAddBookmark(selectedFileListItem, "Book mark 1", videoHandler->position());
    //        });
            myMenu.addAction(tr("Reveal in directory"), this, [this, selectedFileListItem] () {
                if(selectedFileListItem.path.isNull()) {
                    LogHandler::Dialog(this, "Invalid media path.", XLogLevel::Critical);
                    return;
                }
                if(!QFile::exists(selectedFileListItem.path)) {
                    LogHandler::Dialog(this, "Media does not exist.", XLogLevel::Critical);
                    return;
                }
                showInGraphicalShell(selectedFileListItem.path);
            });
            myMenu.addAction(tr("Edit media settings..."), this, [this, selectedFileListItem] () {
                LibraryItemSettingsDialog::getSettings(this, selectedFileListItem.path);
            });
        }

        // Show context menu at handling position
        myMenu.exec(globalPos);
    }
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
            SettingsHandler::setLinkedVRFunscript(playingPacket.path, funscriptPath);
            _syncHandler->clear();
            SettingsHandler::SaveLinkedFunscripts();
        }
    }
    else
    {
        LogHandler::Dialog(this, tr("No script for current video or no video playing"), XLogLevel::Information);
    }
}

void MainWindow::openWelcomeDialog()
{
    _welcomeDialog = new WelcomeDialog(this);
    _welcomeDialog->show();
    _welcomeDialog->raise();
}

//void MainWindow::loadLibraryAsync()
//{
//    QString library = SettingsHandler::getSelectedLibrary();
//    QString vrLibrary = SettingsHandler::getVRLibrary();
//    if(library.isEmpty() && vrLibrary.isEmpty())
//    {
//        setLibraryLoading(false);
//        return;
//    }
//    if(!loadingLibraryFuture.isRunning())
//    {
//        emit libraryLoadingStatus(true, library.isEmpty() ? "Loading VR library..." : "Loading library...");
//        loadingLibraryFuture = QtConcurrent::run([this, library, vrLibrary]() {
//            on_load_library(library.isEmpty() ? vrLibrary : library, library.isEmpty());
//        });
//    }
//}


void MainWindow::onPrepareLibraryLoad()
{
    libraryList->setIconSize({SettingsHandler::getThumbSize(),SettingsHandler::getThumbSize()});
    qDeleteAll(cachedLibraryItems);
    cachedLibraryItems.clear();
    qDeleteAll(cachedVRItems);
    cachedVRItems.clear();
    libraryList->clear();
    ui->actionReload_library->setDisabled(true);
    ui->actionSelect_library->setDisabled(true);
    _playerControlsFrame->setDisabled(true);
}

void MainWindow::onLibraryNotFound()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "ERROR!", "The media library stored in settings does not exist anymore.\nChoose a new one now?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        on_actionSelect_library_triggered();
    }
}

//void MainWindow::on_load_library(QString path, bool vrMode)
//{
//    if (path.isEmpty())
//    {
//        return;
//    }
//    else
//    {
//        QDir directory(path);
//        if(!directory.exists())
//        {
//            emit libraryNotFound();
//            return;
//        }
//    }
//    QString thumbPath = SettingsHandler::getSelectedThumbsDir();
//    QDir thumbDir(thumbPath);
//    if (!thumbDir.exists())
//    {
//        thumbDir.mkdir(thumbPath);
//    }
////    QStringList videoTypes = QStringList()
////            << "*.mp4"
////            << "*.avi"
////            << "*.mpg"
////            << "*.wmv"
////            << "*.mkv"
////            << "*.webm"
////            << "*.mp2"
////            << "*.mpeg"
////            << "*.mpv"
////            << "*.ogg"
////            << "*.m4p"
////            << "*.m4v"
////            << "*.mov"
////            << "*.qt"
////            << "*.flv"
////            << "*.swf"
////            << "*.avchd";

////    QStringList audioTypes = QStringList()
////            << "*.m4a"
////            << "*.mp3"
////            << "*.aac"
////            << "*.flac"
////            << "*.wav"
////            << "*.wma";
//    QStringList playlistTypes = QStringList()
//            << "*.m3u";

//    QStringList mediaTypes;
//    QStringList videoTypes;
//    QStringList audioTypes;
//    foreach(auto ext, SettingsHandler::getVideoExtensions())
//        videoTypes.append("*."+ext);
//    foreach(auto ext, SettingsHandler::getAudioExtensions())
//        audioTypes.append("*."+ext);
//    mediaTypes.append(videoTypes);
//    mediaTypes.append(audioTypes);
//    QDirIterator library(path, mediaTypes, QDir::Files, QDirIterator::Subdirectories);

//    if(!vrMode)
//    {
//        emit prepareLibraryLoad();
//        QThread::sleep(1);

//        auto playlists = SettingsHandler::getPlaylists();
//        foreach(auto playlist, playlists.keys())
//        {
//            setupPlaylistItem(playlist);
//        }
//    }
//    QStringList funscriptsWithMedia;
//    QList<QString> excludedLibraryPaths = SettingsHandler::getLibraryExclusions();
//    while (library.hasNext())
//    {
//        if(loadingLibraryStop)
//            return;
//        QFileInfo fileinfo(library.next());
//        QString fileDir = fileinfo.dir().path();
//        bool isExcluded = false;
//        foreach(QString dir, excludedLibraryPaths)
//        {
//            if(dir != path && (fileDir.startsWith(dir, Qt::CaseInsensitive)))
//                isExcluded = true;
//        }
//        if (isExcluded)
//            continue;
//        QString videoPath = fileinfo.filePath();
//        QString videoPathTemp = fileinfo.filePath();
//        QString fileName = fileinfo.fileName();
//        QString fileNameTemp = fileinfo.fileName();
//        QString fileNameNoExtension = fileNameTemp.remove(fileNameTemp.lastIndexOf('.'), fileNameTemp.length() -  1);
//        QString scriptFile = fileNameNoExtension + ".funscript";
//        QString scriptPath;
//        QString scriptNoExtension = videoPathTemp.remove(videoPathTemp.lastIndexOf('.'), videoPathTemp.length() - 1);
//        fileNameTemp = fileinfo.fileName();
//        QString mediaExtension = "*" + fileNameTemp.remove(0, fileNameTemp.length() - (fileNameTemp.length() - fileNameTemp.lastIndexOf('.')));

//        if (SettingsHandler::getSelectedFunscriptLibrary() == Q_NULLPTR)
//        {
//            scriptPath = scriptNoExtension + ".funscript";
//        }
//        else
//        {
//            scriptNoExtension = SettingsHandler::getSelectedFunscriptLibrary() + QDir::separator() + fileNameNoExtension;
//            scriptPath = SettingsHandler::getSelectedFunscriptLibrary() + QDir::separator() + scriptFile;
//        }

//        QFile fpath(scriptPath);
//        if (!fpath.exists())
//        {
//            scriptPath = nullptr;
//        }

//        LibraryListItemType libratyItemType = LibraryListItemType::Video;
//        QFileInfo scriptZip(scriptNoExtension + ".zip");
//        QString zipFile;
//        if(scriptZip.exists())
//            zipFile = scriptNoExtension + ".zip";
//        if(audioTypes.contains(mediaExtension))
//        {
//            libratyItemType = LibraryListItemType::Audio;
//        }
//        LibraryListItem27 item
//        {
//            libratyItemType,
//            videoPath, // path
//            fileName, // name
//            fileNameNoExtension, //nameNoExtension
//            scriptPath, // script
//            scriptNoExtension,
//            mediaExtension,
//            nullptr,
//            zipFile,
//            fileinfo.birthTime().date(),
//            0
//        };
//        item.thumbFile = mediaLibraryHandler.getThumbPath(item);
//        LibraryListWidgetItem* qListWidgetItem = new LibraryListWidgetItem(item, vrMode ? nullptr : libraryList);
//        if(!vrMode)
//            libraryList->addItem(qListWidgetItem);
//        vrMode ? cachedVRItems.push_back((LibraryListWidgetItem*)qListWidgetItem->clone()) : cachedLibraryItems.push_back((LibraryListWidgetItem*)qListWidgetItem->clone());
//        if(!vrMode && !scriptPath.isEmpty())
//            funscriptsWithMedia.append(scriptPath);
//        if(!vrMode && !zipFile.isEmpty())
//            funscriptsWithMedia.append(zipFile);
//    }

//    if(!vrMode && !SettingsHandler::getHideStandAloneFunscriptsInLibrary())
//    {
//        emit libraryLoadingStatus(true, "Searching for lone funscripts...");
//        QStringList funscriptTypes = QStringList()
//                << "*.funscript"
//                << "*.zip";
//        mediaTypes.clear();
//        mediaTypes.append(funscriptTypes);
//        QDirIterator funscripts(path, mediaTypes, QDir::Files, QDirIterator::Subdirectories);
//        auto availibleAxis = SettingsHandler::getAvailableAxis();
//        while (funscripts.hasNext())
//        {
//            if(loadingLibraryStop)
//                return;
//            QFileInfo fileinfo(funscripts.next());
//            QString fileName = fileinfo.fileName();
//            QString fileNameTemp = fileinfo.fileName();
//            QString scriptPath = fileinfo.filePath();
//            QString scriptPathTemp = fileinfo.filePath();
//            QString scriptNoExtension = scriptPathTemp.remove(scriptPathTemp.lastIndexOf('.'), scriptPathTemp.length() - 1);
//            QString scriptNoExtensionTemp = QString(scriptNoExtension);
//            if(funscriptsWithMedia.contains(scriptPath, Qt::CaseSensitivity::CaseInsensitive))
//                continue;

//            QString scriptMFSExt = scriptNoExtensionTemp.remove(0, scriptNoExtensionTemp.length() - (scriptNoExtensionTemp.length() - scriptNoExtensionTemp.lastIndexOf('.')));
//            bool isMfs = false;
//            foreach(auto axisName, availibleAxis->keys())
//            {
//                auto track = availibleAxis->value(axisName);
//                if("."+track.TrackName == scriptMFSExt)
//                {
//                    isMfs = true;
//                    break;
//                }
//            }
//            if(isMfs)
//                continue;

//            QString fileDir = fileinfo.dir().path();
//            bool isExcluded = false;
//            foreach(QString dir, excludedLibraryPaths)
//            {
//                if(dir != path && (fileDir.startsWith(dir, Qt::CaseInsensitive)))
//                    isExcluded = true;
//            }
//            if (isExcluded)
//                continue;
//            QString zipFile = nullptr;
//            if(scriptPath.endsWith(".zip", Qt::CaseInsensitive))
//            {
//                zipFile = scriptPath;
//            }
//            fileNameTemp = fileinfo.fileName();
//            QString fileNameNoExtension = fileNameTemp.remove(fileNameTemp.lastIndexOf('.'), fileNameTemp.length() -  1);
//            fileNameTemp = fileinfo.fileName();
//            QString mediaExtension = "*" + fileNameTemp.remove(0, fileNameTemp.length() - (fileNameTemp.length() - fileNameTemp.lastIndexOf('.')));
//            LibraryListItem27 item
//            {
//                LibraryListItemType::FunscriptType,
//                scriptPath, // path
//                fileName, // name
//                fileNameNoExtension, //nameNoExtension
//                scriptPath, // script
//                scriptNoExtension,
//                mediaExtension,
//                nullptr,
//                zipFile,
//                fileinfo.birthTime().date(),
//                0
//            };
//            LibraryListWidgetItem* qListWidgetItem = new LibraryListWidgetItem(item, libraryList);
//            libraryList->addItem(qListWidgetItem);
//            cachedLibraryItems.push_back((LibraryListWidgetItem*)qListWidgetItem->clone());
//        }
//    }
//    if(vrMode)
//        emit libraryLoaded();
//    else
//    {
//        QString vrLibrary = SettingsHandler::getVRLibrary();
//        if(vrLibrary.isEmpty())
//        {
//            emit libraryLoaded();
//            return;
//        }
//        QFileInfo vrLibraryInfo(vrLibrary);
//        if(!vrLibraryInfo.exists())
//        {
//            emit libraryLoaded();
//            return;
//        }
//        emit libraryLoadingStatus(true, "Loading VR library...");
//        on_load_library(SettingsHandler::getVRLibrary(), true);
//    }

//}

//LibraryListItem27 MainWindow::createLibraryListItemFromFunscript(QString funscript)
//{

//    QFileInfo fileinfo(funscript);
//    QString fileName = fileinfo.fileName();
//    QString fileNameTemp = fileinfo.fileName();
//    QString scriptPath = fileinfo.filePath();
//    QString scriptPathTemp = fileinfo.filePath();
//    QString scriptNoExtension = scriptPathTemp.remove(scriptPathTemp.lastIndexOf('.'), scriptPathTemp.length() - 1);
//    QString scriptNoExtensionTemp = QString(scriptNoExtension);
//    QString fileDir = fileinfo.dir().path();
//    QString zipFile = nullptr;
//    if(scriptPath.endsWith(".zip", Qt::CaseInsensitive))
//    {
//        zipFile = scriptPath;
//        scriptPath = nullptr;
//    }
//    fileNameTemp = fileinfo.fileName();
//    QString fileNameNoExtension = fileNameTemp.remove(fileNameTemp.lastIndexOf('.'), fileNameTemp.length() -  1);
//    fileNameTemp = fileinfo.fileName();
//    QString mediaExtension = "*" + fileNameTemp.remove(0, fileNameTemp.length() - (fileNameTemp.length() - fileNameTemp.lastIndexOf('.')));
//    return
//    {
//        LibraryListItemType::FunscriptType,
//        scriptPath, // path
//        fileName, // name
//        fileNameNoExtension, //nameNoExtension
//        scriptPath, // script
//        fileNameNoExtension,
//        mediaExtension,
//        nullptr,
//        zipFile,
//        fileinfo.birthTime().date(),
//        0
//    };
//}

void MainWindow::onSetLibraryLoading()
{
    toggleLibraryLoading(true);
}
void MainWindow::toggleLibraryLoading(bool loading)
{
    if(loading)
    {
        libraryLoadingLabel->show();
        libraryLoadingMovie->start();
        libraryLoadingInfoLabel->show();
    }
    else
    {
        libraryLoadingLabel->hide();
        libraryLoadingMovie->stop();
        libraryLoadingInfoLabel->hide();
        libraryLoadingInfoLabel->setText("");
    }
    randomizeLibraryButton->setDisabled(loading);
    savePlaylistButton->setDisabled(loading);
    editPlaylistButton->setDisabled(loading);
    cancelEditPlaylistButton->setDisabled(loading);
    windowedLibraryButton->setDisabled(loading);
    ui->actionSelect_library->setDisabled(loading);
    ui->actionReload_library->setDisabled(loading);

}
void MainWindow::onLibraryLoadingStatusChange(QString message)
{
    if(!message.isEmpty())
    {
        libraryLoadingInfoLabel->setText(message);
    }
}
void MainWindow::onLibraryItemFound(LibraryListItem27 item, bool vrMode)
{
//    auto libraryPointer = new LibraryListItem27(item);
//    QtConcurrent::run([this, libraryPointer, vrMode]() {
        LibraryListWidgetItem* qListWidgetItem = new LibraryListWidgetItem(item, vrMode ? nullptr : libraryList);
        if(!vrMode)
        libraryList->addItem(qListWidgetItem);
        vrMode ? cachedVRItems.push_back((LibraryListWidgetItem*)qListWidgetItem->clone()) : cachedLibraryItems.push_back((LibraryListWidgetItem*)qListWidgetItem->clone());
//        delete libraryPointer;
//    });
}

void MainWindow::onSetLibraryLoaded()
{
    changeLibraryDisplayMode(SettingsHandler::getLibraryView());
    updateLibrarySortUI();
    updateThumbSizeUI(SettingsHandler::getThumbSize());
    sortLibraryList(SettingsHandler::getLibrarySortMode());
    setThumbSize(SettingsHandler::getThumbSize());
    setCurrentLibraryRow(0);
    _playerControlsFrame->setDisabled(false);
    ui->actionReload_library->setDisabled(false);
    ui->actionSelect_library->setDisabled(false);

    toggleLibraryLoading(false);
}

//void MainWindow::startThumbProcess(bool vrMode)
//{
//    stopThumbProcess();
//    thumbProcessIsRunning = true;
//    extractor = new VideoFrameExtractor(this);
//    thumbNailPlayer = new AVPlayer(this);
//    thumbNailPlayer->setInterruptOnTimeout(true);
//    thumbNailPlayer->setInterruptTimeout(10000);
//    thumbNailPlayer->setAsyncLoad(true);
//    extractor->setAsync(true);
//    saveNewThumbs(vrMode);
//}

//void MainWindow::stopThumbProcess()
//{
//    if(thumbProcessIsRunning)
//    {
//        disconnect(extractor, &QtAV::VideoFrameExtractor::frameExtracted,  nullptr, nullptr);
//        disconnect(extractor, &QtAV::VideoFrameExtractor::error,  nullptr, nullptr);
//        disconnect(thumbNailPlayer, &AVPlayer::loaded,  nullptr, nullptr);
//        disconnect(thumbNailPlayer, &AVPlayer::error,  nullptr, nullptr);
//        thumbNailSearchIterator = 0;
//        thumbProcessIsRunning = false;
//        delete extractor;
//        delete thumbNailPlayer;
//    }
//}

//void MainWindow::saveSingleThumb(LibraryListWidgetItem* qListWidgetItem, qint64 position)
//{
//    if(!thumbProcessIsRunning)
//    {
//        extractor = new VideoFrameExtractor(this);
//        thumbNailPlayer = new AVPlayer(this);
//        thumbNailPlayer->setInterruptOnTimeout(true);
//        thumbNailPlayer->setInterruptTimeout(10000);
//        thumbNailPlayer->setAsyncLoad(true);
//        extractor->setAsync(true);
//    }
//    saveThumb(qListWidgetItem, position);
//}

//void MainWindow::saveNewThumbs(bool vrMode)
//{
//    if (thumbProcessIsRunning && thumbNailSearchIterator < (vrMode ? cachedVRItems.count() : cachedLibraryItems.count()))
//    {
//        //Use a non user modifiable list incase they sort random when getting thumbs.
//        LibraryListWidgetItem* listWidgetItem = vrMode ? cachedVRItems.at(thumbNailSearchIterator) : cachedLibraryItems.at(thumbNailSearchIterator);
//        LibraryListItem27 item = listWidgetItem->getLibraryListItem();
//        thumbNailSearchIterator++;
//        QFileInfo thumbInfo(mediaLibraryHandler.getThumbPath(item));
//        if (item.type == LibraryListItemType::Video && !thumbInfo.exists())
//        {
//            disconnect(extractor, nullptr,  nullptr, nullptr);
//            disconnect(thumbNailPlayer, nullptr,  nullptr, nullptr);
//            saveThumb(listWidgetItem, -1, vrMode);
//        }
//        else
//        {

//            saveNewThumbs(vrMode);
//        }
//    }
//    else
//    {
//        stopThumbProcess();
//        if(!vrMode)
//            startThumbProcess(true);
//    }
//}
//void MainWindow::saveThumb(LibraryListWidgetItem* cachedListWidgetItem, qint64 position, bool vrMode)
//{
//    LibraryListItem27 cachedListItem = cachedListWidgetItem->getLibraryListItem();
//    QString videoFile = cachedListItem.path;
//    QString thumbFile = mediaLibraryHandler.getThumbPath(cachedListItem);
////    QIcon thumb;
////    QPixmap bgPixmap(QApplication::applicationDirPath() + "/themes/loading.png");
////    QPixmap scaled = bgPixmap.scaled(SettingsHandler::getThumbSize(), Qt::AspectRatioMode::KeepAspectRatio);
////    thumb.addPixmap(scaled);
////    qListWidgetItem->setIcon(thumb);
//    auto libraryListItems = libraryList->findItems(cachedListItem.nameNoExtension, Qt::MatchFlag::MatchEndsWith);
//    LibraryListWidgetItem* libraryListWidgetItem = 0;
//    if(libraryListItems.length() > 0)
//    {
//        libraryListWidgetItem = ((LibraryListWidgetItem*)libraryListItems.first());
//        libraryListWidgetItem->setThumbFileLoading(false);
//    }
//    cachedListWidgetItem->setThumbFileLoading(false);
//    //://images/icons/loading_current.png
//    if(cachedListItem.type == LibraryListItemType::Audio || cachedListItem.type == LibraryListItemType::FunscriptType)
//    {
////        int thumbSize = SettingsHandler::getThumbSize();
////        QSize size = {thumbSize, thumbSize};
////        cachedListWidgetItem->updateThumbSize(size);
////        if(libraryListWidgetItem)
////        {
////            libraryList->removeItemWidget(libraryListWidgetItem);
////            libraryListWidgetItem->updateThumbSize(size);
////        }

//        saveNewThumbs(vrMode);
//    }
//    else
//    {
//        // Get the duration and randomize the position with in the video.
//        connect(thumbNailPlayer,
//           &AVPlayer::loaded,
//           thumbNailPlayer,
//           [this, videoFile, position]()
//            {
//               LogHandler::Debug(tr("Loaded video for thumb duration: ") + QString::number(thumbNailPlayer->duration()));
//               qint64 randomPosition = position > 0 ? position : XMath::rand((qint64)1, thumbNailPlayer->duration());

//               disconnect(thumbNailPlayer, &AVPlayer::loaded,  nullptr, nullptr);
//               disconnect(thumbNailPlayer, &AVPlayer::error,  nullptr, nullptr);
//               extractor->setSource(videoFile);
//               extractor->setPosition(randomPosition);
//            });


//        connect(thumbNailPlayer,
//           &AVPlayer::error,
//           thumbNailPlayer,
//           [this, videoFile,  cachedListWidgetItem, libraryListWidgetItem, vrMode](QtAV::AVError er)
//            {
//            QString error = tr("Video load error from: ") + videoFile + tr(" Error: ") + er.ffmpegErrorString();
//               LogHandler::Error(error);
//               saveThumbError(cachedListWidgetItem, libraryListWidgetItem, vrMode, error);
//            });


//        connect(extractor,
//           &QtAV::VideoFrameExtractor::frameExtracted,
//           extractor,
//           [this, videoFile, thumbFile, cachedListWidgetItem, libraryListWidgetItem, vrMode](const QtAV::VideoFrame& frame)
//            {
//                if(frame.isValid())
//                {
//                    bool error = false;
//                    QImage img;
//                    try{
//                        LogHandler::Debug(tr("Saving thumbnail: ") + thumbFile + tr(" for video: ") + videoFile);
//                        img = frame.toImage();
////                        auto vf = VideoFormat::pixelFormatFromImageFormat(QImage::Format_ARGB32);
////                        auto vf2 = VideoFormat(vf);
////                        VideoFrame f = frame.to(vf2, {frame.width(), frame.height()}, QRect(0,0,frame.width(), frame.height()));
////                        QImage img(f.frameDataPtr(), f.width(), f.height(), f.bytesPerLine(0), QImage::Format_ARGB32);
//                    }
//                    catch (...) {
//                        error = true;
//                    }
//                    QString errorMessage;
//                    bool hasError = error || img.isNull() || !img.save(thumbFile, nullptr, 15);
//                    if (hasError)
//                    {
//                       errorMessage = tr("Error saving thumbnail: ") + thumbFile + tr(" for video: ") + videoFile;
//                       LogHandler::Debug(errorMessage);
//                    }
//                    cachedListWidgetItem->setThumbFileLoaded(hasError, errorMessage, thumbFile);
//                    if(libraryListWidgetItem)
//                    {
//                        libraryList->removeItemWidget(libraryListWidgetItem);
//                        libraryListWidgetItem->setThumbFileLoaded(hasError, errorMessage, thumbFile);;
//                    }
//                }
//                disconnect(extractor, &QtAV::VideoFrameExtractor::frameExtracted,  nullptr, nullptr);
//                disconnect(extractor, &QtAV::VideoFrameExtractor::error,  nullptr, nullptr);

//               saveNewThumbs(vrMode);
//            });
//        connect(extractor,
//           &QtAV::VideoFrameExtractor::error,
//           extractor,
//           [this, videoFile, cachedListWidgetItem, libraryListWidgetItem, vrMode](const QString &errorMessage)
//            {
//                QString error = tr("Error extracting image from: ") + videoFile + tr(" Error: ") + errorMessage;
//                LogHandler::Error(error);
//                saveThumbError(cachedListWidgetItem, libraryListWidgetItem, vrMode, error);
//            });

//        thumbNailPlayer->setFile(videoFile);
//        thumbNailPlayer->load();
//    }
//}
void MainWindow::onSaveNewThumbLoading(LibraryListItem27 item)
{
    auto libraryListItems = libraryList->findItems(item.nameNoExtension, Qt::MatchFlag::MatchEndsWith);
    if(libraryListItems.length() > 0)
    {
        LibraryListWidgetItem* libraryListWidgetItem = (LibraryListWidgetItem*)libraryListItems.first();
        libraryListWidgetItem->setThumbFile(item.thumbFileLoadingCurrent);
    }

    LibraryListWidgetItem* cachedListWidgetItem = boolinq::from(cachedLibraryItems).firstOrDefault([item](LibraryListWidgetItem* x) { return x->getLibraryListItem().path == item.path; });
    if(cachedListWidgetItem)
        cachedListWidgetItem->setThumbFile(item.thumbFileLoadingCurrent);
}

void MainWindow::onSaveNewThumb(LibraryListItem27 item, bool vrMode, QString thumbFile)
{
    LibraryListWidgetItem* cachedListWidgetItem;
    if(!vrMode)
        cachedListWidgetItem = boolinq::from(cachedLibraryItems).firstOrDefault([item](LibraryListWidgetItem* x) { return x->getLibraryListItem().path == item.path; });
    else
        cachedListWidgetItem = boolinq::from(cachedVRItems).firstOrDefault([item](LibraryListWidgetItem* x) { return x->getLibraryListItem().path == item.path; });
    cachedListWidgetItem->setThumbFile(thumbFile);
    auto libraryListItems = libraryList->findItems(item.nameNoExtension, Qt::MatchFlag::MatchEndsWith);
    if(libraryListItems.length() > 0)
    {
        LibraryListWidgetItem* libraryListWidgetItem = (LibraryListWidgetItem*)libraryListItems.first();
        libraryList->removeItemWidget(libraryListWidgetItem);
        libraryListWidgetItem->setThumbFile(thumbFile);
    }
}

void MainWindow::onSaveThumbError(LibraryListItem27 item, bool vrMode, QString errorMessage)
{
    if(item.ID.isNull()) {
        LogHandler::Dialog(this, "Missing media", XLogLevel::Critical);
        return;
    }
    LibraryListWidgetItem* cachedListWidgetItem;
    if(!vrMode)
        cachedListWidgetItem = boolinq::from(cachedLibraryItems).firstOrDefault([item](LibraryListWidgetItem* x) { return x->getLibraryListItem().path == item.path; });
    else
        cachedListWidgetItem = boolinq::from(cachedVRItems).firstOrDefault([item](LibraryListWidgetItem* x) { return x->getLibraryListItem().path == item.path; });

    auto errorMsg = cachedListWidgetItem->toolTip() + tr("\n\nError: ") + errorMessage;
    cachedListWidgetItem->setThumbFile(item.thumbFile, errorMessage);

    auto libraryListItems = libraryList->findItems(item.nameNoExtension, Qt::MatchFlag::MatchEndsWith);
    if(libraryListItems.length() > 0)
    {
        LibraryListWidgetItem* libraryListWidgetItem = (LibraryListWidgetItem*)libraryListItems.first();
        libraryList->removeItemWidget(libraryListWidgetItem);
        libraryListWidgetItem->setThumbFile(item.thumbFile, errorMessage);
    }
}

void MainWindow::on_actionSelect_library_triggered()
{
    QString currentPath = SettingsHandler::getSelectedLibrary();
    QDir currentDir(currentPath);
    QString defaultPath = !currentPath.isEmpty() && currentDir.exists() ? currentPath : ".";
    QString selectedLibrary = QFileDialog::getExistingDirectory(this, tr("Choose media library"), defaultPath, QFileDialog::ReadOnly);
    if (selectedLibrary != Q_NULLPTR)
    {
        SettingsHandler::setSelectedLibrary(selectedLibrary);
        _mediaLibraryHandler->loadLibraryAsync();
    }
}

void MainWindow::on_LibraryList_itemClicked(QListWidgetItem *item)
{
    if(item)
    {
        auto selectedItem = item->data(Qt::UserRole).value<LibraryListItem27>();
        if((videoHandler->isPlaying() && !videoHandler->isPaused()))
        {
            auto playingFile = videoHandler->file();
            _playerControlsFrame->setPlayIcon(playingFile == selectedItem.path);
        }
        else if(_syncHandler->isPlayingStandAlone() && !_syncHandler->isPaused())
        {
            auto playingFile = _syncHandler->getPlayingStandAloneScript();
            _playerControlsFrame->setPlayIcon(playingFile == selectedItem.path);
        }
        ui->statusbar->showMessage(selectedItem.nameNoExtension);
        selectedLibraryListItem = (LibraryListWidgetItem*)item;
        selectedLibraryListIndex = libraryList->currentRow();
    }
}

void MainWindow::regenerateThumbNail()
{
    QListWidgetItem* selectedItem = libraryList->selectedItems().first();
    LibraryListItem27 selectedFileListItem = ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();
    _mediaLibraryHandler->saveSingleThumb(selectedFileListItem);
}

void MainWindow::setThumbNailFromCurrent()
{
    QListWidgetItem* selectedItem = libraryList->selectedItems().first();
    LibraryListItem27 selectedFileListItem = ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();
    _mediaLibraryHandler->saveSingleThumb(selectedFileListItem, videoHandler->position());
}

void MainWindow::lockThumb()
{
    LibraryListWidgetItem* selectedItem = (LibraryListWidgetItem*)libraryList->selectedItems().first();
    LibraryListItem27 selectedFileListItem = ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();
    _mediaLibraryHandler->lockThumb(selectedFileListItem);
    selectedItem->setThumbFile(selectedFileListItem.thumbFile);
}
void MainWindow::unlockThumb()
{
    LibraryListWidgetItem* selectedItem = (LibraryListWidgetItem*)libraryList->selectedItems().first();
    LibraryListItem27 selectedFileListItem = ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();
    _mediaLibraryHandler->unlockThumb(selectedFileListItem);
    selectedItem->setThumbFile(selectedFileListItem.thumbFile);
}

void MainWindow::on_LibraryList_itemDoubleClicked(QListWidgetItem *item)
{
    auto libraryListItem = item->data(Qt::UserRole).value<LibraryListItem27>();
    if(libraryListItem.type == LibraryListItemType::Audio || libraryListItem.type == LibraryListItemType::Video || libraryListItem.type == LibraryListItemType::FunscriptType)
    {
        stopAndPlayMedia(item->data(Qt::UserRole).value<LibraryListItem27>());
    }
    else if(libraryListItem.type == LibraryListItemType::PlaylistInternal)
    {
        loadPlaylistIntoLibrary(libraryListItem.nameNoExtension);
    }
}

void MainWindow::playFileFromContextMenu()
{
    if(libraryList->count() > 0)
    {
        LibraryListItem27 libraryListItem = ((LibraryListWidgetItem*)libraryList->selectedItems().first())->getLibraryListItem();
        if(libraryListItem.type == LibraryListItemType::Audio || libraryListItem.type == LibraryListItemType::Video || libraryListItem.type == LibraryListItemType::FunscriptType)
        {
            stopAndPlayMedia(libraryListItem);
        }
        else if(libraryListItem.type == LibraryListItemType::PlaylistInternal)
        {
            loadPlaylistIntoLibrary(libraryListItem.nameNoExtension);
            if(selectedPlaylistItems.length() > 0)
            {
                LibraryListItem27 LibraryListItem = setCurrentLibraryRow(0)->getLibraryListItem();
                stopAndPlayMedia(libraryListItem);
            }
        }
    }
}

void MainWindow::playFileWithAudioSync()
{
    LibraryListItem27 selectedFileListItem = ((LibraryListWidgetItem*)libraryList->selectedItems().first())->getLibraryListItem();
    stopAndPlayMedia(selectedFileListItem, nullptr, true);
}

void MainWindow::playFileWithCustomScript()
{
    QString selectedScript = QFileDialog::getOpenFileName(this, tr("Choose script"), SettingsHandler::getSelectedLibrary(), tr("Scripts (*.funscript *.zip)"));
    if (selectedScript != Q_NULLPTR)
    {
        LibraryListItem27 selectedFileListItem = ((LibraryListWidgetItem*)libraryList->selectedItems().first())->getLibraryListItem();
        stopAndPlayMedia(selectedFileListItem, selectedScript);
    }
}
//Hack because QTAV calls stopped and start out of order
void MainWindow::stopAndPlayMedia(LibraryListItem27 selectedFileListItem, QString customScript, bool audioSync)
{
    QFile file(selectedFileListItem.path);
    if (file.exists())
    {
        if ((!videoHandler->isPlaying() && !_syncHandler->isPlayingStandAlone())
              || ((videoHandler->isPlaying() || videoHandler->isPaused()) && videoHandler->file() != selectedFileListItem.path)
              || (_syncHandler->isPlayingStandAlone() && _syncHandler->getPlayingStandAloneScript() != selectedFileListItem.path)
              || !customScript.isEmpty()
              || audioSync)
        {
            if(playingLibraryListItem != nullptr)
                updateMetaData(playingLibraryListItem->getLibraryListItem());
            _playerControlsFrame->SetLoop(false);
            videoHandler->setLoading(true);
            if(videoHandler->isPlaying() || _syncHandler->isPlayingStandAlone())
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
void MainWindow::on_playVideo(LibraryListItem27 selectedFileListItem, QString customScript, bool audioSync)
{
    QFile file(selectedFileListItem.path);
    if (file.exists())
    {
        if ((!videoHandler->isPlaying() && !_syncHandler->isPlayingStandAlone())
                || (selectedFileListItem.type == LibraryListItemType::FunscriptType && _syncHandler->getPlayingStandAloneScript() != selectedFileListItem.path)
                || (videoHandler->file() != selectedFileListItem.path
                || !customScript.isEmpty()))
        {
            QString scriptFile;
            QList<QString> invalidScripts;
            deviceHome();
            _playerControlsFrame->SetLoop(false);
            videoHandler->setLoading(true);
            _syncHandler->stopAll();

            if(selectedFileListItem.type != LibraryListItemType::FunscriptType)
            {
                videoHandler->setFile(selectedFileListItem.path);
                _videoPreviewWidget->setFile(selectedFileListItem.path);
                //videoHandler->load();
            }
            if(!audioSync)
            {
                turnOffAudioSync();
                scriptFile = customScript.isEmpty() ? selectedFileListItem.zipFile.isEmpty() ? selectedFileListItem.script : selectedFileListItem.zipFile : customScript;
                invalidScripts = _syncHandler->load(scriptFile);
            }
            else
            {
                turnOffAudioSync();
                //strokerLastUpdate = QTime::currentTime().msecsSinceStartOfDay();
                //connect(audioSyncFilter, &AudioSyncFilter::levelChanged, this, &MainWindow::on_audioLevel_Change);
            }
            QString filesWithLoadingIssues = "";
            if(!invalidScripts.empty())
            {
                filesWithLoadingIssues += "The following scripts had issues loading:\n\n";
                foreach(auto invalidFunscript, invalidScripts)
                    filesWithLoadingIssues += "* " + invalidFunscript + "\n";
                filesWithLoadingIssues += "\n\nThis is may be due to an invalid JSON format.\nTry downloading the script again or asking the script maker.\nYou may also find some information running XTP in debug mode.";
                LogHandler::Dialog(this, filesWithLoadingIssues, XLogLevel::Critical);
            }
            if(!SettingsHandler::getDisableNoScriptFound() && selectedFileListItem.type != LibraryListItemType::FunscriptType && !audioSync && !_syncHandler->isLoaded() && !invalidScripts.contains(scriptFile))
            {
                on_scriptNotFound(scriptFile);
            }
            if(selectedFileListItem.type == LibraryListItemType::FunscriptType && _syncHandler->isLoaded())
                _syncHandler->playStandAlone();
            else if(selectedFileListItem.type == LibraryListItemType::FunscriptType && !_syncHandler->isLoaded())
            {
                on_noScriptsFound("No scripts found for the media with the same name: " + selectedFileListItem.path);
                skipForward();
            }
            else if(selectedFileListItem.type != LibraryListItemType::FunscriptType)
                videoHandler->play();
            if(playingLibraryListItem != nullptr)
                delete playingLibraryListItem;
            playingLibraryListIndex = libraryList->currentRow();
            playingLibraryListItem = (LibraryListWidgetItem*)libraryList->item(playingLibraryListIndex)->clone();

            processMetaData(selectedFileListItem);
        }
    }
    else
    {
        LogHandler::Dialog(this, tr("File '") + selectedFileListItem.path + tr("' does not exist!"), XLogLevel::Critical);
    }
}

void MainWindow::processMetaData(LibraryListItem27 libraryListItem)
{
    auto libraryListItemMetaData = SettingsHandler::getLibraryListItemMetaData(libraryListItem.path);
    if(libraryListItem.type != LibraryListItemType::VR)
    {
        if(libraryListItemMetaData.lastLoopEnabled && libraryListItemMetaData.lastLoopStart > -1 && libraryListItemMetaData.lastLoopEnd > libraryListItemMetaData.lastLoopStart)
        {
            _playerControlsFrame->SetLoop(true);
        }
    }
    SettingsHandler::setLiveOffset(libraryListItemMetaData.offset);
}

void MainWindow::updateMetaData(LibraryListItem27 libraryListItem)
{
    auto libraryListItemMetaData = SettingsHandler::getLibraryListItemMetaData(libraryListItem.path);
    if(libraryListItem.type != LibraryListItemType::VR)
    {
        libraryListItemMetaData.lastPlayPosition = videoHandler->position();
        libraryListItemMetaData.lastLoopEnabled = _playerControlsFrame->getAutoLoop();
        if(libraryListItemMetaData.lastLoopEnabled)
        {
            libraryListItemMetaData.lastLoopStart = _playerControlsFrame->getSeekSliderLowerValue();
            libraryListItemMetaData.lastLoopEnd = _playerControlsFrame->getSeekSliderUpperValue();
        }
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
    videoHandler->toggleFullscreen();
//    if(!_isFullScreen)
//    {
//        videoHandler->showFullScreen();
//        LogHandler::Debug("Before full VideoSize: width: "+QString::number(videoHandler->size().width()) + " height: " + QString::number(videoHandler->size().height()));
//        QScreen *screen = this->window()->windowHandle()->screen();
//        QSize screenSize = screen->size();
//        _videoSize = videoHandler->size();
//        _appSize = this->size();
//        _appPos = this->pos();
//        _mainStackedWidgetPos = ui->mainStackedWidget->pos();
//        _isMaximized = this->isMaximized();
//        _isFullScreen = true;
//        //QMainWindow::setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
//        _mediaGrid->removeWidget(videoHandler);
////        _controlsHomePlaceHolderGrid->removeWidget(_playerControlsFrame);

////        placeHolderControlsGrid = new QGridLayout(this);
////        placeHolderControlsGrid->setContentsMargins(0,0,0,0);
////        placeHolderControlsGrid->setSpacing(0);
////        placeHolderControlsGrid->addWidget(_playerControlsFrame);

////        playerControlsPlaceHolder = new QFrame(this);
////        playerControlsPlaceHolder->setLayout(placeHolderControlsGrid);
////        playerControlsPlaceHolder->setContentsMargins(0,0,0,0);
////        playerControlsPlaceHolder->installEventFilter(this);
////        playerControlsPlaceHolder->move(QPoint(0, screenSize.height() - _playerControlsFrame->height()));
////        playerControlsPlaceHolder->setFixedWidth(screenSize.width());
////        playerControlsPlaceHolder->setFixedHeight(_playerControlsFrame->height());
////        playerControlsPlaceHolder->setFocusPolicy(Qt::StrongFocus);
//        int rows = screenSize.height() / _playerControlsFrame->height();
//        ui->fullScreenGrid->addWidget(videoHandler, 0, 0, rows, 5);
////        ui->fullScreenGrid->addWidget(playerControlsPlaceHolder, rows - 1, 0, 1, 5);

////        if(libraryWindow == nullptr || libraryWindow->isHidden())
////        {
////            libraryOverlay = true;
////            placeHolderLibraryGrid = new QGridLayout(this);
////            placeHolderLibraryGrid->setContentsMargins(0,0,0,0);
////            placeHolderLibraryGrid->setSpacing(0);
////            placeHolderLibraryGrid->addWidget(libraryList);

////            playerLibraryPlaceHolder = new QFrame(this);
////            playerLibraryPlaceHolder->setLayout(placeHolderLibraryGrid);
////            playerLibraryPlaceHolder->setContentsMargins(0,0,0,0);
////            playerLibraryPlaceHolder->installEventFilter(this);
////            playerLibraryPlaceHolder->move(QPoint(0, screenSize.height()));
////            playerLibraryPlaceHolder->setFixedWidth(ui->libraryFrame->width());
////            playerLibraryPlaceHolder->setFixedHeight(screenSize.height() - _playerControlsFrame->height());
////            ui->fullScreenGrid->addWidget(playerLibraryPlaceHolder, 0, 0, rows - 1, 2);
////            libraryList->setProperty("cssClass", "fullScreenLibrary");
////            _playerControlsFrame->style()->unpolish(libraryList);
////            _playerControlsFrame->style()->polish(libraryList);
////            hideLibrary();
////        }

////        _playerControlsFrame->setProperty("cssClass", "fullScreenControls");
////        _playerControlsFrame->style()->unpolish(_playerControlsFrame);
////        _playerControlsFrame->style()->polish(_playerControlsFrame);
//        ui->mainStackedWidget->setCurrentIndex(1);
//        QMainWindow::centralWidget()->layout()->setMargin(0);
//        QMainWindow::showFullScreen();
//        LogHandler::Debug("After full VideoSize: width: "+QString::number(videoHandler->size().width()) + " height: " + QString::number(videoHandler->size().height()));
//        videoHandler->layout()->setMargin(0);
//        ////ui->mainStackedWidget->move(QPoint(0, 0));
//        //hideControls();
//        ui->menubar->hide();
//        ui->statusbar->hide();
//        QMainWindow::setFocus();
//    }
//    else
//    {
//        ui->mainStackedWidget->setCurrentIndex(0);
//        LogHandler::Debug("Before Normal VideoSize: width: "+QString::number(videoHandler->size().width()) + " height: " + QString::number(videoHandler->size().height()));
//        ui->fullScreenGrid->removeWidget(videoHandler);
//        _mediaGrid->addWidget(videoHandler, 0, 0, 3, 5);
////        playerControlsPlaceHolder->layout()->removeWidget(_playerControlsFrame);
////        ui->fullScreenGrid->removeWidget(playerControlsPlaceHolder);
////        _playerControlsFrame->setWindowFlags(Qt::Widget);
////        _controlsHomePlaceHolderGrid->addWidget(_playerControlsFrame);
////        _playerControlsFrame->setProperty("cssClass", "windowedControls");
////        _playerControlsFrame->style()->unpolish(_playerControlsFrame);
////        _playerControlsFrame->style()->polish(_playerControlsFrame);
////        libraryList->setProperty("cssClass", "windowedLibrary");
////        libraryList->style()->unpolish(libraryList);
////        libraryList->style()->polish(libraryList);

////        if(libraryOverlay)
////        {
////            placeHolderLibraryGrid->removeWidget(libraryList);
////            ui->fullScreenGrid->removeWidget(playerLibraryPlaceHolder);
////            libraryList->setMinimumSize(QSize(0, 0));
////            libraryList->setMaximumSize(QSize(16777215, 16777215));
////            ui->libraryGrid->addWidget(libraryList, 0, 0, 20, 12);
////            windowedLibraryButton->raise();
////            randomizeLibraryButton->raise();
////            libraryOverlay = false;
////            delete placeHolderLibraryGrid;
////            delete playerLibraryPlaceHolder;
////        }

//        videoHandler->layout()->setMargin(9);
//        QMainWindow::centralWidget()->layout()->setMargin(9);


//        if(_isMaximized)
//        {
//            QMainWindow::showMaximized();
//        }
//        else
//        {
//            QMainWindow::showNormal();
//        }
//        LogHandler::Debug("After Normal VideoSize: width: "+QString::number(videoHandler->size().width()) + " height: " + QString::number(videoHandler->size().height()));
//        ui->menubar->show();
//        ui->statusbar->show();
//        //_playerControlsFrame->show();
//        //libraryList->show();
//        _isFullScreen = false;
//        //QMainWindow::setWindowFlags(Qt::WindowFlags());
////        delete placeHolderControlsGrid;
////        delete playerControlsPlaceHolder;
//    }
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
        LibraryListWidgetItem* selectedItem;
        if(libraryList->selectedItems().length() == 0)
        {
            selectedItem = setCurrentLibraryRow(0);
        }
        else
        {
            selectedItem = (LibraryListWidgetItem*)libraryList->selectedItems().first();
        }
        LibraryListItem27 selectedFileListItem = selectedItem->getLibraryListItem();
        if(selectedFileListItem.type == LibraryListItemType::PlaylistInternal)
        {
            loadPlaylistIntoLibrary(selectedFileListItem.nameNoExtension, true);
        }
        else if((videoHandler->isPlaying() && selectedFileListItem.path != videoHandler->file()) || (!videoHandler->isPlaying() && !_syncHandler->isPlayingStandAlone()))
        {
            stopAndPlayMedia(selectedFileListItem);
        }
        else if(videoHandler->isPaused() || videoHandler->isPlaying())
        {
            videoHandler->togglePause();
            if(_syncHandler->isPlayingStandAlone())
            {
                _syncHandler->togglePause();
            }
        }
        else if(_syncHandler->isPlayingStandAlone())
        {
            _syncHandler->togglePause();
        }
    }
}

LibraryListWidgetItem* MainWindow::setCurrentLibraryRow(int row)
{
    if(libraryList->count() > 0)
    {
        libraryList->setCurrentRow(row);
        auto item = (LibraryListWidgetItem*)libraryList->item(row);
        on_LibraryList_itemClicked(item);
        item->setSelected(true);
        return item;
    }
    return nullptr;
}

void MainWindow::on_togglePaused(bool paused)
{
    _playerControlsFrame->setPlayIcon(!paused);
    if(paused)
        deviceSwitchedHome();
}

void MainWindow::stopMedia()
{
    if(videoHandler->isPlaying())
    {
        videoHandler->stop();
    }
    if(_syncHandler->isPlayingStandAlone())
    {
        _syncHandler->stopStandAloneFunscript();
    }
    deviceHome();
}

void MainWindow::on_MuteBtn_toggled(bool checked)
{
    videoHandler->toggleMute();
}

void MainWindow::on_fullScreenBtn_clicked()
{
    toggleFullScreen();
}

void MainWindow::on_seekslider_hover(int position, int sliderValue)
{
    qint64 sliderValueTime = XMath::mapRange(static_cast<qint64>(sliderValue), (qint64)0, (qint64)100, (qint64)0, videoHandler->duration());
//    if (!videoPreviewWidget)
//        videoPreviewWidget = new VideoPreviewWidget();
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

    if(playingLibraryListItem->getType() == LibraryListItemType::Video && (videoHandler->isPlaying() || videoHandler->isPaused()))
    {
        //const int w = Config::instance().previewWidth();
        //const int h = Config::instance().previewHeight();
        //videoPreviewWidget->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        _videoPreviewWidget->setTimestamp(sliderValueTime);
        _videoPreviewWidget->preview(gpos);
        //videoPreviewWidget->raise();
        //videoPreviewWidget->activateWindow();
    }
}

void MainWindow::on_seekslider_leave()
{
    if (!_videoPreviewWidget)
    {
        return;
    }
    if (_videoPreviewWidget->isVisible())
    {
        _videoPreviewWidget->close();
    }
//    delete videoPreviewWidget;
//    videoPreviewWidget = NULL;
}

void MainWindow::on_seekSlider_sliderMoved(int position)
{
    LogHandler::Debug("position: "+ QString::number(position));
    if (!_playerControlsFrame->getAutoLoop())
    {
        bool isStandAloneFunscriptPlaying = playingLibraryListItem->getLibraryListItem().type == LibraryListItemType::FunscriptType;
        qint64 duration = isStandAloneFunscriptPlaying ? _syncHandler->getFunscriptMax() : videoHandler->duration();
        qint64 playerPosition = XMath::mapRange(static_cast<qint64>(position), (qint64)0, (qint64)100, (qint64)0, duration);

        LogHandler::Debug("playerPosition: "+ QString::number(playerPosition));
        if(playerPosition <= 0)
            playerPosition = 50;
        isStandAloneFunscriptPlaying ? _syncHandler->setFunscriptTime(playerPosition) : videoHandler->setPosition(playerPosition);
    }
}


void MainWindow::onLoopRange_valueChanged(int position, int startLoop, int endLoop)
{
    bool isStandAloneFunscriptPlaying = playingLibraryListItem->getLibraryListItem().type == LibraryListItemType::FunscriptType;
    if(endLoop >= 100)
        endLoop = 99;
    qint64 duration = isStandAloneFunscriptPlaying ? _syncHandler->getFunscriptMax() : videoHandler->duration();
    qint64 mediaPosition = isStandAloneFunscriptPlaying ? _syncHandler->getFunscriptTime() : videoHandler->position();

    qint64 currentVideoPositionPercentage = XMath::mapRange(mediaPosition,  (qint64)0, duration, (qint64)0, (qint64)100);
    qint64 destinationVideoPosition = XMath::mapRange((qint64)position, (qint64)0, (qint64)100,  (qint64)0, duration);

    QString timeCurrent = mSecondFormat(destinationVideoPosition);
    _playerControlsFrame->setSeekSliderToolTip(timeCurrent);

    if(currentVideoPositionPercentage < startLoop)
    {
        isStandAloneFunscriptPlaying ? _syncHandler->setFunscriptTime(destinationVideoPosition) : videoHandler->setPosition(destinationVideoPosition);
    }
    else if (currentVideoPositionPercentage >= endLoop)
    {
        qint64 startLoopVideoPosition = XMath::mapRange((qint64)startLoop, (qint64)0, (qint64)100,  (qint64)0, duration);
        if(startLoopVideoPosition <= 0)
            startLoopVideoPosition = 50;
        if (position != startLoopVideoPosition)
            isStandAloneFunscriptPlaying ? _syncHandler->setFunscriptTime(destinationVideoPosition) : videoHandler->setPosition(destinationVideoPosition);
    }
}

void MainWindow::on_media_positionChanged(qint64 position)
{
    bool isStandAloneFunscriptPlaying = playingLibraryListItem->getLibraryListItem().type == LibraryListItemType::FunscriptType;
    qint64 duration = isStandAloneFunscriptPlaying ? _syncHandler->getFunscriptMax() : videoHandler->duration();
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
            if (position != startLoopVideoPosition)
                isStandAloneFunscriptPlaying ? _syncHandler->setFunscriptTime(startLoopVideoPosition) : videoHandler->seek(startLoopVideoPosition);
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

void MainWindow::on_standaloneFunscript_start()
{
    LogHandler::Debug("Enter on_standaloneFunscript_start");
    if(_xSettings->getConnectedVRDeviceHandler())
        _xSettings->getConnectedVRDeviceHandler()->dispose();
    videoHandler->setLoading(false);
    _playerControlsFrame->resetMediaControlStatus(true);
}

void MainWindow::on_standaloneFunscript_stop()
{
    LogHandler::Debug("Enter on_standaloneFunscript_stop");
    videoHandler->setLoading(false);
    _playerControlsFrame->resetMediaControlStatus(false);
}

void MainWindow::on_media_start()
{
    LogHandler::Debug("Enter on_media_start");
    if(_xSettings->getConnectedVRDeviceHandler())
        _xSettings->getConnectedVRDeviceHandler()->dispose();
    if (_syncHandler->isLoaded())
    {
        _syncHandler->syncFunscript();
    }
    videoHandler->setLoading(false);
    _playerControlsFrame->resetMediaControlStatus(true);
    _mediaStopped = false;
}

void MainWindow::on_media_stop()
{
    LogHandler::Debug("Enter on_media_stop");
    videoHandler->setLoading(false);
    _playerControlsFrame->resetMediaControlStatus(false);
    _syncHandler->stopMediaFunscript();
    _mediaStopped = true;
}

void MainWindow::on_scriptNotFound(QString message)
{
    NoMatchingScriptDialog::show(this, message);
}
void MainWindow::on_noScriptsFound(QString message)
{
    LogHandler::Dialog(this, message, XLogLevel::Critical);
}

void MainWindow::onVRMessageRecieved(VRPacket packet)
{
        //LogHandler::Debug("VR path: "+packet.path);
//LogHandler::Debug("VR duration: "+QString::number(packet.duration));
        //LogHandler::Debug("VR currentTime: "+QString::number(packet.currentTime));
//        LogHandler::Debug("VR playbackSpeed: "+QString::number(packet.playbackSpeed));
//        LogHandler::Debug("VR playing: "+QString::number(packet.playing));

    QString videoPath = packet.path.isEmpty() ? packet.path : QUrl::fromPercentEncoding(packet.path.toUtf8());
    if(!videoPath.isEmpty() && videoPath != lastVRScriptPath)
    {
        LogHandler::Debug("onVRMessageRecieved video changed: "+videoPath);
        LogHandler::Debug("onVRMessageRecieved old path: "+lastVRScriptPath);
        vrScriptSelectorCanceled = false;
        vrScriptSelectorRunning = false;
    } else if(videoPath.isEmpty() || packet.duration <= 0 || _syncHandler->isPlaying())
        return;

    if (!vrScriptSelectorRunning && !vrScriptSelectorCanceled && !funscriptFileSelectorOpen)
    {
        vrScriptSelectorRunning = true;
        QFileInfo videoFile(videoPath);
        QString libraryPath = SettingsHandler::getSelectedLibrary();
        QString vrLibraryPath = SettingsHandler::getVRLibrary();
        QString funscriptPath;
        bool saveLinkedScript = false;
        if(videoPath.contains("http"))
        {
            LogHandler::Debug("onVRMessageRecieved Funscript is http: "+ videoPath);
            QUrl funscriptUrl = QUrl(videoPath);
            QString path = funscriptUrl.path();
            QString localpath = path;
            if(path.startsWith("/media"))
                localpath = path.remove("/media/");
            int indexOfSuffix = localpath.lastIndexOf(".");
            QString localFunscriptPath = localpath.replace(indexOfSuffix, localpath.length() - indexOfSuffix, ".funscript");
            QString localFunscriptZipPath = localpath.replace(indexOfSuffix, localpath.length() - indexOfSuffix, ".zip");
            QString libraryScriptPath = libraryPath + QDir::separator() + localFunscriptPath;
            QString libraryScriptZipPath = libraryPath + QDir::separator() + localFunscriptZipPath;
            QFile libraryFile(libraryScriptPath);
            QFile libraryZipFile(libraryScriptZipPath);
            if(libraryFile.exists())
            {
                LogHandler::Debug("onVRMessageRecieved Script found in url path: "+libraryScriptPath);
                funscriptPath = libraryScriptPath;
            }
            else if(libraryZipFile.exists())
            {
                LogHandler::Debug("onVRMessageRecieved Script zip found in url path: "+libraryScriptPath);
                funscriptPath = libraryScriptZipPath;
            }
            else {
                LogHandler::Debug("onVRMessageRecieved Script not found in url path");
            }
        }

        if(funscriptPath.isEmpty())
        {
            funscriptPath = SettingsHandler::getDeoDnlaFunscript(videoPath);
            if(!funscriptPath.isEmpty())
            {
                QFileInfo funscriptFile(funscriptPath);
                if(!funscriptFile.exists())
                {
                    SettingsHandler::removeLinkedVRFunscript(videoPath);
                    funscriptPath = nullptr;
                }
            }
        }

        if (funscriptPath.isEmpty())
        {
            //Check the deo device local video directory for funscript.
            QString tempPath = videoPath;
            QString tempZipPath = videoPath;
            int indexOfSuffix = tempPath.lastIndexOf(".");
            QString localFunscriptPath = tempPath.replace(indexOfSuffix, tempPath.length() - indexOfSuffix, ".funscript");
            QString localFunscriptZipPath = tempZipPath.replace(indexOfSuffix, tempZipPath.length() - indexOfSuffix, ".zip");
            QString libraryScriptPath = libraryPath + QDir::separator() + localFunscriptPath;
            QString libraryScriptZipPath = libraryPath + QDir::separator() + localFunscriptZipPath;
            QFile localFile(localFunscriptPath);
            QFile libraryZipFile(libraryScriptZipPath);
            LogHandler::Debug("onVRMessageRecieved Searching local path: "+localFunscriptPath);
            if(localFile.exists())
            {
                LogHandler::Debug("onVRMessageRecieved script found in path of media");
                funscriptPath = localFunscriptPath;
            }
            else if (libraryZipFile.exists())
            {
                LogHandler::Debug("onVRMessageRecieved script zip found in path of media");
                funscriptPath = libraryScriptZipPath;
            }
            else if(!vrLibraryPath.isEmpty())
            {
                QString vrLibraryScriptPath = vrLibraryPath + QDir::separator() + localFunscriptPath;
                QString vrLibraryScriptZipPath = vrLibraryPath + QDir::separator() + localFunscriptZipPath;
                LogHandler::Debug("onVRMessageRecieved Searching for local path in VR library root: "+ vrLibraryScriptPath);
                QFile localVRFile(vrLibraryScriptPath);
                QFile libraryVRZipFile(vrLibraryScriptZipPath);
                if(localVRFile.exists())
                {
                    LogHandler::Debug("onVRMessageRecieved script found in path of VR media");
                    funscriptPath = vrLibraryScriptPath;
                }
                else if (libraryVRZipFile.exists())
                {
                    LogHandler::Debug("onVRMessageRecieved script zip found in path of VR media");
                    funscriptPath = vrLibraryScriptZipPath;
                }
            }
        }

//        if (funscriptPath.isEmpty())
//        {
//            LogHandler::Debug("onVRMessageRecieved funscript not found in app data");
//            //Check the user selected library location.
//            QString libraryScriptFile = videoFile.fileName().remove(videoFile.fileName().lastIndexOf('.'), videoFile.fileName().length() -  1) + ".funscript";
//            QString libraryScriptZipFile = videoFile.fileName().remove(videoFile.fileName().lastIndexOf('.'), videoFile.fileName().length() -  1) + ".zip";
//            QString libraryScriptPath = libraryPath + QDir::separator() + libraryScriptFile;
//            QString libraryScriptZipPath = libraryPath + QDir::separator() + libraryScriptZipFile;
//            LogHandler::Debug("onVRMessageRecieved Searching for file in library root: "+ libraryScriptPath);
//            QFile libraryFile(libraryScriptPath);
//            QFile libraryZipFile(libraryScriptZipPath);
//            if(libraryFile.exists())
//            {
//                LogHandler::Debug("onVRMessageRecieved Script found in root: "+libraryScriptFile);
//                funscriptPath = libraryScriptPath;
//            }
//            else if(libraryZipFile.exists())
//            {
//                LogHandler::Debug("onVRMessageRecieved Script zip found in root: "+libraryScriptZipFile);
//                funscriptPath = libraryScriptZipPath;
//            }
//            else if(!vrLibraryPath.isEmpty())
//            {
//                QString vrLibraryScriptPath = vrLibraryPath + QDir::separator() + libraryScriptFile;
//                QString vrLibraryScriptZipPath = vrLibraryPath + QDir::separator() + libraryScriptZipFile;
//                LogHandler::Debug("onVRMessageRecieved Searching for file in VR library root: "+ vrLibraryScriptPath);
//                QFile localVRFile(vrLibraryScriptPath);
//                QFile libraryVRZipFile(vrLibraryScriptZipPath);
//                if(localVRFile.exists())
//                {
//                    LogHandler::Debug("onVRMessageRecieved Script found in VR root: "+libraryScriptFile);
//                    funscriptPath = vrLibraryScriptPath;
//                }
//                else if(libraryVRZipFile.exists())
//                {
//                    LogHandler::Debug("onVRMessageRecieved Script zip found in VR root: "+libraryScriptZipFile);
//                    funscriptPath = vrLibraryScriptZipPath;
//                }
//                else {
//                    LogHandler::Debug("onVRMessageRecieved Script not found");
//                }
//            }
//            else {
//                LogHandler::Debug("onVRMessageRecieved Script not found");
//            }
//        }

        if (funscriptPath.isEmpty())
        {
            LogHandler::Debug("onVRMessageRecieved: Search ALL sub directories of both libraries for the funscript");
            QString libraryScriptFile = videoFile.fileName().remove(videoFile.fileName().lastIndexOf('.'), videoFile.fileName().length() -  1) + ".funscript";
            QString libraryScriptZipFile = videoFile.fileName().remove(videoFile.fileName().lastIndexOf('.'), videoFile.fileName().length() -  1) + ".zip";
            if(!libraryPath.isEmpty() && QFileInfo(libraryPath).exists()) {
                QDirIterator directory(libraryPath,QDirIterator::Subdirectories);
                while (vrScriptSelectorRunning && directory.hasNext()) {
                    directory.next();
                    if (QFileInfo(directory.filePath()).isFile()) {
                        QString fileName = directory.fileName();
                        if (fileName.contains(libraryScriptFile) || fileName.contains(libraryScriptZipFile)) {
                            funscriptPath = directory.filePath();
                            LogHandler::Debug("onVRMessageRecieved Script found in library: "+funscriptPath);
                            break;
                        }
                    }
                }
            }
            if (funscriptPath.isEmpty() && !vrLibraryPath.isEmpty() && QFileInfo(vrLibraryPath).exists())
            {
                QDirIterator directory(vrLibraryPath,QDirIterator::Subdirectories);
                while (vrScriptSelectorRunning && directory.hasNext()) {
                    directory.next();
                    if (QFileInfo(directory.filePath()).isFile()) {
                        QString fileName = directory.fileName();
                        if (fileName.contains(libraryScriptFile) || fileName.contains(libraryScriptZipFile)){
                            funscriptPath = directory.filePath();
                            LogHandler::Debug("onVRMessageRecieved Script found in VR library: "+funscriptPath);
                            break;
                        }
                    }
                }
            }
        }

        //If the above locations fail ask the user to select a file manually.
        if (funscriptPath.isEmpty())
        {
            if(!SettingsHandler::getDisableVRScriptSelect())
            {
                LogHandler::Debug("onVRMessageRecieved Enter no scripts found. Ask user");
                if (!SettingsHandler::getDisableSpeechToText())
                    textToSpeech->say("Script for video playing in VR not found. Please check your computer to select a script.");
                funscriptFileSelectorOpen = true;
                funscriptPath = QFileDialog::getOpenFileName(this, "Choose script for video: " + videoFile.fileName(), SettingsHandler::getSelectedLibrary(), "Script Files (*.funscript);;Zip (*.zip)");
                funscriptFileSelectorOpen = false;
                saveLinkedScript = true;
                //LogHandler::Debug("funscriptPath: "+funscriptPath);
            }
            if(funscriptPath.isEmpty())
            {
                LogHandler::Debug("Funscript selector canceled");
                vrScriptSelectorCanceled = true;
            }
        }

        if(!funscriptPath.isEmpty())
        {
            LogHandler::Debug("Starting sync: "+funscriptPath);
            processVRMetaData(videoPath, funscriptPath, packet.duration);
            _syncHandler->syncVRFunscript(funscriptPath);
            if(saveLinkedScript)
            {
                LogHandler::Debug("Saving script into data: "+funscriptPath);
                //Store the location of the file so the user doesnt have to select it again.
                SettingsHandler::setLinkedVRFunscript(videoPath, funscriptPath);
                SettingsHandler::SaveLinkedFunscripts();
            }
        }
        lastVRScriptPath = videoPath;

        vrScriptSelectorRunning = false;
    }
//    else if(vrScriptSelectedCanceledPath != packet.path)
//    {
//        LogHandler::Debug("onVRMessageRecieved funscript found in data store: " + funscriptPath);
//        _syncHandler->syncVRFunscript(funscriptPath);
//        vrScriptSelectedCanceledPath = packet.path;
//    }
}

void MainWindow::processVRMetaData(QString videoPath, QString funscriptPath, qint64 duration)
{
    QFileInfo videoFile(videoPath);
    QString fileNameTemp = videoFile.fileName();
    QString videoPathTemp = videoFile.fileName();
    QString mediaExtension = "*" + fileNameTemp.remove(0, fileNameTemp.length() - (fileNameTemp.length() - fileNameTemp.lastIndexOf('.')));
    QString scriptNoExtension = videoPathTemp.remove(videoPathTemp.lastIndexOf('.'), videoPathTemp.length() - 1);
    QString zipFile;
    if(funscriptPath.endsWith(".zip"))
        zipFile = funscriptPath;
    LibraryListItem27 item ;
    item.type = LibraryListItemType::VR;
    item.path = videoPath; // path
    item.name = videoFile.fileName(); // name
    item.nameNoExtension = scriptNoExtension; //nameNoExtension
    item.script = funscriptPath; // script
    item.scriptNoExtension = scriptNoExtension;
    item.mediaExtension = mediaExtension;
    item.zipFile = zipFile;
    item.modifiedDate = videoFile.birthTime().isValid() ? videoFile.birthTime().date() : videoFile.created().date();
    item.duration = (unsigned)duration;
    _mediaLibraryHandler->setLiveProperties(item);
    playingLibraryListItem = new LibraryListWidgetItem(item);
    processMetaData(item);
}

void MainWindow::on_gamepad_sendTCode(QString value)
{
    if(_xSettings->isDeviceConnected())
    {
        if(SettingsHandler::getFunscriptLoaded( TCodeChannelLookup::Stroke()) &&
                _syncHandler->isPlaying() &&
                ((videoHandler->isPlaying() && !videoHandler->isPaused())
                    || (_xSettings->getConnectedVRDeviceHandler() && _xSettings->getConnectedVRDeviceHandler()->isPlaying())))
        {
            QRegularExpression rx("L0[^\\s]*\\s?");
            value = value.remove(rx);
        }

        if((value.contains(TCodeChannelLookup::Suck()) && value.contains(TCodeChannelLookup::SuckPosition())))
        {
            QRegularExpression rx("A1[^\\s]*\\s?");
            value = value.remove(rx);
        }
        _xSettings->sendTCode(value);
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

void MainWindow::on_media_statusChanged(XMediaStatus status)
{
    switch (status) {
    case XMediaStatus::EndOfMedia:
        if (!_playerControlsFrame->getAutoLoop())
            skipForward();
    break;
    case XMediaStatus::NoMedia:
        //status = tr("No media");
        break;
    case XMediaStatus::InvalidMedia:
        //status = tr("Invalid meida");
        break;
    case XMediaStatus::BufferingMedia:
        videoHandler->setLoading(true);
        break;
    case XMediaStatus::BufferedMedia:
        videoHandler->setLoading(false);
        break;
    case XMediaStatus::LoadingMedia:
        videoHandler->setLoading(true);
        break;
    case XMediaStatus::LoadedMedia:
        videoHandler->setLoading(false);
        break;
    case XMediaStatus::StalledMedia:

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
        if(libraryListItem.type == LibraryListItemType::PlaylistInternal || (SettingsHandler::getSkipPlayingStandAloneFunscriptsInLibrary() && libraryListItem.type == LibraryListItemType::FunscriptType))
        {
            skipForward();
        }
        else
            stopAndPlayMedia(libraryListItem);
    }
}

void MainWindow::skipBack()
{
    if (libraryList->count() > 0)
    {
        if(!videoHandler->isPlaying() || videoHandler->position() < 5000)
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
                stopAndPlayMedia(libraryListItem);
        }
        else
            videoHandler->seek(50);
    }
}

void MainWindow::rewind()
{
    qint64 position = videoHandler->position();
    qint64 videoIncrement = SettingsHandler::getVideoIncrement() * 1000;
    if (position > videoIncrement)
        if(_syncHandler->isPlayingStandAlone())
            _syncHandler->setFunscriptTime(_syncHandler->getFunscriptTime() - videoIncrement);
        else
            videoHandler->seek(videoHandler->position() - videoIncrement);
    else
        skipBack();
}

void MainWindow::fastForward()
{
    qint64 position = videoHandler->position();
    qint64 videoIncrement = SettingsHandler::getVideoIncrement() * 1000;
    if (position < videoHandler->duration() - videoIncrement)
        if(_syncHandler->isPlayingStandAlone())
            _syncHandler->setFunscriptTime(_syncHandler->getFunscriptTime() + videoIncrement);
        else
            videoHandler->seek(videoHandler->position() + videoIncrement);
    else
        skipForward();
}

void MainWindow::media_double_click_event(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton )
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
    if(deviceConnected)
    {
        connectionStatusLabel->setProperty("cssClass", "connectionStatusConnected");
        deviceHome();
    }
    else if(event.status == ConnectionStatus::Connecting)
        connectionStatusLabel->setProperty("cssClass", "connectionStatusConnecting");
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
        connectionStatusLabel->setProperty("cssClass", "connectionStatusDisconnected");
    }
    else
    {
        retryConnectionButton->hide();
    }
    connectionStatusLabel->style()->unpolish(connectionStatusLabel);
    connectionStatusLabel->style()->polish(connectionStatusLabel);
}

void MainWindow::on_device_error(QString error)
{
    LogHandler::Dialog(this, error, XLogLevel::Critical);
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


void MainWindow::on_vr_device_connectionChanged(ConnectionChangedSignal event)
{
    QString message = "";
    message += event.deviceType == DeviceType::Whirligig ? "Whirligig: " : "DeoVR/HereSphere: ";
    message += " " + event.message;
    vrConnectionStatusLabel->setText(message);
    if(SettingsHandler::getWhirligigEnabled() && (event.status == ConnectionStatus::Error || event.status == ConnectionStatus::Disconnected))
    {
        ui->actionChange_current_deo_script->setEnabled(false);
        vrRetryConnectionButton->show();
        deviceHome();
    }
    else if(event.status == ConnectionStatus::Connected)
    {
        ui->actionChange_current_deo_script->setEnabled(true);
        vrRetryConnectionButton->hide();
        deviceHome();
        stopMedia();
        _syncHandler->stopAll();

    }
    else if(event.status == ConnectionStatus::Connecting)
    {
        vrConnectionStatusLabel->show();
    }
    else
    {
        ui->actionChange_current_deo_script->setEnabled(false);
        vrConnectionStatusLabel->hide();
        vrRetryConnectionButton->hide();
    }
}

void MainWindow::on_vr_device_error(QString error)
{
    LogHandler::Dialog(this, "VR sync connection error: "+error, XLogLevel::Critical);
}

void MainWindow::on_xtpWeb_device_connectionChanged(ConnectionChangedSignal event)
{
    QString message = "";
    message += "XTP Web: ";
    message += " " + event.message;
    xtpWebStatusLabel->setText(message);
    xtpWebStatusLabel->show();
    if(event.status == ConnectionStatus::Connected)
    {
        deviceHome();
        stopMedia();
    }
    else if(event.status == ConnectionStatus::Connecting)
    {
        xtpWebStatusLabel->show();
    }
    else
    {
        xtpWebStatusLabel->hide();
    }
}

void MainWindow::on_xtpWeb_device_error(QString error)
{
    LogHandler::Dialog(this, "XTP Web error: "+error, XLogLevel::Critical);
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
                       + SettingsHandler::getSelectedTCodeVersion() + "<br>"
                                                "Copyright 2022 Jason C. Fain<br>"
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
    QLabel qthttpServerInfo;
    qthttpServerInfo.setFrameStyle(QFrame::Panel | QFrame::Sunken);
    qthttpServerInfo.setText("<b>HttpServer</b><br>"
                      "Copyright (C) 2019 Addison Elliott (MIT)<br>"
                      "<a href='https://github.com/addisonElliott/HttpServer'>https://github.com/addisonElliott/HttpServer</a>");
    qthttpServerInfo.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&qthttpServerInfo);
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
    changeLibraryDisplayMode(LibraryView::Thumb);
    setThumbSize(SettingsHandler::getThumbSize());
}

void MainWindow::on_actionList_triggered()
{
    SettingsHandler::setLibraryView(LibraryView::List);
    changeLibraryDisplayMode(LibraryView::List);
    setThumbSize(SettingsHandler::getThumbSize());
}

void MainWindow::changeLibraryDisplayMode(LibraryView value)
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
//    if(selectedPlaylistItems.length() > 0)
//    {
//        libraryList->setDragEnabled(true);
//        libraryList->setDragDropMode(QAbstractItemView::DragDrop);
//        libraryList->setDefaultDropAction(Qt::MoveAction);
//        libraryList->setMovement(QListView::Movement::Snap);
//    }
}

void MainWindow::updateThumbSizeUI(int size)
{
    switch(size)
    {
        case 75:
            action75_Size->setChecked(true);
        break;
        case 100:
            action100_Size->setChecked(true);
        break;
        case 125:
            action125_Size->setChecked(true);
        break;
        case 150:
            action150_Size->setChecked(true);
        break;
        case 175:
            action175_Size->setChecked(true);
        break;
        case 200:
            action200_Size->setChecked(true);
        break;
        default:
            actionCustom_Size->setChecked(true);
        break;
    }
    setThumbSize(size);
}

void MainWindow::on_action75_triggered()
{
    SettingsHandler::setThumbSize(75);
    setThumbSize(75);
}

void MainWindow::on_action100_triggered()
{
    SettingsHandler::setThumbSize(100);
    setThumbSize(100);
}

void MainWindow::on_action125_triggered()
{
    SettingsHandler::setThumbSize(125);
    setThumbSize(125);
}

void MainWindow::on_action150_triggered()
{
    SettingsHandler::setThumbSize(150);
    setThumbSize(150);
}

void MainWindow::on_action175_triggered()
{
    SettingsHandler::setThumbSize(175);
    setThumbSize(175);
}

void MainWindow::on_action200_triggered()
{
    SettingsHandler::setThumbSize(200);
    setThumbSize(200);
}

void MainWindow::on_actionCustom_triggered()
{
    bool ok;
    int size = QInputDialog::getInt(this, tr("Custom size"), "Size (Max:"+QString::number(SettingsHandler::getMaxThumbnailSize().height()) + ")",
                                         SettingsHandler::getThumbSize(), 1, SettingsHandler::getMaxThumbnailSize().height(), 50, &ok);
    if (ok && size > 0)
    {
        setThumbSize(size);
        SettingsHandler::setThumbSize(size);
    }
}

void MainWindow::setThumbSize(int size)
{
    resizeThumbs(size);

    videoHandler->setMinimumHeight(size);
    videoHandler->setMinimumWidth(size);
//    if(SettingsHandler::getLibraryView() == LibraryView::List)
//        libraryList->setViewMode(QListView::ListMode);
//    else
//        libraryList->setViewMode(QListView::IconMode);

}
bool MainWindow::isLibraryLoading()
{
    return _mediaLibraryHandler->isLibraryLoading() || loadingLibraryFuture.isRunning();
}
void MainWindow::resizeThumbs(int size)
{
    if(!isLibraryLoading())
    {
        toggleLibraryLoading(true);
        onLibraryLoadingStatusChange("Resizing thumbs...");
        loadingLibraryFuture = QtConcurrent::run([this, size]() {
            QSize newSize = {size, size};
            for(int i = 0; i < libraryList->count(); i++)
            {
                if(loadingLibraryStop)
                    return;
                ((LibraryListWidgetItem*)libraryList->item(i))->updateThumbSize(newSize);
            }
            emit libraryIconResized(newSize);
        });
    }
}

void MainWindow::libraryListSetIconSize(QSize newSize)
{
    libraryList->setIconSize(newSize);
    toggleLibraryLoading(false);
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
        break;
        case LibrarySortMode::NAME_DESC:
            actionNameDesc_Sort->setChecked(true);
        break;
        case LibrarySortMode::CREATED_ASC:
            actionCreatedAsc_Sort->setChecked(true);
        break;
        case LibrarySortMode::CREATED_DESC:
            actionCreatedDesc_Sort->setChecked(true);
        break;
        case LibrarySortMode::RANDOM:
            actionRandom_Sort->setChecked(true);
        break;
        case LibrarySortMode::TYPE_ASC:
            actionTypeAsc_Sort->setChecked(true);
        break;
        case LibrarySortMode::TYPE_DESC:
            actionTypeDesc_Sort->setChecked(true);
        break;
    }
}

void MainWindow::on_actionNameAsc_triggered()
{
    SettingsHandler::setLibrarySortMode(LibrarySortMode::NAME_ASC);
    sortLibraryList(LibrarySortMode::NAME_ASC);
}
void MainWindow::on_actionNameDesc_triggered()
{
    SettingsHandler::setLibrarySortMode(LibrarySortMode::NAME_DESC);
    sortLibraryList(LibrarySortMode::NAME_DESC);
}
void MainWindow::on_actionRandom_triggered()
{
    SettingsHandler::setLibrarySortMode(LibrarySortMode::RANDOM);
    sortLibraryList(LibrarySortMode::RANDOM);
}
void MainWindow::on_actionCreatedAsc_triggered()
{
    SettingsHandler::setLibrarySortMode(LibrarySortMode::CREATED_ASC);
    sortLibraryList(LibrarySortMode::CREATED_ASC);
}
void MainWindow::on_actionCreatedDesc_triggered()
{
    SettingsHandler::setLibrarySortMode(LibrarySortMode::CREATED_DESC);
    sortLibraryList(LibrarySortMode::CREATED_DESC);
}
void MainWindow::on_actionTypeAsc_triggered()
{
    SettingsHandler::setLibrarySortMode(LibrarySortMode::TYPE_ASC);
    sortLibraryList(LibrarySortMode::TYPE_ASC);
}
void MainWindow::on_actionTypeDesc_triggered()
{
    SettingsHandler::setLibrarySortMode(LibrarySortMode::TYPE_DESC);
    sortLibraryList(LibrarySortMode::TYPE_DESC);
}

void MainWindow::sortLibraryList(LibrarySortMode sortMode)
{
    if(sortMode == LibrarySortMode::RANDOM)
    {
        if(!isLibraryLoading())
        {
            toggleLibraryLoading(true);
            onLibraryLoadingStatusChange("Randomizing...");

            loadingLibraryFuture = QtConcurrent::run([this]() {
                //Fisher and Yates algorithm
                int n = libraryList->count();

                QList<LibraryListWidgetItem*> arr, arr1;
                int index_arr[n];
                int index;

                for (int i = 0; i < n; i++)
                {
                    if(loadingLibraryStop)
                        return;
                    index_arr[i] = 0;
                }

                for (int i = 0; i < n; i++)
                {
                    if(loadingLibraryStop)
                        return;
                    do
                    {
                        if(loadingLibraryStop)
                            return;
                        index = XMath::rand(0, n);
                    }
                    while (index_arr[index] != 0);
                    index_arr[index] = 1;
                    arr1.push_back(((LibraryListWidgetItem*)libraryList->item(index)));
                }
                while(libraryList->count()>0)
                {
                    if(loadingLibraryStop)
                        return;
                    libraryList->takeItem(0);
                }
                foreach(auto item, arr1)
                {
                    if(loadingLibraryStop)
                        return;
                    libraryList->addItem(item);
                }
                randomizeLibraryButton->show();
                toggleLibraryLoading(false);
            });
        }
    }
    else
    {
        randomizeLibraryButton->hide();
    }
    if(sortMode != LibrarySortMode::NONE)
    {
        LibraryListWidgetItem::setSortMode(sortMode);
        libraryList->sortItems();
    }
}

void MainWindow::on_actionChange_theme_triggered()
{
    QFileInfo selectedThemeInfo(SettingsHandler::getSelectedTheme());
    QString selectedTheme = QFileDialog::getOpenFileName(this, "Choose XTP theme", selectedThemeInfo.absoluteDir().absolutePath(), "CSS Files (*.css)");
    if(!selectedTheme.isEmpty())
    {
        SettingsHandler::setSelectedTheme(selectedTheme);
        loadTheme(selectedTheme);
    }
}

void MainWindow::loadTheme(QString cssFilePath)
{
    QFile file(cssFilePath);
    if(file.exists())
    {
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());
        if(!styleSheet.isEmpty())
            setStyleSheet(styleSheet);
        else
            setStyleSheet("");
    }
    else
        setStyleSheet("");
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
    _syncHandler->setStandAloneLoop(false);
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
            updateMetaData(playingLibraryListItem->getLibraryListItem());
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
        LibraryListItem27 selectedFileListItem = ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();
        playlistName = PlaylistDialog::renamePlaylist(this, selectedFileListItem.nameNoExtension, &ok);
    }

    if(newPlaylist && ok)
    {
        auto items = libraryList->findItems(playlistName, Qt::MatchFixedString);
        if(items.empty())
        {
            SettingsHandler::addNewPlaylist(playlistName);
            _mediaLibraryHandler->setupPlaylistItem(playlistName);
        }
        else
        {
            LogHandler::Dialog(this, (tr("Playlist '") + playlistName + tr("' already exists.\nPlease choose another name.")), XLogLevel::Critical);
        }
    }
    return ok ? playlistName : nullptr;
}

void MainWindow::onSetupPlaylistItem(LibraryListItem27 item)
{
    LibraryListWidgetItem* qListWidgetItem = new LibraryListWidgetItem(item, libraryList);
    libraryList->insertItem(0, qListWidgetItem);
    cachedLibraryItems.push_front(new LibraryListWidgetItem(item));
}

void MainWindow::addSelectedLibraryItemToPlaylist(QString playlistName)
{
    QListWidgetItem* selectedItem = libraryList->selectedItems().first();
    LibraryListItem27 selectedFileListItem = ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();
    SettingsHandler::addToPlaylist(playlistName, selectedFileListItem);
}

void MainWindow::loadPlaylistIntoLibrary(QString playlistName, bool autoPlay)
{
    if(!thumbProcessIsRunning)
    {
        if(!isLibraryLoading())
        {
            toggleLibraryLoading(true);
            onLibraryLoadingStatusChange("Loading playlist...");

            libraryList->clear();
            loadingLibraryFuture = QtConcurrent::run([this, playlistName, autoPlay]() {
                selectedPlaylistName = playlistName;
                auto playlists = SettingsHandler::getPlaylists();
                auto playlist = playlists.value(playlistName);
                foreach(auto item, playlist)
                {
                    _mediaLibraryHandler->setLiveProperties(item);
                    LibraryListWidgetItem* qListWidgetItem = new LibraryListWidgetItem(item, libraryList);
                    libraryList->addItem(qListWidgetItem);
                    selectedPlaylistItems.push_back((LibraryListWidgetItem*)qListWidgetItem->clone());
                }
                backLibraryButton->show();
                editPlaylistButton->show();
                librarySortGroup->setEnabled(false);
                changeLibraryDisplayMode(SettingsHandler::getLibraryView());
                resizeThumbs(SettingsHandler::getThumbSize());
                sortLibraryList(LibrarySortMode::NONE);
                toggleLibraryLoading(false);
                setCurrentLibraryRow(0);
                if(autoPlay)
                {
                    emit stopAndPlayVideo(playlist.first());
                }
            });
        }
    }
    else
        LogHandler::Dialog(this, tr("Please wait for thumbnails to fully load!"), XLogLevel::Warning);
}

void MainWindow::backToMainLibrary()
{
    if(_editPlaylistMode)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("WARNING!"), tr("You are currently editing a playlist. Cancel all changes?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
            cancelEditPlaylist();
        else
            return;
    }
    librarySortGroup->setEnabled(true);
    selectedPlaylistName = nullptr;
    qDeleteAll(selectedPlaylistItems);
    selectedPlaylistItems.clear();
    if(cachedLibraryItems.length() == 0)
        _mediaLibraryHandler->loadLibraryAsync();
    else
    {
        if(!isLibraryLoading())
        {
            //disconnect(libraryList, &QListWidget::itemChanged, 0, 0);
            toggleLibraryLoading(true);
            onLibraryLoadingStatusChange("Loading library...");
            libraryList->clear();
            loadingLibraryFuture = QtConcurrent::run([this]() {
                foreach(auto item, cachedLibraryItems)
                {
                    if(loadingLibraryStop)
                        return;
                    auto name = item->text();
                    libraryList->addItem(item->clone());
                }
                backLibraryButton->hide();
                editPlaylistButton->hide();
                savePlaylistButton->hide();
                cancelEditPlaylistButton->hide();
                libraryList->setDragEnabled(false);
//                changeLibraryDisplayModeAndUpdateThumbSize(SettingsHandler::getLibraryView());
//                resizeThumbs(SettingsHandler::getThumbSize());
                sortLibraryList(SettingsHandler::getLibrarySortMode());
                toggleLibraryLoading(false);
                setCurrentLibraryRow(0);
            });
        }
    }
}

void MainWindow::savePlaylist()
{
    QScroller::grabGesture(libraryList->viewport(), QScroller::LeftMouseButtonGesture);
    QList<LibraryListItem27> libraryItems;
    for(int i=0;i<libraryList->count();i++)
    {
        LibraryListItem27 libraryListItem = ((LibraryListWidgetItem*)libraryList->item(i))->getLibraryListItem();
        if(!libraryListItem.nameNoExtension.isEmpty())
            libraryItems.push_back(libraryListItem);
    }
    SettingsHandler::updatePlaylist(selectedPlaylistName, libraryItems);
    savePlaylistButton->hide();
    editPlaylistButton->show();
    cancelEditPlaylistButton->hide();
    _editPlaylistMode = false;
    changeLibraryDisplayMode(SettingsHandler::getLibraryView());
    resizeThumbs(SettingsHandler::getThumbSize());
}
void MainWindow::editPlaylist()
{
    QScroller::grabGesture(libraryList->viewport(), QScroller::MiddleMouseButtonGesture);
    _editPlaylistMode = true;
    savePlaylistButton->show();
    cancelEditPlaylistButton->show();
    editPlaylistButton->hide();
    if(SettingsHandler::getThumbSize() > 75)
    {
        changeLibraryDisplayMode(LibraryView::List);
        resizeThumbs(75);
    }
    else
    {
        changeLibraryDisplayMode(LibraryView::List);
    }
    libraryList->setDragEnabled(true);
    libraryList->setDragDropMode(QAbstractItemView::InternalMove);
    libraryList->setDefaultDropAction(Qt::MoveAction);
    libraryList->setMovement(QListWidget::Movement::Snap);
    libraryList->setDragDropOverwriteMode(false);
    libraryList->setDropIndicatorShown(true);

}
void MainWindow::cancelEditPlaylist()
{
    _editPlaylistMode = false;
    QScroller::grabGesture(libraryList->viewport(), QScroller::LeftMouseButtonGesture);
    savePlaylistButton->hide();
    cancelEditPlaylistButton->hide();
    editPlaylistButton->show();
    //loadPlaylistIntoLibrary(selectedPlaylistName);
    changeLibraryDisplayMode(SettingsHandler::getLibraryView());
    resizeThumbs(SettingsHandler::getThumbSize());
    libraryList->setDragEnabled(false);
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
        LibraryListItem27 playlist = ((LibraryListWidgetItem*)libraryList->selectedItems().first())->getLibraryListItem();
        if(items.empty())
        {
            auto playlists = SettingsHandler::getPlaylists();
            auto storedPlaylist = playlists.value(playlist.nameNoExtension);
            deleteSelectedPlaylist();
            SettingsHandler::updatePlaylist(renamedPlaylistName, storedPlaylist);
            _mediaLibraryHandler->setupPlaylistItem(renamedPlaylistName);
        }
        else if(playlist.nameNoExtension != renamedPlaylistName)
        {
            LogHandler::Dialog(this, (tr("Playlist '") + renamedPlaylistName + tr("' already exists.\nPlease choose another name.")), XLogLevel::Critical);
        }
    }

}
void MainWindow::deleteSelectedPlaylist()
{
    LibraryListWidgetItem* selectedItem = (LibraryListWidgetItem*)libraryList->selectedItems().first();
    LibraryListItem27 selectedFileListItem = selectedItem->getLibraryListItem();
    SettingsHandler::deletePlaylist(selectedFileListItem.nameNoExtension);
    libraryList->removeItemWidget(selectedItem);
    LibraryListWidgetItem* deleteCache = boolinq::from(cachedLibraryItems).firstOrDefault([selectedItem](const LibraryListWidgetItem* x) { return x->text() == selectedItem->text(); });
    cachedLibraryItems.removeOne(deleteCache);
    delete deleteCache;
    delete selectedItem;
}


LibraryListItem27 MainWindow::getSelectedLibraryListItem()
{
    QListWidgetItem* selectedItem = libraryList->selectedItems().first();
    return ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();
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

void MainWindow::onSetMoneyShot(LibraryListItem27 libraryListItem, qint64 currentPosition)
{
    auto libraryListItemMetaData = SettingsHandler::getLibraryListItemMetaData(libraryListItem.path);
    libraryListItemMetaData.moneyShotMillis = currentPosition;
    SettingsHandler::updateLibraryListItemMetaData(libraryListItemMetaData);
}
void MainWindow::onAddBookmark(LibraryListItem27 libraryListItem, QString name, qint64 currentPosition)
{
    auto libraryListItemMetaData = SettingsHandler::getLibraryListItemMetaData(libraryListItem.path);
    libraryListItemMetaData.bookmarks.append({name, currentPosition});
    SettingsHandler::updateLibraryListItemMetaData(libraryListItemMetaData);
}
void MainWindow::skipToMoneyShot()
{
    _syncHandler->skipToMoneyShot();

    if(!SettingsHandler::getSkipToMoneyShotSkipsVideo())
        return;
    if(videoHandler->isPlaying())
    {
        if(_playerControlsFrame->getAutoLoop())
            _playerControlsFrame->SetLoop(false);
        LibraryListItem27 selectedLibraryListItem27 = playingLibraryListItem->getLibraryListItem();
        auto libraryListItemMetaData = SettingsHandler::getLibraryListItemMetaData(selectedLibraryListItem27.path);
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
}

void MainWindow::skipToActionBegin()
{
    if(_syncHandler->isLoaded())
    {
        if(_playerControlsFrame->getAutoLoop())
            _playerControlsFrame->SetLoop(false);
        qint64 min = _syncHandler->getFunscriptMin();
        if(min > 1500)
        {
            if(videoHandler->isPlaying())
            {
                videoHandler->setPosition(min - 1000);
            }
            else if(_syncHandler->isPlayingStandAlone())
            {
                _syncHandler->setFunscriptTime(min - 1000);
            }
        }
    }
}

void MainWindow::on_actionReload_theme_triggered()
{
    loadTheme(SettingsHandler::getSelectedTheme());
}

void MainWindow::on_actionStored_DLNA_links_triggered()
{
    _dlnaScriptLinksDialog->showDialog();
}
