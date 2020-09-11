#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    SettingsHandler::Load();

    QFile file(SettingsHandler::getSelectedTheme());
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    setStyleSheet(styleSheet);

    deoConnectionStatusLabel = new QLabel(this);
    deoRetryConnectionButton = new QPushButton(this);
    deoRetryConnectionButton->hide();
    deoRetryConnectionButton->setText("DeoVR Retry");
    ui->statusbar->addPermanentWidget(deoConnectionStatusLabel);
    ui->statusbar->addPermanentWidget(deoRetryConnectionButton);

    connectionStatusLabel = new QLabel(this);
    retryConnectionButton = new QPushButton(this);
    retryConnectionButton->hide();
    retryConnectionButton->setText("TCode Retry");
    ui->statusbar->addPermanentWidget(connectionStatusLabel);
    ui->statusbar->addPermanentWidget(retryConnectionButton);

    videoHandler = new VideoHandler(this);
    ui->MediaGrid->addWidget(videoHandler);

    ui->videoLoadingLabel->hide();
    movie = new QMovie(QApplication::applicationDirPath() + "/themes/loading.gif");
    ui->videoLoadingLabel->setMovie(movie);

    ui->SeekSlider->setDisabled(true);
    ui->SeekSlider->SetRange(0, 100);
    ui->SeekSlider->setOption(RangeSlider::Option::RightHandle);
    ui->SeekSlider->setUpperValue(0);

    ui->VolumeSlider->setDisabled(false);
    ui->VolumeSlider->SetRange(0, 30);
    ui->VolumeSlider->setOption(RangeSlider::Option::RightHandle);
    ui->VolumeSlider->setUpperValue(SettingsHandler::getPlayerVolume());
    setVolumeIcon(SettingsHandler::getPlayerVolume());

    ui->LibraryList->setUniformItemSizes(true);
    ui->LibraryList->setContextMenuPolicy(Qt::CustomContextMenu);

    thumbCaptureTime = 35000;
    currentMaxThumbSize = {175,175};
    libraryViewGroup = new QActionGroup(this);
    libraryViewGroup->addAction(ui->actionList);
    libraryViewGroup->addAction(ui->actionThumbnail);

    videoPreviewWidget = new VideoPreviewWidget(this);
    videoPreviewWidget->resize(160, 90);
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
    libraryThumbSizeGroup->addAction(action75_Size);
    libraryThumbSizeGroup->addAction(action100_Size);
    libraryThumbSizeGroup->addAction(action125_Size);
    libraryThumbSizeGroup->addAction(action150_Size);
    libraryThumbSizeGroup->addAction(action175_Size);
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

    _xSettings = new SettingsDialog(this);
    _xSettings->init(videoHandler);

    connect(_xSettings, &SettingsDialog::deoDeviceConnectionChange, this, &MainWindow::on_deo_device_connectionChanged);
    connect(_xSettings, &SettingsDialog::deoDeviceError, this, &MainWindow::on_deo_device_error);
    connect(_xSettings, &SettingsDialog::deviceConnectionChange, this, &MainWindow::on_device_connectionChanged);
    connect(_xSettings, &SettingsDialog::deviceError, this, &MainWindow::on_device_error);
    connect(_xSettings->getDeoHandler(), &DeoHandler::messageRecieved, this, &MainWindow::onDeoMessageRecieved);

    connect(action75_Size, &QAction::triggered, this, &MainWindow::on_action75_triggered);
    connect(action100_Size, &QAction::triggered, this, &MainWindow::on_action100_triggered);
    connect(action125_Size, &QAction::triggered, this, &MainWindow::on_action125_triggered);
    connect(action150_Size, &QAction::triggered, this, &MainWindow::on_action150_triggered);
    connect(action175_Size, &QAction::triggered, this, &MainWindow::on_action175_triggered);

    connect(videoHandler, &VideoHandler::positionChanged, this, &MainWindow::on_media_positionChanged);
    connect(videoHandler, &VideoHandler::mediaStatusChanged, this, &MainWindow::on_media_statusChanged);
    connect(videoHandler, &VideoHandler::started, this, &MainWindow::on_media_start);
    connect(videoHandler, &VideoHandler::stopped, this, &MainWindow::on_media_stop);

    connect(ui->SeekSlider, &RangeSlider::upperValueMove, this, &MainWindow::on_seekSlider_sliderMoved);
    connect(ui->SeekSlider, &RangeSlider::onHover, this, &MainWindow::on_seekslider_hover );
    connect(ui->SeekSlider, &RangeSlider::onLeave, this, &MainWindow::on_seekslider_leave );
    connect(ui->VolumeSlider, &RangeSlider::upperValueMove, this, &MainWindow::on_VolumeSlider_valueChanged);
    //connect(player, static_cast<void(AVPlayer::*)(AVPlayer::Error )>(&AVPlayer::error), this, &MainWindow::on_media_error);

    connect(videoHandler, &VideoHandler::doubleClicked, this, &MainWindow::media_double_click_event);
    //connect(vw, &XVideoWidget::singleClicked, this, &MainWindow::media_single_click_event);
    connect(this, &MainWindow::keyPressed, this, &MainWindow::on_key_press);
    //connect(videoHandler, &VideoHandler::mouseEnter, this, &MainWindow::on_video_mouse_enter);

    connect(ui->LibraryList, &QListWidget::customContextMenuRequested, this, &MainWindow::onLibraryList_ContextMenuRequested);


    connect(retryConnectionButton, &QPushButton::clicked, _xSettings, &SettingsDialog::initDeviceRetry);
    connect(deoRetryConnectionButton, &QPushButton::clicked, _xSettings, &SettingsDialog::initDeoRetry);
    connect(QApplication::instance(), &QCoreApplication::aboutToQuit, this, &MainWindow::dispose);



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
    else
    {
        // pass the event on to the parent class
        return QWidget::eventFilter(obj, event);
    }
}

void MainWindow::on_key_press(QKeyEvent * event)
{
    switch(event->key())
    {
        case Qt::Key_Space:
        case Qt::Key_MediaPause:
            MainWindow::togglePause();
            break;
        case Qt::Key_F11:
        case Qt::Key_Escape:
            MainWindow::toggleFullScreen();
            break;
        case Qt::Key_M:
            MainWindow::on_MuteBtn_toggled(!videoHandler->isMute());
            break;
        case Qt::Key_MediaStop:
        case Qt::Key_MediaTogglePlayPause:
            MainWindow::on_media_stop();
            break;
        case Qt::Key_MediaNext:
        case Qt::Key_Right:
            MainWindow::skipForward();
            break;
        case Qt::Key_MediaPrevious:
        case Qt::Key_Left:
            MainWindow::skipBack();
            break;
    }
}

void MainWindow::onLibraryList_ContextMenuRequested(const QPoint &pos)
{
    // Handle global position
    QPoint globalPos = ui->LibraryList->mapToGlobal(pos);

    // Create menu and insert some actions
    QMenu myMenu;

    myMenu.addAction("Play", this, &MainWindow::playFileFromContextMenu);
    myMenu.addAction("Play with funscript...", this, &MainWindow::playFileWithCustomScript);
    myMenu.addAction("Regenerate thumbnail", this, &MainWindow::regenerateThumbNail);
    myMenu.addAction("Set thumbnail from current", this, &MainWindow::setThumbNailFromCurrent);

    LibraryListItem selectedItem = getLibraryListItemFromQListItem(ui->LibraryList->selectedItems()[0]);
    QString funscriptPath = SettingsHandler::getDeoDnlaFunscript(selectedItem.path);
    if (funscriptPath != nullptr)
    {
        myMenu.addAction("Change DeoVR funscript...", this, &MainWindow::changeDeoFunscript);
    }

    // Show context menu at handling position
    myMenu.exec(globalPos);
}

void MainWindow::changeDeoFunscript()
{
    LibraryListItem selectedItem = getLibraryListItemFromQListItem(ui->LibraryList->selectedItems()[0]);
    QFileInfo videoFile(selectedItem.path);
    funscriptFileSelectorOpen = true;
    QString funscriptPath = QFileDialog::getOpenFileName(this, "Choose script for video: " + videoFile.fileName(), SettingsHandler::getSelectedLibrary(), "Script Files (*.funscript)");
    funscriptFileSelectorOpen = false;
    if (funscriptPath != nullptr)
        //Store the location of the file so the above check doesnt happen again.
        SettingsHandler::setDeoDnlaFunscript(selectedItem.path, funscriptPath);
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
            ui->LibraryList->clear();
            libraryItems.clear();
            QDirIterator library(path, QStringList()
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
                        << "*.avchd", QDir::Files, QDirIterator::Subdirectories);
            while (library.hasNext())
            {
                QFileInfo fileinfo(library.next());
                QString videoPath = fileinfo.filePath();
                QString videoPathTemp = fileinfo.filePath();
                QString fileName = fileinfo.fileName();
                QString fileNameTemp = fileinfo.fileName();
                QString scriptFile = fileNameTemp.remove(fileNameTemp.lastIndexOf('.'), fileNameTemp.length() -  1) + ".funscript";
                QString scriptPath;
                if (SettingsHandler::getSelectedFunscriptLibrary() == Q_NULLPTR)
                {
                    scriptPath = videoPathTemp.remove(videoPathTemp.lastIndexOf('.'), videoPathTemp.length() -  1) + ".funscript";
                }
                else
                {
                    scriptPath = SettingsHandler::getSelectedFunscriptLibrary() + QDir::separator() + scriptFile;
                }
                if (!funscriptHandler->exists(scriptPath))
                {
                    scriptPath = nullptr;
                }
                QString thumbFile =  thumbPath + fileName + ".jpg";
                LibraryListItem item
                {
                    videoPath, // path
                    fileName, // name
                    scriptPath, // script
                    thumbFile
                };
                QVariant listItem;
                listItem.setValue(item);
                QListWidgetItem* qListWidgetItem = new QListWidgetItem;
                if (!funscriptHandler->exists(scriptPath))
                {
                    qListWidgetItem->setToolTip(videoPath + "\nNo script file of the same name found.\nRight click and Play with funscript.");
                    qListWidgetItem->setForeground(QColorConstants::Gray);
                }
                else
                {
                    qListWidgetItem->setToolTip(videoPath);
                }

                QFileInfo thumbInfo(thumbFile);
                QString thumbString = thumbFile;
                if (!thumbInfo.exists())
                {
                    thumbString = QApplication::applicationDirPath() + "/themes/loading.png";
                }
                QIcon thumb;
                QPixmap bgPixmap(thumbString);
                QPixmap scaled = bgPixmap.scaled(currentMaxThumbSize, Qt::AspectRatioMode::KeepAspectRatio);
                thumb.addPixmap(scaled);
                qListWidgetItem->setIcon(thumb);

                qListWidgetItem->setText(fileinfo.fileName());
                qListWidgetItem->setData(Qt::UserRole, listItem);
                ui->LibraryList->addItem(qListWidgetItem);
                libraryItems.push_back(qListWidgetItem);
            }
            saveNewThumbs();
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
int thumbNailSearchIterator = 0;
void MainWindow::saveNewThumbs()
{
    if (thumbNailSearchIterator < libraryItems.count())
    {
        QListWidgetItem* listWidgetItem = libraryItems.at(thumbNailSearchIterator);
        LibraryListItem item = getLibraryListItemFromQListItem(listWidgetItem);
        thumbNailSearchIterator++;
        QFileInfo thumbInfo(item.thumbFile);
        if (!thumbInfo.exists())
        {
            saveThumb(item.path, item.thumbFile, listWidgetItem);
        }
        else
        {
            saveNewThumbs();
        }
    }
    else
    {
        thumbNailSearchIterator = 0;
    }


}
void MainWindow::saveThumb(const QString& videoFile, const QString& thumbFile, QListWidgetItem* qListWidgetItem, qint64 position)
{
    QIcon thumb;
    QPixmap bgPixmap(QApplication::applicationDirPath() + "/themes/loading.png");
    QPixmap scaled = bgPixmap.scaled(currentMaxThumbSize, Qt::AspectRatioMode::KeepAspectRatio);
    thumb.addPixmap(scaled);
    qListWidgetItem->setIcon(thumb);
    VideoFrameExtractor* extractor = new VideoFrameExtractor;

    connect(extractor,
           &QtAV::VideoFrameExtractor::frameExtracted,
           extractor,
           [this, extractor, videoFile, thumbFile, qListWidgetItem](const QtAV::VideoFrame& frame)
            {

               const auto& img = frame.toImage();
               QString thumbFileTemp = thumbFile;

               if (!img.save(thumbFile, nullptr, 15))
               {
                   //LogHandler::Debug("Error saving thumbnail: " + thumbFile + " for video: " + videoFile);
                   thumbFileTemp = "://images/icons/error.png";
               }
               QIcon thumb;
               QPixmap bgPixmap(thumbFileTemp);
               QPixmap scaled = bgPixmap.scaled(currentMaxThumbSize, Qt::AspectRatioMode::KeepAspectRatio);
               thumb.addPixmap(scaled);
               qListWidgetItem->setIcon(thumb);
               if(thumbNailSearchIterator > 0)
               {
                   saveNewThumbs();
               }
               extractor->deleteLater();
            });
    connect(extractor,
           &QtAV::VideoFrameExtractor::error,
           extractor,
           [this, extractor, videoFile, qListWidgetItem](const QString &errorMessage)
            {

               //LogHandler::Debug("Error accessing video file: " + videoFile + " Error: " + errorMessage);

               QIcon thumb;
               QPixmap bgPixmap("://images/icons/error.png");
               QPixmap scaled = bgPixmap.scaled(currentMaxThumbSize);
               thumb.addPixmap(scaled);
               qListWidgetItem->setIcon(thumb);
               if(thumbNailSearchIterator > 0)
               {
                   saveNewThumbs();
               }
               extractor->deleteLater();
            });

    extractor->setAsync(true);
    extractor->setSource(videoFile);

    AVPlayer* thumbNailPlayer = new AVPlayer;

    thumbNailPlayer->setAsyncLoad(true);
    thumbNailPlayer->setFile(videoFile);
    thumbNailPlayer->load();
    connect(thumbNailPlayer,
           &AVPlayer::loaded,
           thumbNailPlayer,
           [thumbNailPlayer, extractor, position]()
            {
               //LogHandler::Debug("Loaded video for thumb duration: "+ QString::number(thumbNailPlayer->duration()));
               qint64 randomPosition = position > 0 ? position : XMath::rand(1, thumbNailPlayer->duration());
               //LogHandler::Debug("randomPosition: " + QString::number(randomPosition));
               extractor->setPosition(randomPosition);
               thumbNailPlayer->deleteLater();
            });

    connect(thumbNailPlayer,
           &AVPlayer::error,
           thumbNailPlayer,
           [this, thumbNailPlayer, extractor, position]()
            {

               //LogHandler::Debug("Video load error");
               qint64 randomPosition = XMath::rand(1, position > 0 ? position : thumbCaptureTime);
               //LogHandler::Debug("randomPosition: " + QString::number(randomPosition));
               extractor->setPosition(randomPosition);
               thumbNailPlayer->deleteLater();
            });
}

void MainWindow::on_libray_path_select(QString path)
{
    this->on_load_library(path);
}

void MainWindow::on_actionSelect_library_triggered()
{
    QString selectedLibrary = QFileDialog::getExistingDirectory(this, tr("Choose media library"), ".", QFileDialog::ReadOnly);
    if (selectedLibrary != Q_NULLPTR) {
        on_libray_path_select(selectedLibrary);

        SettingsHandler::setSelectedLibrary(selectedLibrary);
    }
}

void MainWindow::on_LibraryList_itemClicked(QListWidgetItem *item)
{
//    if(!videoHandler->isPlaying())
//    {
//        loadVideo(item->data(Qt::UserRole).value<LibraryListItem>());
//    }
}

void MainWindow::on_LibraryList_itemDoubleClicked(QListWidgetItem *item)
{
    playVideo(item->data(Qt::UserRole).value<LibraryListItem>());
}

void MainWindow::regenerateThumbNail()
{
    QListWidgetItem* selectedItem = ui->LibraryList->selectedItems().first();
    LibraryListItem selectedFileListItem = getLibraryListItemFromQListItem(selectedItem);
    saveThumb(selectedFileListItem.path, selectedFileListItem.thumbFile, selectedItem);
}

void MainWindow::setThumbNailFromCurrent()
{
    QListWidgetItem* selectedItem = ui->LibraryList->selectedItems().first();
    LibraryListItem selectedFileListItem = getLibraryListItemFromQListItem(selectedItem);
    saveThumb(selectedFileListItem.path, selectedFileListItem.thumbFile, selectedItem, videoHandler->position());
}

void MainWindow::playFileFromContextMenu()
{
    LibraryListItem selectedFileListItem = getLibraryListItemFromQListItem(ui->LibraryList->selectedItems().first());
    playVideo(selectedFileListItem);
}

void MainWindow::playFileWithCustomScript()
{
    QString selectedScript = QFileDialog::getOpenFileName(this, tr("Choose script"), SettingsHandler::getSelectedLibrary(), tr("Script Files (*.funscript)"));
    if (selectedScript != Q_NULLPTR)
    {
        LibraryListItem selectedFileListItem = getLibraryListItemFromQListItem(ui->LibraryList->selectedItems().first());
        playVideo(selectedFileListItem, selectedScript);
    }
}

void MainWindow::playVideo(LibraryListItem selectedFileListItem, QString customScript)
{
    QFile file(selectedFileListItem.path);
    if (file.exists())
    {
        ui->videoLoadingLabel->show();
        movie->start();
        videoHandler->stop();
        if (videoHandler->file() != selectedFileListItem.path)
        {
            videoHandler->setFile(selectedFileListItem.path);
            videoPreviewWidget->setFile(selectedFileListItem.path);
            videoHandler->load();
            QString scriptFile = customScript == nullptr ? selectedFileListItem.script : customScript;
            _funscriptLoaded = funscriptHandler->load(scriptFile);
        }
        //QUrl url = QUrl::fromLocalFile(selectedFileListItem.path);
        videoHandler->play();
        playingVideoListIndex = ui->LibraryList->currentRow();
        if(!_funscriptLoaded)
        {
            LogHandler::Dialog("Error loading script " + customScript + "!\nTry right clicking on the video in the list\nand loading with another script.", XLogLevel::Warning);

        }
    }
    else
    {
        LogHandler::Dialog("File '" + selectedFileListItem.path + "' does not exist!", XLogLevel::Critical);
    }
}

void MainWindow::togglePause()
{
    videoHandler->togglePause();
}

void MainWindow::toggleFullScreen()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QSize screenSize = screen->size();
    if(!QMainWindow::isFullScreen())
    {
        videoSize = videoHandler->size();
        appSize = size();
        appPos = pos();
        QMainWindow::setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        ui->MainFrame->layout()->removeWidget(videoHandler);
        ui->MainFrame->layout()->removeWidget(ui->playerControlsFrame);

        playerControlsPlaceHolder = new QFrame;
        placeHolderControlsGrid = new QGridLayout;
        placeHolderControlsGrid->setContentsMargins(0,0,0,0);
        placeHolderControlsGrid->setSpacing(0);
        playerControlsPlaceHolder->setLayout(placeHolderControlsGrid);
        playerControlsPlaceHolder->setContentsMargins(0,0,0,0);
        playerControlsPlaceHolder->installEventFilter(this);
        playerControlsPlaceHolder->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        playerControlsPlaceHolder->move(QPoint(0, screenSize.height() - ui->playerControlsFrame->height()));
        playerControlsPlaceHolder->setFixedWidth(screenSize.width());
        playerControlsPlaceHolder->setFixedHeight(ui->playerControlsFrame->height());
        playerControlsPlaceHolder->layout()->addWidget(ui->playerControlsFrame);

        QMainWindow::layout()->addWidget(videoHandler);
        QMainWindow::layout()->addWidget(playerControlsPlaceHolder);
        ui->playerControlsFrame->setProperty("cssClass", "fullScreenControls");
        ui->playerControlsFrame->style()->unpolish(ui->playerControlsFrame);
        ui->playerControlsFrame->style()->polish(ui->playerControlsFrame);
        QMainWindow::showFullScreen();
        videoHandler->layout()->setMargin(0);
        videoHandler->move(QPoint(0, 0));
        //videoHandler->resize(QSize(screenSize.width()+1, screenSize.height()+1));
        ui->playerControlsFrame->hide();
        videoHandler->resize(screenSize);
        ui->menubar->hide();
    }
    else
    {
        QMainWindow::setWindowFlags(Qt::Window);
        QMainWindow::showNormal();
        QMainWindow::resize(appSize);
        QMainWindow::move(appPos);
        ui->menubar->show();
        QMainWindow::layout()->removeWidget(videoHandler);
        ui->MediaGrid->addWidget(videoHandler);
        playerControlsPlaceHolder->layout()->removeWidget(ui->playerControlsFrame);
        QMainWindow::layout()->removeWidget(playerControlsPlaceHolder);
        ui->playerControlsFrame->setWindowFlags(Qt::Widget);
        ui->playerControlsFrame->setMinimumSize(QSize(700, 0));
        ui->playerControlsFrame->setMaximumSize(QSize(16777215, 16777215));
        ui->ControlsGrid->addWidget(ui->playerControlsFrame);
        ui->playerControlsFrame->setProperty("cssClass", "windowedControls");
        ui->playerControlsFrame->style()->unpolish(ui->playerControlsFrame);
        ui->playerControlsFrame->style()->polish(ui->playerControlsFrame);
        ui->playerControlsFrame->show();
        videoHandler->layout()->setMargin(9);
        delete placeHolderControlsGrid;
        delete playerControlsPlaceHolder;
    }
}

void MainWindow::hideControls()
{
    if (isFullScreen())
    {
        ui->playerControlsFrame->hide();
    }
}

void MainWindow::showControls()
{
    if (isFullScreen())
    {
        ui->playerControlsFrame->show();
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
    if (ui->LibraryList->count() > 0)
    {
        if(ui->LibraryList->selectedItems().length() == 0)
        {
            ui->LibraryList->setCurrentRow(0);
        }
        LibraryListItem selectedFileListItem = getLibraryListItemFromQListItem(ui->LibraryList->selectedItems().first());
        if(selectedFileListItem.path != videoHandler->file() || !videoHandler->isPlaying())
        {
            playVideo(selectedFileListItem);
        }
        else if(ui->LibraryList->selectedItems().length() == 0)
        {
            playingVideoListIndex = 0;
            ui->LibraryList->setCurrentRow(0);
            playVideo(ui->LibraryList->selectedItems()[0]->data(Qt::UserRole).value<LibraryListItem>());
        }
    }
}

void MainWindow::on_PauseBtn_clicked()
{
    MainWindow::togglePause();
}

void MainWindow::on_StopBtn_clicked()
{
    if(videoHandler->isPlaying())
    {
        if(videoHandler->isPaused())
        {
            ui->PauseBtn->setChecked(false);
        }
        videoHandler->stop();
    }
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
    qint64 sliderValueTime = XMath::mapRange(static_cast<qint64>(sliderValue), 0, 100, 0, videoHandler->duration());
//    if (!videoPreviewWidget)
//        videoPreviewWidget = new VideoPreviewWidget();
    videoPreviewWidget->setTimestamp(sliderValueTime);
    videoPreviewWidget->preview();
//    LogHandler::Debug("mousePosition: "+QString::number(sliderValue));
//    LogHandler::Debug("time: "+QString::number(sliderValueTime));
//    LogHandler::Debug("position: "+QString::number(position));
    QPoint gpos;
    if(MainWindow::isFullScreen())
        gpos = mapToGlobal(playerControlsPlaceHolder->pos() + ui->SeekSlider->pos() + QPoint(position, 0));
    else
        gpos = mapToGlobal(ui->playerControlsFrame->pos() + ui->SeekSlider->pos() + QPoint(position, 0));
//    LogHandler::Debug("gpos x: "+QString::number(gpos.x()));
//    LogHandler::Debug("gpos y: "+QString::number(gpos.y()));
    QToolTip::showText(gpos, QTime(0, 0, 0).addMSecs(sliderValueTime).toString(QString::fromLatin1("HH:mm:ss")));
//    if (!Config::instance().previewEnabled())
//        return;

   //const int w = Config::instance().previewWidth();
    //const int h = Config::instance().previewHeight();
    videoPreviewWidget->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    videoPreviewWidget->resize(175, 175);
    videoPreviewWidget->move(gpos - QPoint(175/2, 175));
    videoPreviewWidget->raise();
    videoPreviewWidget->show();
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
    qint64 playerPosition = XMath::mapRange(static_cast<qint64>(position), 0, 100, 0, videoHandler->duration());
    ui->SeekSlider->setToolTip(QTime(0, 0, 0).addMSecs(position).toString(QString::fromLatin1("HH:mm:ss")));
    videoHandler->setPosition(playerPosition);
}


void MainWindow::on_SeekSlider_valueChanged(int position)
{

}

void MainWindow::on_media_positionChanged(qint64 position)
{
    ui->lblCurrentDuration->setText( second_to_minutes(position / 1000).append("/").append( second_to_minutes( (videoHandler->duration())/1000 ) ) );

    //ui->lblCurrentDuration->setText(QTime(0, 0, 0).addMSecs(position).toString(QString::fromLatin1("HH:mm:ss")));
    qint64 duration = videoHandler->duration();
    if (duration > 0)
    {
        qint64 sliderPosition = XMath::mapRange(position, 0, duration, 0, 100);
        ui->SeekSlider->setUpperValue(static_cast<int>(sliderPosition));
    }
}

void MainWindow::on_media_start()
{
    ui->videoLoadingLabel->hide();
    movie->stop();
    ui->SeekSlider->setDisabled(false);
    ui->StopBtn->setDisabled(false);
    ui->PauseBtn->setDisabled(false);
    ui->PauseBtn->setChecked(false);
    ui->fullScreenBtn->setDisabled(false);
    if(funscriptFuture.isRunning())
    {
        funscriptFuture.cancel();
        funscriptFuture.waitForFinished();
    }
    if (_funscriptLoaded)
    {
        funscriptFuture = QtConcurrent::run(syncFunscript, videoHandler, _xSettings, tcodeHandler, funscriptHandler);
    }
}
void MainWindow::onDeoMessageRecieved(DeoPacket packet)
{
//        LogHandler::Debug("Deo path: "+packet.path);
//LogHandler::Debug("Deo duration: "+QString::number(packet.duration));
//        LogHandler::Debug("Deo currentTime: "+QString::number(packet.currentTime));
//        LogHandler::Debug("Deo playbackSpeed: "+QString::number(packet.playbackSpeed));
//        LogHandler::Debug("Deo playing: "+QString::number(packet.playing));

    if (!funscriptFileSelectorOpen && packet.duration > 0)
    {
        QString videoPath = packet.path;
        QFileInfo videoFile(videoPath);
        QString funscriptPath = SettingsHandler::getDeoDnlaFunscript(videoPath);
        QFileInfo funscriptFile(funscriptPath);
        if (funscriptPath == nullptr || !funscriptFile.exists())
        {
            _funscriptLoaded = false;
            //Check the deo device local video directory for funscript.
            int indexOfSuffix = packet.path.lastIndexOf(".");
            QString localFunscriptPath = packet.path.replace(indexOfSuffix, packet.path.length() - indexOfSuffix, ".funscript");
            QFile localFile(localFunscriptPath);
            if(localFile.exists())
            {
                funscriptPath = localFunscriptPath;
            }
            else
            {
                //Check the user selected library location.
                QFileInfo fileinfo(packet.path);
                QString libraryScriptFile = fileinfo.fileName().remove(fileinfo.fileName().lastIndexOf('.'), fileinfo.fileName().length() -  1) + ".funscript";
                QString libraryScriptPath = SettingsHandler::getSelectedFunscriptLibrary() + QDir::separator() + libraryScriptFile;
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
                funscriptPath = QFileDialog::getOpenFileName(this, "Choose script for video: " + videoFile.fileName(), SettingsHandler::getSelectedLibrary(), "Script Files (*.funscript)");
                funscriptFileSelectorOpen = false;
            }
            //Store the location of the file so the above check doesnt happen again.
            SettingsHandler::setDeoDnlaFunscript(videoPath, funscriptPath);
        }
    }
}

void syncDeoFunscript(DeoHandler* deoPlayer, VideoHandler* xPlayer, SettingsDialog* xSettings, TCodeHandler* tcodeHandler, FunscriptHandler* funscriptHandler)
{
    DeviceHandler* device = xSettings->getSelectedDeviceHandler();
    std::shared_ptr<FunscriptAction> actionPosition;
    DeoPacket* currentDeoPacket = deoPlayer->getCurrentDeoPacket();
    QString currentVideo;
    qint64 timeTracker = 0;
    qint64 lastDeoTime = 0;
    qint64 timer1 = QTime::currentTime().msecsSinceStartOfDay();
    qint64 timer2 = QTime::currentTime().msecsSinceStartOfDay();
    //qint64 elapsedTracker = 0;
    bool funscriptLoaded = false;
    //QElapsedTimer timer;
    while (deoPlayer->isConnected() && !xPlayer->isPlaying())
    {
        //timer.start();
        if(xSettings->isConnected() && funscriptLoaded && currentDeoPacket != nullptr && currentDeoPacket->duration > 0 && currentDeoPacket->playing)
        {
            //execute once every milli second
            if (timer2 - timer1 >= 1)
            {
                timer1 = timer2;
                qint64 currentTime = currentDeoPacket->currentTime;
                if (currentTime > timeTracker + 100 || lastDeoTime > currentTime)
                {
                    lastDeoTime = currentTime;
                    LogHandler::Debug("current time reset: "+QString::number(currentTime));
                    LogHandler::Debug("timeTracker: "+QString::number(timeTracker));
                    timeTracker = currentTime;
                }
                else
                {
                    timeTracker++;
                    currentTime = timeTracker;
                }
                actionPosition = funscriptHandler->getPosition(currentTime);
                if (actionPosition != nullptr)
                {
                    device->sendTCode(tcodeHandler->funscriptToTCode(actionPosition->pos, actionPosition->speed));
                }
            }
            timer2 = QTime::currentTime().msecsSinceStartOfDay();
            //elapsedTracker = (round(timer.nsecsElapsed()) / 1000000);
            //LogHandler::Debug("timer nsecsElapsed: "+QString::number(elapsedTracker));
        }
        else if(xSettings->isConnected() && currentDeoPacket != nullptr && currentDeoPacket->duration > 0 && currentDeoPacket->playing)
        {
            if (!currentDeoPacket->path.isEmpty() && !currentDeoPacket->path.isNull())
            {
                QString funscriptPath = SettingsHandler::getDeoDnlaFunscript(currentDeoPacket->path);
                currentVideo = currentDeoPacket->path;
                funscriptLoaded = funscriptHandler->load(funscriptPath);
            }
        }

        if(currentDeoPacket != nullptr && currentVideo != currentDeoPacket->path)
        {
            currentVideo = currentDeoPacket->path;
            funscriptLoaded = false;
        }
        currentDeoPacket = deoPlayer->getCurrentDeoPacket();

    }
    LogHandler::Debug("exit syncDeoFunscript");
}

void syncFunscript(VideoHandler* player, SettingsDialog* xSettings, TCodeHandler* tcodeHandler, FunscriptHandler* funscriptHandler)
{
    std::unique_ptr<FunscriptAction> actionPosition;
    DeviceHandler* device = xSettings->getSelectedDeviceHandler();
    while (player->isPlaying())
    {
        if(xSettings->isConnected())
        {
            qint64 position = player->position();
            actionPosition = funscriptHandler->getPosition(position);
            if (actionPosition != nullptr)
            {
                device->sendTCode(tcodeHandler->funscriptToTCode(actionPosition->pos, actionPosition->speed));
            }
            actionPosition = nullptr;
        }
        QThread::currentThread()->msleep(1);
    }
    LogHandler::Debug("exit syncFunscript");
}

void MainWindow::on_media_stop()
{
    ui->videoLoadingLabel->hide();
    movie->stop();
    ui->SeekSlider->setUpperValue(0);
    ui->SeekSlider->setDisabled(true);
    ui->StopBtn->setDisabled(true);
    ui->PauseBtn->setDisabled(true);
    ui->PauseBtn->setChecked(false);
    ui->fullScreenBtn->setDisabled(true);
    ui->lblCurrentDuration->setText("00:00:00");

    if(funscriptFuture.isRunning())
    {
        funscriptFuture.cancel();
    }

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
        skipForward();
    break;
    case NoMedia:
        //status = tr("No media");
        break;
    case InvalidMedia:
        //status = tr("Invalid meida");
        break;
    case BufferingMedia:
        ui->videoLoadingLabel->show();
        movie->start();
        break;
    case BufferedMedia:
        ui->videoLoadingLabel->hide();
        movie->stop();
        break;
    case LoadingMedia:
        ui->videoLoadingLabel->show();
        movie->start();
        break;
    case LoadedMedia:
        ui->videoLoadingLabel->hide();
        movie->stop();
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
    ++playingVideoListIndex;
    if(playingVideoListIndex < ui->LibraryList->count())
    {
        ui->LibraryList->setCurrentRow(playingVideoListIndex);
        on_PlayBtn_clicked();
    }
}

void MainWindow::skipBack()
{
    --playingVideoListIndex;
    if(playingVideoListIndex >= 0)
    {
        ui->LibraryList->setCurrentRow(playingVideoListIndex);
        on_PlayBtn_clicked();
    }
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
    if ( event->button() == Qt::LeftButton )
    {
        MainWindow::togglePause();
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

void MainWindow::on_device_error(QString error)
{
    LogHandler::Dialog(error, XLogLevel::Critical);
}

void MainWindow::on_deo_device_connectionChanged(ConnectionChangedSignal event)
{
    deviceConnected = event.status == ConnectionStatus::Connected;
    QString message = "";
    message += "DeoVR: ";
    message += " " + event.message;
    deoConnectionStatusLabel->setText(message);
    if(event.status == ConnectionStatus::Error || event.status == ConnectionStatus::Disconnected)
    {
        deoRetryConnectionButton->show();
        if(funscriptFuture.isRunning())
        {
            funscriptFuture.cancel();
            funscriptFuture.waitForFinished();
        }
    }
    else if(deviceConnected)
    {
        deoRetryConnectionButton->hide();
        if(funscriptFuture.isRunning())
        {
            funscriptFuture.cancel();
            funscriptFuture.waitForFinished();
        }
        funscriptFuture = QtConcurrent::run(syncDeoFunscript, _xSettings->getDeoHandler(), videoHandler, _xSettings, tcodeHandler, funscriptHandler);

    }
    else
    {
        deoRetryConnectionButton->hide();
    }
}

void MainWindow::on_deo_device_error(QString error)
{
    LogHandler::Dialog("Deo error: "+error, XLogLevel::Critical);
}

QString MainWindow::second_to_minutes(int seconds)
{
    int sec = seconds;
    QString mn = QString::number( (sec ) / 60);
    int _tmp_mn  = mn.toInt() * 60;
    QString sc= QString::number( (seconds - _tmp_mn  ) % 60 );

    return (mn.length() == 1 ? "0" + mn : mn ) + ":" + (sc.length() == 1 ? "0" + sc : sc);
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
                                                                    "Donate: <a href='https://www.patreon.com/Khrull'>https://www.patreon.com/Khrull</a>");
    copyright.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&copyright);
    QLabel sources;
    sources.setText("This software uses libraries from:");
    sources.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&sources);
    QLabel qtInfo;
    qtInfo.setFrameStyle(QFrame::Panel | QFrame::Sunken);
    qtInfo.setText("<b>Qt 1.15.0</b><br>"
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
                      "Copyright (c) 2019 ThisIsClark<br>"
                      "<a href='https://github.com/ThisIsClark/Qt-RangeSlider'>https://github.com/ThisIsClark/Qt-RangeSlider</a>");
    rangeSliderInfo.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&rangeSliderInfo);
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
    _xSettings->exec();
}

void MainWindow::on_actionThumbnail_triggered()
{
    SettingsHandler::setLibraryView(LibraryView::Thumb);
    ui->LibraryList->setResizeMode(QListView::Adjust);
    ui->LibraryList->setFlow(QListView::LeftToRight);
    ui->LibraryList->setViewMode(QListView::IconMode);
    ui->LibraryList->setTextElideMode(Qt::ElideMiddle);
    ui->LibraryList->setSpacing(2);
    updateThumbSizeUI(SettingsHandler::getThumbSize());
}

void MainWindow::on_actionList_triggered()
{
    SettingsHandler::setLibraryView(LibraryView::List);
    ui->LibraryList->setResizeMode(QListView::Fixed);
    ui->LibraryList->setFlow(QListView::TopToBottom);
    ui->LibraryList->setViewMode(QListView::ListMode);
    ui->LibraryList->setTextElideMode(Qt::ElideRight);
    ui->LibraryList->setSpacing(1);
    updateThumbSizeUI(SettingsHandler::getThumbSizeList());
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
    }
}

void MainWindow::on_action75_triggered()
{
    setThumbSize(75);
    ui->LibraryList->setIconSize(currentThumbSize);
}

void MainWindow::on_action100_triggered()
{
    setThumbSize(100);
    ui->LibraryList->setIconSize(currentThumbSize);
}

void MainWindow::on_action125_triggered()
{
    setThumbSize(125);
    ui->LibraryList->setIconSize(currentThumbSize);
}

void MainWindow::on_action150_triggered()
{
    setThumbSize(150);
    ui->LibraryList->setIconSize(currentThumbSize);
}

void MainWindow::on_action175_triggered()
{
    setThumbSize(175);
    ui->LibraryList->setIconSize(currentThumbSize);
}

void MainWindow::setThumbSize(int size)
{
    currentThumbSize = {size, size};
    if (SettingsHandler::getLibraryView() == LibraryView::List)
    {
        SettingsHandler::setThumbSizeList(size);
    }
    else
    {
        SettingsHandler::setThumbSize(size);
    }
}

LibraryListItem MainWindow::getLibraryListItemFromQListItem(QListWidgetItem* qListWidgetItem)
{
    return qListWidgetItem->data(Qt::UserRole).value<LibraryListItem>();
}


void MainWindow::on_actionChange_theme_triggered()
{
    QFileInfo selectedThemeInfo(SettingsHandler::getSelectedTheme());
    QString selectedTheme = QFileDialog::getOpenFileName(this, "Choose XTP theme", selectedThemeInfo.absoluteDir().absolutePath(), "CSS Files (*.css)");
    if(!selectedTheme.isNull())
        SettingsHandler::setSelectedTheme(selectedTheme);
    QFile file(selectedTheme);
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    setStyleSheet(styleSheet);
}
