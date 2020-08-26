#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDirIterator>
#include <qfloat16.h>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QScreen>

int voulumeBeforeMute;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setStyleSheet("QMainWindow {"
                  "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1.5, stop: 0 black, stop: 1 darkred); "
                  "color: white; "
                  "}"
                  "QLabel {"
                  "background-color: transparent; "
                  "color: white; "
                  "}"
                  "QListWidget {"
                  "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1.5, stop: 0 black, stop: 1 darkred); "
                  "color: white; "
                  "}"
                  "QMenuBar {"
                  "background-color: black; "
                  "color: white; "
                  "}"
                  "QMenu {"
                  "background-color: black; "
                  "color: white; "
                  "}"
                  "QMenu:hover:!pressed {"
                  "background-color: darkred; "
                  "color: white; "
                  "}"
                  "QAction:hover:!pressed {"
                  "background-color: darkred; "
                  "color: white; "
                  "}"
                  "QRadioButton {"
                  "background-color: transparent; "
                  "color: white; "
                  "}"
                  "QScrollBar:handle:vertical {"
                  "background-color: #4f0000; "
                  "}"
                  "QScrollBar:horizontal {"
                  "background-color: black; "
                  "}"
                  "QScrollBar:handle:horizontal {"
                  "background-color: #3e0000; "
                  "}"
                  "QScrollBar:vertical {"
                  "background-color: black; "
                  "}"
                  "QPushbutton { "
                    "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1, stop: 0 black, stop: 1 grey); "
                    "border-style: outset; "
                    "border-width: 2px; "
                    "border-radius: 10px; "
                    "border-color: beige; "
                    "font: bold 14px; "
                    "min-width: 10em; "
                    "padding: 6px; "
                    "}; ");
//                   "QPushButton:pressed { "
//                      "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 red, stop: 1 black); "
//                      "border-style: inset; "
//                    "}; ");







//    QFile file(":/qss/default.qss");
//    file.open(QFile::ReadOnly);
//    QString styleSheet = QLatin1String(file.readAll());
    SettingsHandler::Load();

    //keyPress = new KeyPress();
    //keyPress->show();
    serialHandler = new SerialHandler(this);
    udpHandler = new UdpHandler(this);
    videoHandler = new VideoHandler(this);
    ui->MediaGrid->addWidget(videoHandler);
    loadSerialPorts();

    ui->SeekSlider->setDisabled(true);
    ui->SeekSlider->SetRange(0, 100);
    ui->SeekSlider->setOption(RangeSlider::Option::RightHandle);
    ui->SeekSlider->setUpperValue(0);
    ui->SeekSlider->setBackGroundEnabledColor(QColorConstants::Red);

    on_load_library(SettingsHandler::getSelectedLibrary());

    ui->VolumeSlider->setDisabled(false);
    ui->VolumeSlider->SetRange(0, 30);
    ui->VolumeSlider->setOption(RangeSlider::Option::RightHandle);
    ui->VolumeSlider->setUpperValue(SettingsHandler::getPlayerVolume());
    ui->VolumeSlider->setBackGroundEnabledColor(QColorConstants::Red);

    ui->SerialOutputCmb->setCurrentText(SettingsHandler::getSerialPort());
    ui->networkAddressTxt->setText(SettingsHandler::getServerAddress());
    ui->networkPortTxt->setText(SettingsHandler::getServerPort());
    if(SettingsHandler::getSelectedDevice() == DeviceType::Serial)
    {
        selectedDevice = serialHandler;
        ui->serialOutputRdo->setChecked(true);
    }
    else if (SettingsHandler::getSelectedDevice() == DeviceType::Network)
    {
        selectedDevice = udpHandler;
        ui->networkOutputRdo->setChecked(true);
    }
    setDeviceStatusStyle(ConnectionStatus::Disconnected, DeviceType::Serial);
    setDeviceStatusStyle(ConnectionStatus::Disconnected, DeviceType::Network);

    QFont font( "Sans Serif", 7);

    xRangeLabel = new QLabel("X Range");
    xRangeLabel->setFont(font);
    ui->RangeSettingsGrid->addWidget(xRangeLabel, 0,0);
    xRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
    xRangeSlider->setBackGroundEnabledColor(QColorConstants::Red);
    xRangeSlider->SetRange(1, 999);
    xRangeSlider->setLowerValue(SettingsHandler::getXMin());
    xRangeSlider->setUpperValue(SettingsHandler::getXMax());
    ui->RangeSettingsGrid->addWidget(xRangeSlider, 1,0);

    yRollRangeLabel = new QLabel("Y Roll Range");
    yRollRangeLabel->setFont(font);
    ui->RangeSettingsGrid->addWidget(yRollRangeLabel, 2,0);
    yRollRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
    yRollRangeSlider->setBackGroundEnabledColor(QColorConstants::Red);
    yRollRangeSlider->SetRange(1, 999);
    yRollRangeSlider->setLowerValue(SettingsHandler::getYRollMin());
    yRollRangeSlider->setUpperValue(SettingsHandler::getYRollMax());
    ui->RangeSettingsGrid->addWidget(yRollRangeSlider, 3,0);

    xRollRangeLabel = new QLabel("X Roll Range");
    xRollRangeLabel->setFont(font);
    ui->RangeSettingsGrid->addWidget(xRollRangeLabel, 4,0);
    xRollRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
    xRollRangeSlider->setBackGroundEnabledColor(QColorConstants::Red);
    xRollRangeSlider->SetRange(1, 999);
    xRollRangeSlider->setLowerValue(SettingsHandler::getXRollMin());
    xRollRangeSlider->setUpperValue(SettingsHandler::getXRollMax());
    ui->RangeSettingsGrid->addWidget(xRollRangeSlider, 5,0);

    offSetLabel = new QLabel("Offset: " + QString::number(SettingsHandler::getoffSet()));
    offSetLabel->setFont(font);
    ui->RangeSettingsGrid->addWidget(offSetLabel, 6,0);
    offSetSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::RightHandle, nullptr);
    offSetSlider->setBackGroundEnabledColor(QColorConstants::Red);
    offSetSlider->SetRange(1, 2000);
    offSetSlider->setUpperValue(SettingsHandler::getoffSetMap());
    ui->RangeSettingsGrid->addWidget(offSetSlider, 7,0);


    connect(xRangeSlider, &RangeSlider::lowerValueChanged, this, &MainWindow::onXRange_valueChanged);
    connect(xRangeSlider, &RangeSlider::upperValueChanged, this, &MainWindow::onXRange_valueChanged);
    connect(yRollRangeSlider, &RangeSlider::lowerValueChanged, this, &MainWindow::onYRollRange_valueChanged);
    connect(yRollRangeSlider, &RangeSlider::upperValueChanged, this, &MainWindow::onYRollRange_valueChanged);
    connect(xRollRangeSlider, &RangeSlider::lowerValueChanged, this, &MainWindow::onXRollRange_valueChanged);
    connect(xRollRangeSlider, &RangeSlider::upperValueChanged, this, &MainWindow::onXRollRange_valueChanged);
    connect(offSetSlider, &RangeSlider::lowerValueChanged, this, &MainWindow::onOffSet_valueChanged);

    connect(videoHandler, &VideoHandler::positionChanged, this, &MainWindow::on_media_positionChanged);
    connect(videoHandler, &VideoHandler::mediaStatusChanged, this, &MainWindow::on_media_statusChanged);
    connect(videoHandler, &VideoHandler::started, this, &MainWindow::on_media_start);
    connect(videoHandler, &VideoHandler::stopped, this, &MainWindow::on_media_stop);

    connect(ui->SeekSlider, &RangeSlider::upperValueMove, this, &MainWindow::on_seekSlider_sliderMoved);
    connect(ui->VolumeSlider, &RangeSlider::upperValueMove, this, &MainWindow::on_VolumeSlider_valueChanged);
    //connect(player, static_cast<void(AVPlayer::*)(AVPlayer::Error )>(&AVPlayer::error), this, &MainWindow::on_media_error);

    connect(videoHandler, &VideoHandler::doubleClicked, this, &MainWindow::media_double_click_event);
    //connect(vw, &XVideoWidget::singleClicked, this, &MainWindow::media_single_click_event);
    connect(this, &MainWindow::keyPressed, this, &MainWindow::on_key_press);
    //connect(videoHandler, &VideoHandler::mouseEnter, this, &MainWindow::on_video_mouse_enter);
    connect(serialHandler, &SerialHandler::connectionChange, this, &MainWindow::on_device_connectionChanged);
    connect(serialHandler, &SerialHandler::errorOccurred, this, &MainWindow::on_device_error);
    connect(udpHandler, &UdpHandler::connectionChange, this, &MainWindow::on_device_connectionChanged);
    connect(udpHandler, &UdpHandler::errorOccurred, this, &MainWindow::on_device_error);

    ui->LibraryList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->LibraryList, &QListWidget::customContextMenuRequested, this, &MainWindow::onLibraryList_ContextMenuRequested);

    if(SettingsHandler::getSelectedDevice() == DeviceType::Network)
    {
        on_networkOutputRdo_clicked();
    }
}

MainWindow::~MainWindow()
{
    SettingsHandler::Save();
    udpHandler->dispose();
    serialHandler->dispose();
    delete ui;
    delete videoHandler;
    if(initFuture.isRunning())
    {
        initFuture.cancel();
        initFuture.waitForFinished();
    }
    if(funscriptFuture.isRunning())
    {
        funscriptFuture.cancel();
        funscriptFuture.waitForFinished();
    }
    //delete keyPress;
}

void MainWindow::on_key_press(QKeyEvent * event)
{
    switch(event->key())
    {
        case Qt::Key_Space:
            MainWindow::togglePause();
            break;
        case Qt::Key_F11:
            MainWindow::toggleFullScreen();
            break;
    }
}

void MainWindow::on_video_mouse_enter(QEvent * event)
{
    if (isFullScreen())
    {
        toggleControls();
        QTimer::singleShot(2000, this, &MainWindow::toggleControls);
    }
}

void MainWindow::onXRange_valueChanged(int value)
{
    SettingsHandler::setXMin(xRangeSlider->GetLowerValue());
    SettingsHandler::setXMax(xRangeSlider->GetUpperValue());
    if (!videoHandler->isPlaying() && selectedDevice->isRunning())
    {
        selectedDevice->sendTCode("L0" + QString::number(value) + "S1000");
    }
}

void MainWindow::onYRollRange_valueChanged(int value)
{
    SettingsHandler::setYRollMin(yRollRangeSlider->GetLowerValue());
    SettingsHandler::setYRollMax(yRollRangeSlider->GetUpperValue());
    if ( selectedDevice->isRunning())
    {
        selectedDevice->sendTCode("R1" + QString::number(value) + "S1000");
    }
}

void MainWindow::onXRollRange_valueChanged(int value)
{
    SettingsHandler::setXRollMin(xRollRangeSlider->GetLowerValue());
    SettingsHandler::setXRollMax(xRollRangeSlider->GetUpperValue());
    if (selectedDevice->isRunning())
    {
        selectedDevice->sendTCode("R2" + QString::number(value) + "S1000");
    }
}

void MainWindow::onOffSet_valueChanged(int value)
{
    SettingsHandler::setoffSet(offSetSlider->GetUpperValue());
    offSetLabel->setText("Offset: " + QString::number(SettingsHandler::getoffSet()));
}

void MainWindow::onLibraryList_ContextMenuRequested(const QPoint &pos)
{
    // Handle global position
    QPoint globalPos = ui->LibraryList->mapToGlobal(pos);

    // Create menu and insert some actions
    QMenu myMenu;

    myMenu.addAction("Play", this, &MainWindow::playFileFromContextMenu);
    myMenu.addAction("Play with funscript...", this, &MainWindow::playFileWithCustomScript);

    // Show context menu at handling position
    myMenu.exec(globalPos);
}

void MainWindow::on_load_library(QString path)
{
    if (!path.isNull() && !path.isEmpty())
    {
        QDir directory(path);
        if (directory.exists())
        {
            ui->LibraryList->clear();
            videos.clear();
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
                    LogHandler::Debug("Script does not exist for video: " + videoPath);
                }
                LibraryListItem item
                {
                    videoPath, // path
                    fileName, // name
                    scriptPath // script
                };
                QVariant listItem;
                listItem.setValue(item);
                QListWidgetItem* qListWidgetItem = new QListWidgetItem;
                qListWidgetItem->setText(fileinfo.fileName());
                qListWidgetItem->setToolTip(videoPath);
                qListWidgetItem->setData(Qt::UserRole, listItem);
                ui->LibraryList->addItem(qListWidgetItem);
                videos.push_back(videoPath);
            }
        }
        else
        {
           LogHandler::Dialog("Library path '" + path + "' does not exist", XLogLevel::Critical);
           on_actionSelect_library_triggered();
        }
    }
    else
    {
        on_actionSelect_library_triggered();
    }
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
    selectedFileListItem = item->data(Qt::UserRole).value<LibraryListItem>();
}

void MainWindow::on_LibraryList_itemDoubleClicked(QListWidgetItem *item)
{
    selectedFileListItem = item->data(Qt::UserRole).value<LibraryListItem>();
    playFile(selectedFileListItem);
}
void MainWindow::playFileFromContextMenu()
{
    LibraryListItem selectedFileListItem = ui->LibraryList->selectedItems().first()->data(Qt::UserRole).value<LibraryListItem>();
    playFile(selectedFileListItem);
}

void MainWindow::playFileWithCustomScript()
{
    QString selectedScript = QFileDialog::getOpenFileName(this, tr("Choose script"), SettingsHandler::getSelectedLibrary(), tr("Script Files (*.funscript)"));
    if (selectedScript != Q_NULLPTR)
    {
        LibraryListItem selectedFileListItem = ui->LibraryList->selectedItems().first()->data(Qt::UserRole).value<LibraryListItem>();
        playFile(selectedFileListItem, selectedScript);
    }
}

void MainWindow::playFile(LibraryListItem selectedFileListItem, QString customScript)
{
    videoHandler->stop();
    QFile file(selectedFileListItem.path);
    if (file.exists())
    {
        videoHandler->setFile(selectedFileListItem.path);
        videoHandler->load();
        QString scriptFile = customScript == nullptr ? selectedFileListItem.script : customScript;
        if(funscriptHandler->load(scriptFile))
        {
            SettingsHandler::setSelectedFile(selectedFileListItem.path);
            //QUrl url = QUrl::fromLocalFile(selectedFileListItem.path);
            videoHandler->play();
            selectedFileListIndex = ui->LibraryList->currentRow();
        }
        else
        {
            LogHandler::Dialog("Error loading script " + customScript + "!", XLogLevel::Critical);
        }
    }
    else {
        LogHandler::Dialog("File '" + selectedFileListItem.path + "' does not exist!", XLogLevel::Critical);
    }
}

void MainWindow::loadSerialPorts()
{
    ui->SerialOutputCmb->clear();
    serialPorts.clear();
    serialPorts = serialHandler->getPorts();
    foreach(SerialComboboxItem item , serialPorts)
    {
        QVariant itemVarient;
        itemVarient.setValue(item);
        ui->SerialOutputCmb->addItem(item.friendlyName, itemVarient);
    }
}

void MainWindow::togglePause()
{
    videoHandler->togglePause();
}

void MainWindow::toggleFullScreen()
{
    if(videoHandler->isPlaying()) {
        //toggleUI();
        QScreen *screen = QGuiApplication::primaryScreen();
        QSize screenSize = screen->size();
        if(!QMainWindow::isFullScreen()) {
            videoSize = videoHandler->size();
            appSize = size();
            appPos = pos();
            setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
            videoHandler->move(QPoint(0, 0));
            ui->MainFrame->layout()->removeWidget(videoHandler);
            //ui->MainFrame->layout()->setMargin(0);
//            ui->MainFrame->hide();
//            ui->statusbar->hide();
            ui->menubar->hide();
            QMainWindow::layout()->addWidget(videoHandler);
            QMainWindow::showFullScreen();
            videoHandler->layout()->setMargin(0);
//            QMainWindow::layout()->setContentsMargins(0,0,0,0);
//            QMainWindow::layout()->setMargin(0);
            videoHandler->move(QPoint(0, 0));
            //videoHandler->resize(QSize(screenSize.width()+1, screenSize.height()+1));
            videoHandler->resize(screenSize);
        }
        else
        {
            //videoHandler->setParent(this, Qt::Widget);
            setWindowFlags(Qt::Window);
            //videoHandler->resize(videoSize);
            QMainWindow::showNormal();
            QMainWindow::resize(appSize);
            QMainWindow::move(appPos);
            QMainWindow::layout()->removeWidget(videoHandler);
//            QMainWindow::layout()->setContentsMargins(9,9,9,9);
//            QMainWindow::layout()->setMargin(9);
            ui->MediaGrid->addWidget(videoHandler);
            videoHandler->layout()->setMargin(9);
        }
    }
}

void MainWindow::toggleControls()
{
    if(!ui->TCodeQuickSettingsGroup->isHidden())
    {
        ui->TCodeQuickSettingsGroup->hide();
    }
    else
    {
        ui->TCodeQuickSettingsGroup->show();
    }
}

void MainWindow::on_VolumeSlider_valueChanged(int value)
{
    if(!videoHandler->isMute())
    {
        videoHandler->setVolume(value);
        SettingsHandler::setPlayerVolume(value);
    }
    ui->VolumeSlider->setToolTip(QString::number(value));
}

void MainWindow::on_PlayBtn_clicked()
{
    if(selectedFileListItem.name != "" && selectedFileListItem.path != videoHandler->file())
    {
        MainWindow::playFile(selectedFileListItem);
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
    videoHandler->toggleMute();
    if (checked)
    {
        voulumeBeforeMute = ui->VolumeSlider->GetUpperValue();
        ui->VolumeSlider->setUpperValue(0);
    }
    else
    {
        ui->VolumeSlider->setUpperValue(voulumeBeforeMute);
    }
}

void MainWindow::on_fullScreenBtn_clicked()
{
    MainWindow::toggleFullScreen();
}

void MainWindow::on_serialRefreshBtn_clicked()
{
    MainWindow::loadSerialPorts();
}

void MainWindow::on_seekSlider_sliderMoved(int position)
{
    qint64 playerPosition = XMath::mapRange(static_cast<qint64>(position), 0, 100, 0, videoHandler->duration());
    ui->SeekSlider->setToolTip(QTime(0, 0, 0).addMSecs(position).toString(QString::fromLatin1("HH:mm:ss")));
    videoHandler->setPosition(playerPosition);
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
    ui->SeekSlider->setDisabled(false);
    ui->StopBtn->setDisabled(false);
    ui->PauseBtn->setDisabled(false);
    ui->fullScreenBtn->setDisabled(false);
    if(funscriptFuture.isRunning())
    {
        funscriptFuture.cancel();
        funscriptFuture.waitForFinished();
    }
    funscriptFuture = QtConcurrent::run(syncFunscript, videoHandler, serialHandler, udpHandler, tcodeHandler, funscriptHandler);
}

void syncFunscript(VideoHandler* player, SerialHandler* serialHandler, UdpHandler* udpHandler, TCodeHandler* tcodeHandler, FunscriptHandler* funscriptHandler)
{
    std::unique_ptr<FunscriptAction> actionPosition;
    while (player->isPlaying())
    {
        if(serialHandler->isConnected() || udpHandler->isConnected()) {
            qint64 position = player->position();
            actionPosition = funscriptHandler->getPosition(position);
            if (actionPosition != nullptr)
            {
                if (SettingsHandler::getSelectedDevice() == DeviceType::Serial)
                    serialHandler->sendTCode(tcodeHandler->funscriptToTCode(actionPosition->pos, actionPosition->speed));
                else if (SettingsHandler::getSelectedDevice() == DeviceType::Network)
                    udpHandler->sendTCode(tcodeHandler->funscriptToTCode(actionPosition->pos, actionPosition->speed));
            }
            actionPosition.reset();
        }
        Sleep(1);
    }
    //serialHandler->sendTCode(tcodeHandler->funscriptToTCode(actionPosition->pos));
    LogHandler::Debug("exit syncFunscript");
}

void MainWindow::on_media_stop()
{
    ui->SeekSlider->setUpperValue(0);
    qDebug(">>>>>>>>>>>>>>disable slider");
    ui->SeekSlider->setDisabled(true);
    ui->StopBtn->setDisabled(true);
    ui->PauseBtn->setDisabled(true);
    ui->fullScreenBtn->setDisabled(true);
    ui->lblCurrentDuration->setText("00:00:00");

    if(funscriptFuture.isRunning())
    {
        funscriptFuture.cancel();
    }
    if (selectedDevice->isRunning())
    {
        selectedDevice->dispose();
    }

}

void MainWindow::on_media_statusChanged(MediaStatus status)
{
    switch (status) {
    case EndOfMedia:
        ++selectedFileListIndex;
        if(selectedFileListIndex < videos.length())
        {
            ui->LibraryList->setCurrentRow(selectedFileListIndex);
            selectedFileListItem = ui->LibraryList->selectedItems()[0]->data(Qt::UserRole).value<LibraryListItem>();
            on_PlayBtn_clicked();
        }
    break;
    case NoMedia:
        //status = tr("No media");
        break;
    case InvalidMedia:
        //status = tr("Invalid meida");
        break;
    case BufferingMedia:
        //status = tr("Buffering...");
        break;
    case BufferedMedia:
        //status = tr("Buffered");
        break;
    case LoadingMedia:
        //status = tr("Loading...");
        break;
    case LoadedMedia:
        //status = tr("Loaded");
        break;
    case StalledMedia:
        //status = tr("Stalled");
        break;
    default:
        //status = QString();
        //onStopPlay();
        break;
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
    if(event.deviceType == DeviceType::Serial)
    {
        SettingsHandler::setSerialPort(ui->SerialOutputCmb->currentText());
        ui->serialStatuslbl->setText(event.message);
    }
    else if (event.deviceType == DeviceType::Network)
    {
    }
    if (event.status == ConnectionStatus::Error)
        setDeviceStatusStyle(event.status, event.deviceType, event.message);
    else
        setDeviceStatusStyle(event.status, event.deviceType);
}

void MainWindow::setDeviceStatusStyle(ConnectionStatus status, DeviceType deviceType, QString message)
{
    QString statusUnicode = "\u2717";
    QString statusColor = "red";
    QFont font( "Sans Serif", 12);
    if (status == ConnectionStatus::Connected)
    {
        statusUnicode = "\u2713";
        statusColor = "green";
    }
    else if (status == ConnectionStatus::Connecting)
    {
        statusUnicode = "\u231B";
        statusColor = "yellow";
    }
    if (deviceType == DeviceType::Serial)
    {
        ui->serialStatuslbl->setText(statusUnicode + " " + message);
        ui->serialStatuslbl->setFont(font);
        ui->serialStatuslbl->setStyleSheet("color: " + statusColor);
    }
    else if (deviceType == DeviceType::Network)
    {
        ui->networkStatuslbl->setText(statusUnicode + " " + message);
        ui->networkStatuslbl->setFont(font);
        ui->networkStatuslbl->setStyleSheet("color: " + statusColor);
    }
}

void MainWindow::on_device_error(QString error)
{
    LogHandler::Dialog(error, XLogLevel::Critical);
}

QString MainWindow::second_to_minutes(int seconds)
{
    int sec = seconds;
    QString mn = QString::number( (sec ) / 60);
    int _tmp_mn  = mn.toInt() * 60;
    QString sc= QString::number( (seconds - _tmp_mn  ) % 60 );

    return (mn.length() == 1 ? "0" + mn : mn ) + ":" + (sc.length() == 1 ? "0" + sc : sc);
}

void initSerial(SerialHandler* serialHandler, SerialComboboxItem serialInfo)
{
    if(!serialHandler->isRunning())
        serialHandler->init(serialInfo.portName);
}

void initNetwork(UdpHandler* udpHandler, NetworkAddress address)
{
    if(!udpHandler->isRunning())
        udpHandler->init(address);
}

void MainWindow::initSerialEvent()
{
    if (selectedDevice->isRunning())
    {
        selectedDevice->dispose();
    }
    if(initFuture.isRunning())
    {
        initFuture.cancel();
        initFuture.waitForFinished();
    }
    SettingsHandler::setSelectedDevice(DeviceType::Serial);
    selectedDevice = serialHandler;
    initFuture = QtConcurrent::run(initSerial, serialHandler, selectedSerialPort);
}

void MainWindow::initNetworkEvent()
{
    if (selectedDevice->isRunning())
    {
        selectedDevice->dispose();
    }
    if(initFuture.isRunning())
    {
        initFuture.cancel();
        initFuture.waitForFinished();
    }
    SettingsHandler::setSelectedDevice(DeviceType::Network);
    selectedDevice = udpHandler;
    if(SettingsHandler::getServerAddress() != "" && SettingsHandler::getServerPort() != "")
    {
        NetworkAddress address { ui->networkAddressTxt->text(),  ui->networkPortTxt->text().toInt() };
        initFuture = QtConcurrent::run(initNetwork, udpHandler, address);
    }
}

void MainWindow::on_serialOutputRdo_clicked()
{
    ui->SerialOutputCmb->setEnabled(true);;
    ui->networkAddressTxt->setEnabled(false);
    ui->networkPortTxt->setEnabled(false);
    initSerialEvent();
}

void MainWindow::on_networkOutputRdo_clicked()
{
    ui->SerialOutputCmb->setEnabled(false);
    ui->networkAddressTxt->setEnabled(true);
    ui->networkPortTxt->setEnabled(true);
    initNetworkEvent();
}

void MainWindow::on_networkAddressTxt_editingFinished()
{
    SettingsHandler::setServerAddress(ui->networkAddressTxt->text());
    on_networkOutputRdo_clicked();
}

void MainWindow::on_networkPortTxt_editingFinished()
{
    SettingsHandler::setServerPort(ui->networkPortTxt->text());
    on_networkOutputRdo_clicked();
}

void MainWindow::on_SerialOutputCmb_currentIndexChanged(int index)
{
    SerialComboboxItem serialInfo = ui->SerialOutputCmb->currentData(Qt::UserRole).value<SerialComboboxItem>();
    selectedSerialPort = serialInfo;
    if (SettingsHandler::getSelectedDevice() == DeviceType::Serial)
    {
        on_serialOutputRdo_clicked();
    }
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
                                                                    "Copyright 2020 Jason C. Fain<br>"
                                                                    "Donate: <a href='https://www.patreon.com/Khrull'>https://www.patreon.com/Khrull</a>");
    copyright.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&copyright);
    QLabel sources;
    sources.setText("This software uses libraries from:");
    sources.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&sources);
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
                     "Home page: <a href='http://qtav.org'>href='http://qtav.org</a>");
    qtAVInfo.setAlignment(Qt::AlignHCenter);
    layout.addWidget(&qtAVInfo);
    QLabel libAVInfo;
    libAVInfo.setFrameStyle(QFrame::Panel | QFrame::Sunken);
    libAVInfo.setText("<b>Libav</b><br>"
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
