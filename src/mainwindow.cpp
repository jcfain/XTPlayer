#include "mainwindow.h"
#include "ui_mainwindow.h"
MainWindow::MainWindow(QStringList arguments, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    SettingsHandler::Load();
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

    videoHandler = new VideoHandler(this);
    ui->MediaGrid->addWidget(videoHandler, 0, 0, 3, 5);

    _movie = new QMovie("://images/Eclipse-1s-loading-200px.gif");
    _videoLoadingLabel = new QLabel(this);
    _videoLoadingLabel->setMovie(_movie);
    _videoLoadingLabel->setAttribute(Qt::WA_TransparentForMouseEvents );
    _videoLoadingLabel->setMaximumSize(200,200);
    _videoLoadingLabel->setStyleSheet("* {background: transparent}");
    _videoLoadingLabel->setAlignment(Qt::AlignCenter);
    ui->MediaGrid->addWidget(_videoLoadingLabel, 1, 2);
    setLoading(false);

    ui->SeekSlider->setDisabled(true);
    ui->SeekSlider->SetRange(0, 100);
    ui->SeekSlider->setOption(RangeSlider::Option::RightHandle);
    ui->SeekSlider->setUpperValue(0);

    ui->VolumeSlider->setDisabled(false);
    ui->VolumeSlider->SetRange(0, 30);
    ui->VolumeSlider->setOption(RangeSlider::Option::RightHandle);
    ui->VolumeSlider->setUpperValue(SettingsHandler::getPlayerVolume());
    setVolumeIcon(SettingsHandler::getPlayerVolume());

    libraryList = new QListWidget(this);
    libraryList->setUniformItemSizes(true);
    libraryList->setContextMenuPolicy(Qt::CustomContextMenu);
    libraryList->setProperty("id", "libraryList");
    libraryList->setMovement(QListView::Static);

    QScroller::grabGesture(libraryList->viewport(), QScroller::LeftMouseButtonGesture);
    auto scroller = QScroller::scroller(libraryList->viewport());
    QScrollerProperties scrollerProperties;
    QVariant overshootPolicy = QVariant::fromValue<QScrollerProperties::OvershootPolicy>(QScrollerProperties::OvershootAlwaysOff);
    scrollerProperties.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, overshootPolicy);
    QScroller::scroller(libraryList)->setScrollerProperties(scrollerProperties);
    scroller->setScrollerProperties(scrollerProperties);
    ui->libraryGrid->addWidget(libraryList, 0, 0, 20, 10);

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
    ui->libraryGrid->addWidget(windowedLibraryButton, 0, 1);

    libraryWindow = new LibraryWindow(this);
    libraryWindow->setProperty("id", "libraryWindow");

    randomizeLibraryButton = new QPushButton(this);
    randomizeLibraryButton->setProperty("id", "randomizeLibraryButton");
    QIcon reloadIcon("://images/icons/reload.svg");
    randomizeLibraryButton->setIcon(reloadIcon);
    ui->libraryGrid->addWidget(randomizeLibraryButton, 0, 2);

    savePlaylistButton = new QPushButton(this);
    savePlaylistButton->setProperty("id", "savePlaylistButton");
    savePlaylistButton->setText("SAVE");
    //QIcon reloadIcon("://images/icons/reload.svg");
    //randomizeLibraryButton->setIcon(reloadIcon);
    ui->libraryGrid->addWidget(savePlaylistButton, 0, 3);
    savePlaylistButton->hide();

    ui->libraryFrame->setFrameShadow(QFrame::Sunken);

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
    libraryThumbSizeGroup->addAction(action75_Size);
    libraryThumbSizeGroup->addAction(action100_Size);
    libraryThumbSizeGroup->addAction(action125_Size);
    libraryThumbSizeGroup->addAction(action150_Size);
    libraryThumbSizeGroup->addAction(action175_Size);
    libraryThumbSizeGroup->addAction(action200_Size);

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

    on_load_library(SettingsHandler::getSelectedLibrary());

    auto splitterSizes = SettingsHandler::getMainWindowSplitterPos();
    if (splitterSizes.count() > 0)
        ui->mainFrameSplitter->setSizes(splitterSizes);
    _xSettings = new SettingsDialog(this);
    tcodeHandler = new TCodeHandler();

    connect(ui->mainFrameSplitter, &QSplitter::splitterMoved, this, &MainWindow::on_mainwindow_splitterMove);
    connect(backLibraryButton, &QPushButton::clicked, this, &MainWindow::backToMainLibrary);
    connect(randomizeLibraryButton, &QPushButton::clicked, this, &MainWindow::on_actionRandom_triggered);
    connect(windowedLibraryButton, &QPushButton::clicked, this, &MainWindow::onLibraryWindowed_Clicked);
    connect(savePlaylistButton, &QPushButton::clicked, this, &MainWindow::playListChanged);

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
    connect(actionNameAsc_Sort, &QAction::triggered, this, &MainWindow::on_actionNameAsc_triggered);
    connect(actionNameDesc_Sort, &QAction::triggered, this, &MainWindow::on_actionNameDesc_triggered);
    connect(actionRandom_Sort, &QAction::triggered, this, &MainWindow::on_actionRandom_triggered);
    connect(actionCreatedAsc_Sort, &QAction::triggered, this, &MainWindow::on_actionCreatedAsc_triggered);
    connect(actionCreatedDesc_Sort, &QAction::triggered, this, &MainWindow::on_actionCreatedDesc_triggered);
    connect(actionTypeAsc_Sort, &QAction::triggered, this, &MainWindow::on_actionTypeAsc_triggered);
    connect(actionTypeDesc_Sort, &QAction::triggered, this, &MainWindow::on_actionTypeDesc_triggered);

    connect(videoHandler, &VideoHandler::positionChanged, this, &MainWindow::on_media_positionChanged, Qt::QueuedConnection);
    connect(videoHandler, &VideoHandler::mediaStatusChanged, this, &MainWindow::on_media_statusChanged);
    connect(videoHandler, &VideoHandler::playing, this, &MainWindow::on_media_start);
    connect(videoHandler, &VideoHandler::stopping, this, &MainWindow::on_media_stop);
    connect(videoHandler, &VideoHandler::togglePaused, this, &MainWindow::onVideoHandler_togglePaused);

    connect(ui->SeekSlider, &RangeSlider::upperValueMove, this, &MainWindow::on_seekSlider_sliderMoved);
    connect(ui->SeekSlider, &RangeSlider::onHover, this, &MainWindow::on_seekslider_hover );
    connect(ui->SeekSlider, &RangeSlider::onLeave, this, &MainWindow::on_seekslider_leave );
    connect(ui->VolumeSlider, &RangeSlider::upperValueMove, this, &MainWindow::on_VolumeSlider_valueChanged);
    //connect(player, static_cast<void(AVPlayer::*)(AVPlayer::Error )>(&AVPlayer::error), this, &MainWindow::on_media_error);

    connect(videoHandler, &VideoHandler::doubleClicked, this, &MainWindow::media_double_click_event);
    connect(videoHandler, &VideoHandler::rightClicked, this, &MainWindow::media_single_click_event);
    connect(this, &MainWindow::keyPressed, this, &MainWindow::on_key_press);
    connect(this, &MainWindow::change, this, &MainWindow::on_mainwindow_change);
    //connect(videoHandler, &VideoHandler::mouseEnter, this, &MainWindow::on_video_mouse_enter);

    connect(libraryList, &QListWidget::customContextMenuRequested, this, &MainWindow::onLibraryList_ContextMenuRequested);
    connect(libraryList, &QListWidget::itemDoubleClicked, this, &MainWindow::on_LibraryList_itemDoubleClicked);
    connect(libraryList, &QListWidget::itemClicked, this, &MainWindow::on_LibraryList_itemClicked);


    connect(retryConnectionButton, &QPushButton::clicked, _xSettings, &SettingsDialog::initDeviceRetry);
    connect(deoRetryConnectionButton, &QPushButton::clicked, _xSettings, &SettingsDialog::initDeoRetry);
    connect(QApplication::instance(), &QCoreApplication::aboutToQuit, this, &MainWindow::dispose);

    QFile file(SettingsHandler::getSelectedTheme());
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    setStyleSheet(styleSheet);

    setFocus();
    _defaultAppSize = this->size();

}
MainWindow::~MainWindow()
{

}
void MainWindow::dispose()
{
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
        MainWindow::toggleFullScreen();
    }
    else if(action == actions.Mute)
    {
        MainWindow::on_MuteBtn_toggled(!videoHandler->isMute());
    }
    else if(action == actions.Stop)
    {
        MainWindow::on_StopBtn_clicked();
    }
     else if(action == actions.Next)
    {
        MainWindow::skipForward();
    }
    else if(action == actions.Back)
    {
        MainWindow::skipBack();
    }
    else if(action == actions.VolumeUp)
    {
        int currentVolume = ui->VolumeSlider->GetUpperValue();
        int maxVolume = ui->VolumeSlider->GetMaximum();
        int newVolume = currentVolume - maxVolume <= 5 ? currentVolume + 5 : maxVolume;
        ui->VolumeSlider->setUpperValue(newVolume);
        on_VolumeSlider_valueChanged(newVolume);
    }
    else if(action == actions.VolumeDown)
    {
        int currentVolume = ui->VolumeSlider->GetUpperValue();
        int minVolume = ui->VolumeSlider->GetMinimum();
        int newVolume = currentVolume - minVolume  >= 5 ? currentVolume - 5 : minVolume;
        ui->VolumeSlider->setUpperValue(newVolume);
        on_VolumeSlider_valueChanged(newVolume);
    }
    else if(action == actions.Loop)
    {
        toggleLoop();
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
}

void MainWindow::deviceHome()
{
    _xSettings->getSelectedDeviceHandler()->sendTCode(tcodeHandler->getAllHome());
}

void MainWindow::on_mainwindow_splitterMove(int pos, int index)
{
    SettingsHandler::setMainWindowSplitterPos(ui->mainFrameSplitter->sizes());
}

void MainWindow::onLibraryWindowed_Clicked()
{
    ui->libraryGrid->removeWidget(libraryList);
    ui->libraryGrid->removeWidget(randomizeLibraryButton);
    ui->libraryGrid->removeWidget(windowedLibraryButton);
    ui->libraryGrid->removeWidget(backLibraryButton);
    ((QGridLayout*)libraryWindow->layout())->addWidget(libraryList, 0, 0, 20, 10);
    ((QGridLayout*)libraryWindow->layout())->addWidget(backLibraryButton, 0, 0);
    ((QGridLayout*)libraryWindow->layout())->addWidget(randomizeLibraryButton, 0, 0);
    ((QGridLayout*)libraryWindow->layout())->addWidget(windowedLibraryButton, 0, 2);
    windowedLibraryButton->hide();
    ui->libraryFrame->hide();
    libraryWindow->show();
    if(SettingsHandler::getLibrarySortMode() != LibrarySortMode::RANDOM)
        randomizeLibraryButton->hide();
    else
        randomizeLibraryButton->show();
    if(selectedPlaylistItems.length() > 0)
        backLibraryButton->show();
    else
        backLibraryButton->hide();
}

void MainWindow::onLibraryWindowed_Closed()
{
    libraryWindow->layout()->removeWidget(libraryList);
    ui->libraryGrid->addWidget(libraryList, 0, 0, 20, 10);
    ((QGridLayout*)libraryWindow->layout())->addWidget(backLibraryButton, 0, 0);
    ui->libraryGrid->addWidget(windowedLibraryButton, 0, 1);
    ui->libraryGrid->addWidget(randomizeLibraryButton, 0, 2);
    windowedLibraryButton->show();
    ui->libraryFrame->show();
    if(SettingsHandler::getLibrarySortMode() != LibrarySortMode::RANDOM)
        randomizeLibraryButton->hide();
    else
        randomizeLibraryButton->show();

    if(selectedPlaylistItems.length() > 0)
        backLibraryButton->show();
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

    myMenu.addAction("Play", this, &MainWindow::playFileFromContextMenu);
    if(selectedFileListItem.type == LibraryListItemType::PlaylistInternal)
    {
        myMenu.addAction("Open", this, [this]()
        {
            QListWidgetItem* selectedItem = libraryList->selectedItems().first();
            LibraryListItem selectedFileListItem = ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();
            loadPlaylistIntoLibrary(selectedFileListItem.nameNoExtension);
        });
        myMenu.addAction("Delete...", this, [this]()
        {
            QListWidgetItem* selectedItem = libraryList->selectedItems().first();
            LibraryListItem selectedFileListItem = ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "WARNING!", "Are you sure you want to delete the playlist: " + selectedFileListItem.nameNoExtension,
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes)
            {
                SettingsHandler::deletePlaylist(selectedFileListItem.nameNoExtension);
                libraryList->removeItemWidget(selectedItem);
                cachedLibraryItems.removeOne((LibraryListWidgetItem*)selectedItem);
                delete selectedItem;
            }
        });
    }
    if(selectedFileListItem.type != LibraryListItemType::PlaylistInternal)
    {
        if(selectedPlaylistItems.length() > 0)
        {
            myMenu.addAction("Remove from playlist", this, &MainWindow::removeFromPlaylist);
        }
        myMenu.addAction("Play with funscript...", this, &MainWindow::playFileWithCustomScript);
        if(selectedPlaylistItems.length() == 0)
        {
            QMenu* subMenu = myMenu.addMenu(tr("Add to playlist"));
            subMenu->addAction("New playlist...", this, [this]()
            {
                QString playlist = on_actionNew_playlist_triggered();
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
            myMenu.addAction("Regenerate thumbnail", this, &MainWindow::regenerateThumbNail);
            myMenu.addAction("Set thumbnail from current", this, &MainWindow::setThumbNailFromCurrent);
        }
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
        QString funscriptPath = QFileDialog::getOpenFileName(this, "Choose script for video: " + videoFile.fileName(), SettingsHandler::getSelectedLibrary(), "Script Files (*.funscript)");
        funscriptFileSelectorOpen = false;
        if (!funscriptPath.isEmpty())
        {
            SettingsHandler::setDeoDnlaFunscript(playingPacket.path, funscriptPath);
            funscriptHandler->setLoaded(false);
        }
    }
    else
    {
        LogHandler::Dialog("No packet for current video or no video playing", XLogLevel::Information);
    }
}

void MainWindow::on_load_library(QString path)
{
    if (!path.isNull() && !path.isEmpty())
    {
        QString thumbPath = QCoreApplication::applicationDirPath() + "/thumbs/";
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
                QString thumbFile;
                bool audioOnly = false;
                if(videoPath.contains("m4a"))
                    LogHandler::Debug("ya");
                if(audioTypes.contains(mediaExtension))
                {
                    thumbFile = "://images/icons/audio.png";
                    audioOnly = true;
                }
                else
                    thumbFile = thumbPath + fileName + ".jpg";
                LibraryListItem item
                {
                    audioOnly ? LibraryListItemType::Audio : LibraryListItemType::Video,
                    videoPath, // path
                    fileName, // name
                    fileNameNoExtension, //nameNoExtension
                    scriptPath, // script
                    scriptNoExtension,
                    mediaExtension,
                    thumbFile,
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
           LogHandler::Dialog("Library path '" + path + "' does not exist anymore!", XLogLevel::Critical);
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
void MainWindow::saveThumb(const QString& videoFile, const QString& thumbFile, LibraryListWidgetItem* qListWidgetItem, qint64 position)
{
//    QIcon thumb;
//    QPixmap bgPixmap(QApplication::applicationDirPath() + "/themes/loading.png");
//    QPixmap scaled = bgPixmap.scaled(SettingsHandler::getThumbSize(), Qt::AspectRatioMode::KeepAspectRatio);
//    thumb.addPixmap(scaled);
//    qListWidgetItem->setIcon(thumb);
    LibraryListItem item = qListWidgetItem->getLibraryListItem();
    if(item.type == LibraryListItemType::Audio)
    {
        QIcon thumb;
        QPixmap bgPixmap(item.thumbFile);
        int thumbSize = SettingsHandler::getThumbSize();
        QSize size = {thumbSize, thumbSize};
        QPixmap scaled = bgPixmap.scaled(SettingsHandler::getMaxThumbnailSize(), Qt::AspectRatioMode::KeepAspectRatio);
        thumb.addPixmap(scaled);
        qListWidgetItem->setIcon(thumb);
        qListWidgetItem->setSizeHint({thumbSize, thumbSize-(thumbSize/4)});
        saveNewThumbs();
    }
    else
    {
        connect(extractor,
           &QtAV::VideoFrameExtractor::frameExtracted,
           extractor,
           [this, videoFile, thumbFile, qListWidgetItem](const QtAV::VideoFrame& frame)
            {

               const auto& img = frame.toImage();
               QString thumbFileTemp = thumbFile;

               if (!img.save(thumbFile, nullptr, 15))
               {
                   LogHandler::Debug("Error saving thumbnail: " + thumbFile + " for video: " + videoFile);
                   thumbFileTemp = "://images/icons/error.png";
               }
               QIcon thumb;
               QPixmap bgPixmap(thumbFileTemp);
               int thumbSize = SettingsHandler::getThumbSize();
               QSize size = {thumbSize, thumbSize};
               QPixmap scaled = bgPixmap.scaled(SettingsHandler::getMaxThumbnailSize(), Qt::AspectRatioMode::KeepAspectRatio);
               thumb.addPixmap(scaled);
               qListWidgetItem->setIcon(thumb);
               qListWidgetItem->setSizeHint({thumbSize, thumbSize-(thumbSize/4)});
               saveNewThumbs();
            });
        connect(extractor,
           &QtAV::VideoFrameExtractor::error,
           extractor,
           [this, videoFile, qListWidgetItem](const QString &errorMessage)
            {

               LogHandler::Debug("Error extracting image from: " + videoFile + " Error: " + errorMessage);

               QIcon thumb;
               QPixmap bgPixmap("://images/icons/error.png");
               int thumbSize = SettingsHandler::getThumbSize();
               QSize size = {thumbSize,thumbSize};
               QPixmap scaled = bgPixmap.scaled(SettingsHandler::getMaxThumbnailSize(), Qt::AspectRatioMode::KeepAspectRatio);
               thumb.addPixmap(scaled);
               auto errorMsg = qListWidgetItem->toolTip() + "\n\nError: "+ errorMessage;
               qListWidgetItem->setIcon(thumb);
               qListWidgetItem->setToolTip(errorMsg);
               qListWidgetItem->setSizeHint({thumbSize, thumbSize-(thumbSize/4)});
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
               LogHandler::Debug("Loaded video for thumb duration: "+ QString::number(thumbNailPlayer->duration()));
               qint64 randomPosition = position > 0 ? position : XMath::rand((qint64)1, thumbNailPlayer->duration());
               //LogHandler::Debug("randomPosition: " + QString::number(randomPosition));
               extractor->setPosition(randomPosition);
            });

        connect(thumbNailPlayer,
           &AVPlayer::error,
           thumbNailPlayer,
           [this, qListWidgetItem](QtAV::AVError er)
            {
               LogHandler::Debug("Video load error");
               QIcon thumb;
               QPixmap bgPixmap("://images/icons/error.png");
               int thumbSize = SettingsHandler::getThumbSize();
               QSize size = {thumbSize,thumbSize};
               QPixmap scaled = bgPixmap.scaled(SettingsHandler::getMaxThumbnailSize(), Qt::AspectRatioMode::KeepAspectRatio);
               thumb.addPixmap(scaled);
               auto errorMsg = qListWidgetItem->toolTip() + "\n\nError: "+ er.ffmpegErrorString();
               qListWidgetItem->setIcon(thumb);
               qListWidgetItem->setToolTip(errorMsg);
               qListWidgetItem->setSizeHint({thumbSize, thumbSize-(thumbSize/4)});
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
    QString selectedLibrary = QFileDialog::getExistingDirectory(this, tr("Choose media library"), ".", QFileDialog::ReadOnly);
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
        QIcon icon(playingFile != selectedItem.path ? "://images/icons/play.svg" : "://images/icons/pause.svg" );
        ui->PlayBtn->setIcon(icon);
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
        playVideo(item->data(Qt::UserRole).value<LibraryListItem>());
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
        playVideo(libraryListItem);
    }
    else if(libraryListItem.type == LibraryListItemType::PlaylistInternal)
    {
        loadPlaylistIntoLibrary(libraryListItem.nameNoExtension);
        if(selectedPlaylistItems.length() > 0)
        {
            LibraryListItem libraryListItem = setCurrentLibraryRow(0)->getLibraryListItem();
            playVideo(libraryListItem);
        }
    }
}

void MainWindow::playFileWithCustomScript()
{
    QString selectedScript = QFileDialog::getOpenFileName(this, tr("Choose script"), SettingsHandler::getSelectedLibrary(), tr("Scripts (*.funscript *.zip)"));
    if (selectedScript != Q_NULLPTR)
    {
        LibraryListItem selectedFileListItem = ((LibraryListWidgetItem*)libraryList->selectedItems().first())->getLibraryListItem();
        playVideo(selectedFileListItem, selectedScript);
    }
}

void MainWindow::playVideo(LibraryListItem selectedFileListItem, QString customScript)
{
    QFile file(selectedFileListItem.path);
    if (file.exists())
    {
        //on_media_stop();
        if (videoHandler->file() != selectedFileListItem.path || !customScript.isEmpty())
        {
            ui->loopToggleButton->setChecked(false);
            setLoading(true);
            videoHandler->stop();
            videoHandler->setFile(selectedFileListItem.path);
            videoPreviewWidget->setFile(selectedFileListItem.path);
            videoHandler->load();
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
                   LogHandler::Dialog("Custom zip file missing main funscript.", XLogLevel::Warning);
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
                    LogHandler::Dialog("Zip file missing main funscript.", XLogLevel::Warning);
                }
            }

            if(funscriptHandlers.length() > 0)
            {
                qDeleteAll(funscriptHandlers);
                funscriptHandlers.clear();
            }
            deviceHome();

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
        //QUrl url = QUrl::fromLocalFile(selectedFileListItem.path);
        videoHandler->play();
        playingLibraryListIndex = libraryList->currentRow();
        playingLibraryListItem = (LibraryListWidgetItem*)libraryList->item(playingLibraryListIndex);
        if(!funscriptHandler->isLoaded())
        {
            LogHandler::Dialog("Error loading script " + customScript + "!\nTry right clicking on the video in the list\nand loading with another script.", XLogLevel::Warning);

        }
    }
    else
    {
        LogHandler::Dialog("File '" + selectedFileListItem.path + "' does not exist!", XLogLevel::Critical);
    }
}

void MainWindow::on_mainwindow_change(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        QWindowStateChangeEvent* stateEvent = (QWindowStateChangeEvent*)event;
        if(stateEvent->oldState() == Qt::WindowState::WindowMaximized && !_isFullScreen && !_isMaximized)
        {
            this->resize(_defaultAppSize);
            QMainWindow::move(_appPos.x() < 10 ? 10 : _appPos.x(), _appPos.y() < 10 ? 10 : _appPos.y());
        }
        else if(stateEvent->oldState() == Qt::WindowState::WindowMaximized && !_isFullScreen)
        {
            _isMaximized = false;
        }
    }
}

void MainWindow::toggleFullScreen()
{
    if(!_isFullScreen)
    {
        QScreen *screen = this->window()->windowHandle()->screen();
        QSize screenSize = screen->size();
        _videoSize = videoHandler->size();
        _appSize = this->size();
        _appPos = this->pos();
        _isMaximized = this->isMaximized();
        _isFullScreen = true;
        QMainWindow::setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        ui->MediaGrid->removeWidget(videoHandler);
        ui->MediaGrid->removeWidget(_videoLoadingLabel);
        ui->ControlsGrid->removeWidget(ui->playerControlsFrame);

        placeHolderControlsGrid = new QGridLayout(this);
        placeHolderControlsGrid->setContentsMargins(0,0,0,0);
        placeHolderControlsGrid->setSpacing(0);
        placeHolderControlsGrid->addWidget(ui->playerControlsFrame);

        playerControlsPlaceHolder = new QFrame(this);
        playerControlsPlaceHolder->setLayout(placeHolderControlsGrid);
        playerControlsPlaceHolder->setContentsMargins(0,0,0,0);
        playerControlsPlaceHolder->installEventFilter(this);
        playerControlsPlaceHolder->move(QPoint(0, screenSize.height() - ui->playerControlsFrame->height()));
        playerControlsPlaceHolder->setFixedWidth(screenSize.width());
        playerControlsPlaceHolder->setFixedHeight(ui->playerControlsFrame->height());

        int rows = screenSize.height() / ui->playerControlsFrame->height();
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
            playerLibraryPlaceHolder->setFixedHeight(screenSize.height() - ui->playerControlsFrame->height());
            ui->fullScreenGrid->addWidget(playerLibraryPlaceHolder, 0, 0, rows - 1, 2);
            libraryList->setProperty("cssClass", "fullScreenLibrary");
            ui->playerControlsFrame->style()->unpolish(libraryList);
            ui->playerControlsFrame->style()->polish(libraryList);
            hideLibrary();
        }

        _videoLoadingLabel->raise();
        ui->playerControlsFrame->setProperty("cssClass", "fullScreenControls");
        ui->playerControlsFrame->style()->unpolish(ui->playerControlsFrame);
        ui->playerControlsFrame->style()->polish(ui->playerControlsFrame);
        ui->mainStackedWidget->setCurrentIndex(1);
        QMainWindow::resize(screenSize);
        QMainWindow::centralWidget()->layout()->setMargin(0);
        QMainWindow::showFullScreen();
        videoHandler->layout()->setMargin(0);
        ui->mainStackedWidget->move(QPoint(0, 0));
        //videoHandler->move(QPoint(0, 0));
        //videoHandler->resize(QSize(screenSize.width()+1, screenSize.height()+1));
        hideControls();
        ui->menubar->hide();
        ui->statusbar->hide();
        videoHandler->resize(screenSize);
        QMainWindow::setFocus();
    }
    else
    {
        ui->fullScreenGrid->removeWidget(videoHandler);
        playerControlsPlaceHolder->layout()->removeWidget(ui->playerControlsFrame);
        videoHandler->resize(_videoSize);
        ui->MediaGrid->addWidget(videoHandler, 0, 0, 3, 5);
        ui->MediaGrid->addWidget(_videoLoadingLabel, 1, 2);
        ui->fullScreenGrid->removeWidget(playerControlsPlaceHolder);
        _videoLoadingLabel->raise();
        ui->playerControlsFrame->setWindowFlags(Qt::Widget);
        ui->playerControlsFrame->setMinimumSize(QSize(700, 65));
        ui->playerControlsFrame->setMaximumSize(QSize(16777215, 65));
        ui->ControlsGrid->addWidget(ui->playerControlsFrame);
        ui->playerControlsFrame->setProperty("cssClass", "windowedControls");
        ui->playerControlsFrame->style()->unpolish(ui->playerControlsFrame);
        ui->playerControlsFrame->style()->polish(ui->playerControlsFrame);
        libraryList->setProperty("cssClass", "windowedLibrary");
        ui->playerControlsFrame->style()->unpolish(libraryList);
        ui->playerControlsFrame->style()->polish(libraryList);

        if(libraryOverlay)
        {
            placeHolderLibraryGrid->removeWidget(libraryList);
            ui->fullScreenGrid->removeWidget(playerLibraryPlaceHolder);
            libraryList->setMinimumSize(QSize(0, 0));
            libraryList->setMaximumSize(QSize(16777215, 16777215));
            ui->libraryGrid->addWidget(libraryList, 0, 0, 40, 10);
            windowedLibraryButton->raise();
            randomizeLibraryButton->raise();
            libraryOverlay = false;
            delete placeHolderLibraryGrid;
            delete playerLibraryPlaceHolder;
        }

        videoHandler->layout()->setMargin(9);
        QMainWindow::centralWidget()->layout()->setMargin(9);

        ui->mainStackedWidget->setCurrentIndex(0);

        ui->menubar->show();
        ui->statusbar->show();
        ui->playerControlsFrame->show();
        libraryList->show();
        QMainWindow::setWindowFlags(Qt::WindowFlags());
        if(_isMaximized)
        {
            QMainWindow::showMaximized();
        }
        else
        {
            QMainWindow::resize(_appSize);
            QMainWindow::move(_appPos.x() < 100 ? 100 : _appPos.x(), _appPos.y() < 100 ? 100 : _appPos.y());
            QMainWindow::showNormal();
        }
        _isFullScreen = false;
        delete placeHolderControlsGrid;
        delete playerControlsPlaceHolder;
    }
}

void MainWindow::toggleLoop()
{
    if(!ui->loopToggleButton->isChecked() && !autoLoopOn)
    {
        autoLoopOn = true;
        ui->loopToggleButton->setChecked(true);
        qint64 currentVideoPositionPercentage = XMath::mapRange(videoHandler->position(),  (qint64)0, videoHandler->duration(), (qint64)0, (qint64)100);
        ui->SeekSlider->setLowerValue(currentVideoPositionPercentage);
    }
    else if (ui->loopToggleButton->isChecked() && autoLoopOn)
    {
        autoLoopOn = false;
        int lowerValue = ui->SeekSlider->GetLowerValue();
        qint64 currentVideoPositionPercentage = XMath::mapRange(videoHandler->position(),  (qint64)0, videoHandler->duration(), (qint64)0, (qint64)100);
        ui->SeekSlider->setUpperValue(currentVideoPositionPercentage > lowerValue + ui->SeekSlider->GetMinimumRange()
                                      ? currentVideoPositionPercentage : currentVideoPositionPercentage + lowerValue + ui->SeekSlider->GetMinimumRange());
    }
    else
    {
        ui->loopToggleButton->setChecked(false);
    }
}

void MainWindow::hideControls()
{
    if (_isFullScreen)
    {
        ui->playerControlsFrame->hide();
    }
}

void MainWindow::showControls()
{
    if (_isFullScreen)
    {
        ui->playerControlsFrame->show();
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
    if(!videoHandler->isMute())
    {
        videoHandler->setVolume(value);
        SettingsHandler::setPlayerVolume(value);
    }
    setVolumeIcon(value);
    ui->VolumeSlider->setToolTip(QString::number(value));
}

void MainWindow::on_PlayBtn_clicked()
{
    if (libraryList->count() > 0)
    {
        if(libraryList->selectedItems().length() == 0)
        {
            LibraryListWidgetItem* selectedItem = setCurrentLibraryRow(0);
            playVideo(selectedItem->getLibraryListItem());
            return;
        }
        LibraryListWidgetItem* selectedItem = (LibraryListWidgetItem*)libraryList->selectedItems().first();
        LibraryListItem selectedFileListItem = selectedItem->getLibraryListItem();
        if(selectedFileListItem.path != videoHandler->file() || !videoHandler->isPlaying())
        {
            playVideo(selectedFileListItem);
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

void MainWindow::on_StopBtn_clicked()
{
    if(videoHandler->isPlaying())
    {
        videoHandler->stop();
    }
    QIcon icon("://images/icons/play.svg");
    ui->PlayBtn->setIcon(icon);
}

void MainWindow::onVideoHandler_togglePaused(bool paused)
{
    QIcon icon(paused ? "://images/icons/play.svg" : "://images/icons/pause.svg" );
    ui->PlayBtn->setIcon(icon);
}

void MainWindow::on_MuteBtn_toggled(bool checked)
{
    if (checked)
    {
        voulumeBeforeMute = ui->VolumeSlider->GetUpperValue();
        setVolumeIcon(0);
        ui->VolumeSlider->setUpperValue(0);
    }
    else
    {
        setVolumeIcon(voulumeBeforeMute);
        ui->VolumeSlider->setUpperValue(voulumeBeforeMute);
    }
    videoHandler->toggleMute();
}

void MainWindow::setVolumeIcon(int volume)
{
    if (volume > 15)
        ui->MuteBtn->setIcon(QIcon(":/images/icons/speakerLoud.svg"));
    else if (volume > 0)
        ui->MuteBtn->setIcon(QIcon(":/images/icons/speakerMid.svg"));
    else
        ui->MuteBtn->setIcon(QIcon(":/images/icons/speakerMute.svg"));
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
            gpos = mapToGlobal(playerControlsPlaceHolder->pos() + ui->SeekSlider->pos() + QPoint(position, 0));
            QToolTip::showText(gpos, QTime(0, 0, 0).addMSecs(sliderValueTime).toString(QString::fromLatin1("HH:mm:ss")));
        }
        else
        {
            auto tootipPos = mapToGlobal(QPoint(ui->medialAndControlsFrame->pos().x(), 0) + ui->controlsHomePlaceHolder->pos() + ui->SeekSlider->pos() + QPoint(position, 0));
            QToolTip::showText(tootipPos, QTime(0, 0, 0).addMSecs(sliderValueTime).toString(QString::fromLatin1("HH:mm:ss")));
            gpos = QPoint(ui->medialAndControlsFrame->pos().x(), 0) + ui->controlsHomePlaceHolder->pos() + ui->SeekSlider->pos() + QPoint(position, 0);
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
    if (!ui->loopToggleButton->isChecked())
    {
        qint64 playerPosition = XMath::mapRange(static_cast<qint64>(position), (qint64)0, (qint64)100, (qint64)0, videoHandler->duration());
        ui->SeekSlider->setToolTip(QTime(0, 0, 0).addMSecs(position).toString(QString::fromLatin1("HH:mm:ss")));
        videoHandler->setPosition(playerPosition);
    }
}


void MainWindow::on_SeekSlider_valueChanged(int position)
{

}

void MainWindow::onLoopRange_valueChanged(int position)
{
    int endLoop = ui->SeekSlider->GetUpperValue();
    int startLoop = ui->SeekSlider->GetLowerValue();
    qint64 duration = videoHandler->duration();

    qint64 currentVideoPositionPercentage = XMath::mapRange(videoHandler->position(),  (qint64)0, duration, (qint64)0, (qint64)100);
    qint64 destinationVideoPosition = XMath::mapRange((qint64)position, (qint64)0, (qint64)100,  (qint64)0, duration);

    QString timeCurrent = mSecondFormat(destinationVideoPosition);
    ui->SeekSlider->setToolTip(timeCurrent);

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
    if (!ui->loopToggleButton->isChecked())
    {
        if (duration > 0)
        {
            ui->SeekSlider->setUpperValue(static_cast<int>(videoToSliderPosition));
        }
    }
    else
    {
        int endLoop = ui->SeekSlider->GetUpperValue();
        qint64 endLoopToVideoPosition = XMath::mapRange((qint64)endLoop, (qint64)0, (qint64)100,  (qint64)0, duration);
        if (position >= endLoopToVideoPosition)
        {
            int startLoop = ui->SeekSlider->GetLowerValue();
            qint64 startLoopVideoPosition = XMath::mapRange((qint64)startLoop, (qint64)0, (qint64)100,  (qint64)0, duration);
            if(startLoopVideoPosition <= 0)
                startLoopVideoPosition = 50;
            if (videoHandler->position() != startLoopVideoPosition)
                videoHandler->setPosition(startLoopVideoPosition);
        }
        QPoint gpos;
        qint64 videoToSliderPosition = XMath::mapRange(position,  (qint64)0, duration, (qint64)0, (qint64)100);
        int hoverposition = XMath::mapRange((int)videoToSliderPosition,  (int)0, (int)100, (int)0, (int)ui->SeekSlider->width()) - 15;
        if(_isFullScreen)
        {
            gpos = mapToGlobal(playerControlsPlaceHolder->pos() + ui->SeekSlider->pos() + QPoint(hoverposition, 0));
            QToolTip::showText(gpos, QTime(0, 0, 0).addMSecs(position).toString(QString::fromLatin1("HH:mm:ss")), this);
        }
        else
        {
            auto tootipPos = mapToGlobal(QPoint(ui->medialAndControlsFrame->pos().x(), 0) + ui->controlsHomePlaceHolder->pos() + ui->SeekSlider->pos() + QPoint(hoverposition, 0));
            QToolTip::showText(tootipPos, QTime(0, 0, 0).addMSecs(position).toString(QString::fromLatin1("HH:mm:ss")), this);
            gpos = QPoint(ui->medialAndControlsFrame->pos().x(), 0) + ui->controlsHomePlaceHolder->pos() + ui->SeekSlider->pos() + QPoint(hoverposition, 0);
        }
    }
    ui->lblCurrentDuration->setText(mSecondFormat(position).append("/").append(mSecondFormat(duration)));
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
    toggleMediaControlStatus(true);
}

void MainWindow::on_media_stop()
{
    LogHandler::Debug("Enter on_media_stop");
    toggleMediaControlStatus(false);
    if(funscriptFuture.isRunning())
    {
        funscriptFuture.cancel();
    }

}
void MainWindow::setLoading(bool loading)
{
    if(loading)
    {
        _videoLoadingLabel->show();
        _movie->start();
    }
    else
    {
        _videoLoadingLabel->hide();
        _movie->stop();
    }
}
void MainWindow::toggleMediaControlStatus(bool playing)
{
    LogHandler::Debug("Enter toggleMediaControlStatus: "+QString::number(playing));
    if(playing)
    {
        setLoading(false);
        ui->SeekSlider->setUpperValue(0);
        ui->SeekSlider->setDisabled(false);
        QIcon icon("://images/icons/pause.svg" );
        ui->PlayBtn->setIcon(icon);
    }
    else
    {
        setLoading(false);
        ui->SeekSlider->setUpperValue(0);
        ui->SeekSlider->setDisabled(true);
        QIcon icon("://images/icons/play.svg" );
        ui->PlayBtn->setIcon(icon);
        ui->lblCurrentDuration->setText("00:00:00/00:00:00");
    }
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
            if (funscriptPath == nullptr)
            {
                funscriptFileSelectorOpen = true;
                if (!SettingsHandler::getDisableSpeechToText())
                    textToSpeech->say("Script for video playing in Deeo VR not found. Please check your computer to select a script.");
                funscriptPath = QFileDialog::getOpenFileName(this, "Choose script for video: " + videoFile.fileName(), SettingsHandler::getSelectedLibrary(), "Script Files (*.funscript)");
                funscriptFileSelectorOpen = false;
                //LogHandler::Debug("funscriptPath: "+funscriptPath);
                if(funscriptPath.isEmpty())
                {
                    vrScriptSelectorCanceled = true;
                    vrScriptSelectedCanceledPath = packet.path;
                }

            }
            //Store the location of the file so the above check doesnt happen again.
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
        else if(xSettings->isConnected() && currentVRPacket.duration > 0 && currentVRPacket.playing)
        {
            if (!currentVRPacket.path.isEmpty() && !currentVRPacket.path.isNull())
            {
                QString funscriptPath = SettingsHandler::getDeoDnlaFunscript(currentVRPacket.path);
                currentVideo = currentVRPacket.path;
                funscriptHandler->load(funscriptPath);

                qDeleteAll(funscriptHandlers);
                funscriptHandlers.clear();
                xSettings->getSelectedDeviceHandler()->sendTCode(tcodeHandler->getRunningHome());

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
        if(funscriptHandler->isLoaded() && (videoHandler->isPlaying() || _xSettings->getDeoHandler()->isPlaying()))
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
        if (!ui->loopToggleButton->isChecked())
            skipForward();
    break;
    case NoMedia:
        //status = tr("No media");
        break;
    case InvalidMedia:
        //status = tr("Invalid meida");
        break;
    case BufferingMedia:
        setLoading(true);
        break;
    case BufferedMedia:
        setLoading(false);
        break;
    case LoadingMedia:
        setLoading(true);
        break;
    case LoadedMedia:
        setLoading(false);
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

        playVideo(item->getLibraryListItem());
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

        playVideo(item->getLibraryListItem());
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
    libraryList->setResizeMode(QListView::Adjust);
    libraryList->setFlow(QListView::LeftToRight);
    libraryList->setViewMode(QListView::IconMode);
    libraryList->setTextElideMode(Qt::ElideMiddle);
    libraryList->setSpacing(2);
    if(selectedPlaylistItems.length() > 0)
    {
        libraryList->setDragEnabled(true);
        libraryList->setDragDropMode(QAbstractItemView::DragDrop);
        libraryList->setDefaultDropAction(Qt::MoveAction);
        libraryList->setMovement(QListView::Movement::Snap);
    }
    updateThumbSizeUI(SettingsHandler::getThumbSize());
}

void MainWindow::on_actionList_triggered()
{
    SettingsHandler::setLibraryView(LibraryView::List);
    libraryList->setResizeMode(QListView::Fixed);
    libraryList->setFlow(QListView::TopToBottom);
    libraryList->setViewMode(QListView::ListMode);
    libraryList->setTextElideMode(Qt::ElideRight);
    libraryList->setSpacing(0);
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

void MainWindow::setThumbSize(int size)
{
    SettingsHandler::setThumbSize(size);
    for(int i = 0; i < libraryList->count(); i++)
    {
        libraryList->item(i)->setSizeHint({size, size-(size/4)});
    }
    libraryList->setIconSize({size, size});
//    if(SettingsHandler::getLibraryView() == LibraryView::List)
//        libraryList->setViewMode(QListView::ListMode);
//    else
//        libraryList->setViewMode(QListView::IconMode);

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

    if(playingLibraryListItem != nullptr)
        libraryList->setCurrentItem(playingLibraryListItem);
    else if(selectedLibraryListItem != nullptr)
        libraryList->setCurrentItem(selectedLibraryListItem);
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
    LibraryListWidgetItem::setSortMode(LibrarySortMode::RANDOM);
    SettingsHandler::setLibrarySortMode(LibrarySortMode::RANDOM);

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
        ui->SeekSlider->setOption(RangeSlider::Option::DoubleHandles);
        ui->SeekSlider->SetRange(0, 100);
        connect(ui->SeekSlider, QOverload<int>::of(&RangeSlider::lowerValueChanged), this, &MainWindow::onLoopRange_valueChanged);
        connect(ui->SeekSlider, QOverload<int>::of(&RangeSlider::upperValueChanged), this, &MainWindow::onLoopRange_valueChanged);
        videoHandler->setRepeat(-1);
        ui->SeekSlider->updateColor();
        qint64 videoToSliderPosition = XMath::mapRange(videoHandler->position(),  (qint64)0, videoHandler->duration(), (qint64)0, (qint64)100);
        ui->SeekSlider->setLowerValue(videoToSliderPosition);
    }
    else
    {
        ui->SeekSlider->setOption(RangeSlider::Option::RightHandle);
        ui->SeekSlider->SetRange(0, 100);
        on_media_positionChanged(videoHandler->position());
        ui->SeekSlider->updateColor();
        disconnect(ui->SeekSlider, QOverload<int>::of(&RangeSlider::lowerValueChanged), this, &MainWindow::onLoopRange_valueChanged);
        disconnect(ui->SeekSlider, QOverload<int>::of(&RangeSlider::upperValueChanged), this, &MainWindow::onLoopRange_valueChanged);
        qint64 position = videoHandler->position();
        videoHandler->setRepeat();
        videoHandler->setPosition(position);
    }
}

QString MainWindow::on_actionNew_playlist_triggered()
{
    bool ok;
    QString playlistName = AddPlaylistDialog::getNewPlaylist(this, &ok);
    if(ok)
    {
        SettingsHandler::addNewPlaylist(playlistName);
        setupPlaylistItem(playlistName);
    }
    return playlistName;
}

void MainWindow::setupPlaylistItem(QString playlistName)
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
        "://images/icons/playlist.png",
        nullptr,
        QDate::currentDate(),
        0
    };
    LibraryListWidgetItem* qListWidgetItem = new LibraryListWidgetItem(item);
    libraryList->insertItem(0, qListWidgetItem);
    cachedLibraryItems.push_front((LibraryListWidgetItem*)qListWidgetItem->clone());
}

void MainWindow::addSelectedLibraryItemToPlaylist(QString playlistName)
{
    LibraryListWidgetItem* qListWidgetItem = (LibraryListWidgetItem*)(libraryList->findItems(playlistName, Qt::MatchExactly).first());
    QListWidgetItem* selectedItem = libraryList->selectedItems().first();
    LibraryListItem selectedFileListItem = ((LibraryListWidgetItem*)selectedItem)->getLibraryListItem();
    SettingsHandler::addToPlaylist(playlistName, selectedFileListItem);
    qListWidgetItem->updateToolTip();
}

void MainWindow::backToMainLibrary()
{
    selectedPlaylistName = nullptr;
    selectedPlaylistItems.clear();
    if(cachedLibraryItems.length() == 0)
        on_load_library(SettingsHandler::getSelectedLibrary());
    else
    {
        libraryList->clear();
        foreach(auto item, cachedLibraryItems)
        {
            libraryList->addItem((LibraryListWidgetItem*)item->clone());
        }
    }
    backLibraryButton->hide();
    libraryList->setDragEnabled(false);
    disconnect(libraryList->model(), &QAbstractItemModel::dataChanged, this, &MainWindow::playListChanged);
}

void MainWindow::loadPlaylistIntoLibrary(QString playlistName)
{
//    if(!thumbProcessIsRunning)
//    {
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
        libraryList->setDragEnabled(true);
        libraryList->setDragDropMode(QAbstractItemView::InternalMove);
        libraryList->setDefaultDropAction(Qt::MoveAction);
        libraryList->setMovement(QListWidget::Movement::Snap);
        libraryList->setDragDropOverwriteMode(false);
        libraryList->setDropIndicatorShown(true);
        connect(libraryList->model(), &QAbstractItemModel::dataChanged, this, [this]() {
            savePlaylistButton->show();
        });
//    }
//    else
//        LogHandler::Dialog("Please wait for thumbnails to fully load!", XLogLevel::Warning);
}

void MainWindow::playListChanged()
{
    QList<LibraryListItem> libraryItems;
    for(int i=0;i<libraryList->count();i++)
    {
        LibraryListItem libraryListItem = ((LibraryListWidgetItem*)libraryList->item(i))->getLibraryListItem();
        if(!libraryListItem.nameNoExtension.isEmpty())
            libraryItems.push_back(libraryListItem);
    }
    SettingsHandler::updatePlaylist(selectedPlaylistName, libraryItems);
    savePlaylistButton->hide();
}

void MainWindow::removeFromPlaylist()
{
    QListWidgetItem* selectedItem = libraryList->selectedItems().first();
    delete selectedItem;
    savePlaylistButton->show();
}
