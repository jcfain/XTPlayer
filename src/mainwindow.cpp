#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDirIterator>
#include <qfloat16.h>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    SettingsHandler::Load();

    //keyPress = new KeyPress();
    //keyPress->show();
    serialHandler = new SerialHandler(this);
    udpHandler = new UdpHandler(this);
    videoHandler = new VideoHandler(this);
    ui->MediaGrid->addWidget(videoHandler);
    loadSerialPorts();

    ui->SeekSlider->setDisabled(true);


    on_load_library(SettingsHandler::getSelectedLibrary());
    ui->VolumeSlider->setValue(SettingsHandler::getPlayerVolume());

    ui->SerialOutputCmb->setCurrentText(SettingsHandler::getSerialPort());
    ui->networkAddressTxt->setText(SettingsHandler::getServerAddress());
    ui->networkPortTxt->setText(SettingsHandler::getServerPort());
    if(SettingsHandler::getSelectedDevice() == DeviceType::Serial)
    {
        ui->serialOutputRdo->setChecked(true);
    }
    else if (SettingsHandler::getSelectedDevice() == DeviceType::Network)
    {
        ui->networkOutputRdo->setChecked(true);
    }

    QFont font( "Sans Serif", 7);

    xRangeLabel = new QLabel("X Range");
    xRangeLabel->setFont(font);
    ui->RangeSettingsGrid->addWidget(xRangeLabel, 0,0);
    xRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
    xRangeSlider->SetRange(1, 999);
    xRangeSlider->setLowerValue(SettingsHandler::getXMin());
    xRangeSlider->setUpperValue(SettingsHandler::getXMax());
    ui->RangeSettingsGrid->addWidget(xRangeSlider, 1,0);

    yRollRangeLabel = new QLabel("Y Roll Range");
    yRollRangeLabel->setFont(font);
    ui->RangeSettingsGrid->addWidget(yRollRangeLabel, 2,0);
    yRollRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
    yRollRangeSlider->SetRange(1, 999);
    yRollRangeSlider->setLowerValue(SettingsHandler::getYRollMin());
    yRollRangeSlider->setUpperValue(SettingsHandler::getYRollMax());
    ui->RangeSettingsGrid->addWidget(yRollRangeSlider, 3,0);

    xRollRangeLabel = new QLabel("X Roll Range");
    xRollRangeLabel->setFont(font);
    ui->RangeSettingsGrid->addWidget(xRollRangeLabel, 4,0);
    xRollRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
    xRollRangeSlider->SetRange(1, 999);
    xRollRangeSlider->setLowerValue(SettingsHandler::getXRollMin());
    xRollRangeSlider->setUpperValue(SettingsHandler::getXRollMax());
    ui->RangeSettingsGrid->addWidget(xRollRangeSlider, 5,0);

    offSetLabel = new QLabel("Offset: " + QString::number(SettingsHandler::getoffSet()));
    offSetLabel->setFont(font);
    ui->RangeSettingsGrid->addWidget(offSetLabel, 6,0);
    offSetSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::LeftHandle, nullptr);
    offSetSlider->SetRange(1, 2000);
    offSetSlider->setLowerValue(SettingsHandler::getoffSetMap());
    ui->RangeSettingsGrid->addWidget(offSetSlider, 7,0);


    connect(xRangeSlider, &RangeSlider::lowerValueChanged, this, &MainWindow::on_x_range_valueChanged);
    connect(xRangeSlider, &RangeSlider::upperValueChanged, this, &MainWindow::on_x_range_valueChanged);
    connect(yRollRangeSlider, &RangeSlider::lowerValueChanged, this, &MainWindow::on_yRoll_range_valueChanged);
    connect(yRollRangeSlider, &RangeSlider::upperValueChanged, this, &MainWindow::on_yRoll_range_valueChanged);
    connect(xRollRangeSlider, &RangeSlider::lowerValueChanged, this, &MainWindow::on_xRoll_range_valueChanged);
    connect(xRollRangeSlider, &RangeSlider::upperValueChanged, this, &MainWindow::on_xRoll_range_valueChanged);
    connect(offSetSlider, &RangeSlider::lowerValueChanged, this, &MainWindow::on_offSet_valueChanged);

    connect(videoHandler->player, &AVPlayer::positionChanged, this, &MainWindow::on_media_positionChanged);
    connect(videoHandler->player, &AVPlayer::mediaStatusChanged, this, &MainWindow::on_media_statusChanged);
    connect(videoHandler->player, &AVPlayer::started, this, &MainWindow::on_media_start);
    connect(ui->SeekSlider, &QSlider::sliderMoved, this, &MainWindow::on_seekSlider_sliderMoved);
    //connect(player, static_cast<void(AVPlayer::*)(AVPlayer::Error )>(&AVPlayer::error), this, &MainWindow::on_media_error);

    connect(videoHandler, &VideoHandler::doubleClicked, this, &MainWindow::media_double_click_event);
    //connect(vw, &XVideoWidget::singleClicked, this, &MainWindow::media_single_click_event);
    connect(this, &MainWindow::keyPressed, this, &MainWindow::on_key_press);
    connect(videoHandler, &VideoHandler::mouseEnter, this, &MainWindow::on_video_mouse_enter);
    connect(serialHandler, &SerialHandler::connectionChange, this, &MainWindow::on_device_connectionChanged);
    connect(serialHandler, &SerialHandler::errorOccurred, this, &MainWindow::on_device_error);
    connect(udpHandler, &UdpHandler::connectionChange, this, &MainWindow::on_device_connectionChanged);
    connect(udpHandler, &UdpHandler::errorOccurred, this, &MainWindow::on_device_error);
    if(SettingsHandler::getSelectedDevice() == DeviceType::Network)
    {
        initNetworkEvent();
    }
}

MainWindow::~MainWindow()
{
    SettingsHandler::Save();
    udpHandler->dispose();
    serialHandler->dispose();
    delete ui;
    delete videoHandler;
    //delete keyPress;
}

void  MainWindow::on_key_press(QKeyEvent * event)
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

void  MainWindow::on_video_mouse_enter(QEvent * event)
{
    if (isFullScreen())
    {
        toggleControls();
        QTimer::singleShot(2000, this, &MainWindow::toggleControls);
    }
}

void MainWindow::on_x_range_valueChanged()
{
    SettingsHandler::setXMin(xRangeSlider->GetLowerValue());
    SettingsHandler::setXMax(xRangeSlider->GetUpperValue());
}

void MainWindow::on_yRoll_range_valueChanged()
{
    SettingsHandler::setYRollMin(yRollRangeSlider->GetLowerValue());
    SettingsHandler::setYRollMax(yRollRangeSlider->GetUpperValue());
}

void MainWindow::on_xRoll_range_valueChanged()
{
    SettingsHandler::setXRollMin(xRollRangeSlider->GetLowerValue());
    SettingsHandler::setXRollMax(xRollRangeSlider->GetUpperValue());
}

void MainWindow::on_offSet_valueChanged()
{
    SettingsHandler::setoffSet(offSetSlider->GetLowerValue());
    offSetLabel->setText("Offset: " + QString::number(SettingsHandler::getoffSet()));
}

void MainWindow::on_load_library(QString path)
{
    if (!path.isNull() && !path.isEmpty()) {
        QDir directory(path);
        if (directory.exists()) {
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
            while (library.hasNext()) {
                QFileInfo fileinfo(library.next());
                QString videoPath = fileinfo.filePath();
                QString videoPathTemp = fileinfo.filePath();
                QString fileName = fileinfo.fileName();
                QString fileNameTemp = fileinfo.fileName();
                QString scriptFile = fileNameTemp.remove(fileNameTemp.lastIndexOf('.'), fileNameTemp.length() -  1) + ".funscript";
                QString scriptPath;
                if (SettingsHandler::getSelectedFunscriptLibrary() == Q_NULLPTR) {
                    scriptPath = videoPathTemp.remove(videoPathTemp.lastIndexOf('.'), videoPathTemp.length() -  1) + ".funscript";
                } else {
                    scriptPath = SettingsHandler::getSelectedFunscriptLibrary() + QDir::separator() + scriptFile;
                }
                if (funscriptHandler->exists(scriptPath))
                {
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
                else
                {
                    LogHandler::Debug("Script does not exist for video: " + videoPath);
                }
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
    MainWindow::playFile(selectedFileListItem);
}


void MainWindow::playFile(LibraryListItem selectedFileListItem)
{
    videoHandler->player->stop();
    QFile file(selectedFileListItem.path);
    if (file.exists())
    {
        videoHandler->player->setFile(selectedFileListItem.path);
        videoHandler->player->load();
        if(funscriptHandler->load(selectedFileListItem.script))
        {
            SettingsHandler::setSelectedFile(selectedFileListItem.path);
            //QUrl url = QUrl::fromLocalFile(selectedFileListItem.path);
            videoHandler->player->play();
            selectedFileListIndex = ui->LibraryList->currentRow();
        }
        else
        {
            LogHandler::Dialog("Error loading '" + selectedFileListItem.script + "'!", XLogLevel::Critical);
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
    videoHandler->player->togglePause();
}

#include <QScreen>
void MainWindow::toggleFullScreen()
{
    toggleUI();
    if(videoHandler->player->state() == AVPlayer::PlayingState) {
        QScreen *screen = QGuiApplication::primaryScreen();
        QSize screenSize = screen->size();
        if(!isFullScreen()) {
            videoSize = videoHandler->size();
            appSize = size();
            appPos = pos();
            //videoHandler->setParent(this, Qt::Tool);
            setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
            //videoHandler->move(QPoint(0, 0));
            showFullScreen();
            //videoHandler->resize(QSize(screenSize.width()+1, screenSize.height()+1));
            videoHandler->resize(screenSize);
        }
        else
        {
            //videoHandler->setParent(this, Qt::Widget);
            setWindowFlags(Qt::Window);
            //videoHandler->resize(videoSize);
            showNormal();
            resize(appSize);
            move(appPos);
            ui->MediaGrid->addWidget(videoHandler);
        }
    }
}

void MainWindow::toggleUI()
{
    if(!ui->LibraryList->isHidden())
    {
        ui->LibraryList->hide();
        ui->menubar->hide();
    }
    else
    {
        ui->LibraryList->show();
        ui->menubar->show();
    }
    toggleControls();
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
    videoHandler->player->audio()->setVolume(value);

    ui->VolumeSlider->setToolTip(QString::number(value));

    SettingsHandler::setPlayerVolume(value);
}

void MainWindow::on_PlayBtn_clicked()
{
    if(selectedFileListItem.name != "") {
        MainWindow::playFile(selectedFileListItem);
    }
}

void MainWindow::on_PauseBtn_clicked()
{
    MainWindow::togglePause();
}

void MainWindow::on_StopBtn_clicked()
{
    if(videoHandler->player->PlayingState == AVPlayer::State::PlayingState) {
        videoHandler->player->stop();
    }
}

void MainWindow::on_MuteBtn_toggled(bool checked)
{
    videoHandler->player->audio()->setMute(checked);
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
    qint64 playerPosition = XMath::mapRange(static_cast<qint64>(position), 0, 100, 0, videoHandler->player->duration());
    ui->SeekSlider->setToolTip(QTime(0, 0, 0).addMSecs(position).toString(QString::fromLatin1("HH:mm:ss")));
    videoHandler->player->setPosition(playerPosition);
}

void MainWindow::on_media_positionChanged(qint64 position)
{
    ui->lblCurrentDuration->setText( second_to_minutes(position / 1000).append("/").append( second_to_minutes( (videoHandler->player->duration())/1000 ) ) );

    //ui->lblCurrentDuration->setText(QTime(0, 0, 0).addMSecs(position).toString(QString::fromLatin1("HH:mm:ss")));
    qint64 duration = videoHandler->player->duration();
    if (duration > 0)
    {
        qint64 sliderPosition = XMath::mapRange(position, 0, duration, 0, 100);
        ui->SeekSlider->setValue(static_cast<int>(sliderPosition));
    }
}

void MainWindow::on_media_start()
{
    ui->SeekSlider->setDisabled(false);
    QFuture<void> future = QtConcurrent::run(syncFunscript, videoHandler->player, serialHandler, udpHandler, tcodeHandler, funscriptHandler);
}

void syncFunscript(AVPlayer* player, SerialHandler* serialHandler, UdpHandler* udpHandler, TCodeHandler* tcodeHandler, FunscriptHandler* funscriptHandler)
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
    ui->SeekSlider->setValue(0);
    qDebug(">>>>>>>>>>>>>>disable slider");
    ui->SeekSlider->setDisabled(true);
    ui->lblCurrentDuration->setText("00:00:00");

}

void MainWindow::on_media_statusChanged(MediaStatus status)
{
    switch(videoHandler->player->mediaStatus())
    {
        case MediaStatus::EndOfMedia:
            ++selectedFileListIndex;
            if(selectedFileListIndex < videos.length())
            {
                ui->LibraryList->setCurrentRow(selectedFileListIndex);
                selectedFileListItem = ui->LibraryList->selectedItems()[0]->data(Qt::UserRole).value<LibraryListItem>();
                on_PlayBtn_clicked();
            }
        break;
    }
    /*
    switch (player->mediaStatus()) {
    case NoMedia:
        status = tr("No media");
        break;
    case InvalidMedia:
        status = tr("Invalid meida");
        break;
    case BufferingMedia:
        status = tr("Buffering...");
        break;
    case BufferedMedia:
        status = tr("Buffered");
        break;
    case LoadingMedia:
        status = tr("Loading...");
        break;
    case LoadedMedia:
        status = tr("Loaded");
        break;
    case StalledMedia:
        status = tr("Stalled");
        break;
    default:
        status = QString();
        onStopPlay();
        break;
        */
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
        ui->serialStatuslbl->setText(event.message);
        SettingsHandler::setSerialPort(ui->SerialOutputCmb->currentText());
    }
    else if (event.deviceType == DeviceType::Network)
    {
        ui->networkStatuslbl->setText(event.message);
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
    QtConcurrent::run(initSerial, serialHandler, selectedSerialPort);
}

void MainWindow::initNetworkEvent()
{
    if(SettingsHandler::getServerAddress() != "" && SettingsHandler::getServerPort() != "")
    {
        NetworkAddress address { ui->networkAddressTxt->text(),  ui->networkPortTxt->text().toInt() };
        QtConcurrent::run(initNetwork, udpHandler, address);
    }
}

void MainWindow::on_serialOutputRdo_clicked()
{
    SettingsHandler::setSelectedDevice(DeviceType::Serial);
    udpHandler->dispose();
    ui->SerialOutputCmb->setEnabled(false);;
    ui->networkAddressTxt->setEnabled(false);
    ui->networkPortTxt->setEnabled(false);
    initSerialEvent();
}

void MainWindow::on_networkOutputRdo_clicked()
{
    SettingsHandler::setSelectedDevice(DeviceType::Network);
    serialHandler->dispose();
    ui->SerialOutputCmb->setEnabled(true);
    ui->networkAddressTxt->setEnabled(true);
    ui->networkPortTxt->setEnabled(true);
    initNetworkEvent();
}

void MainWindow::on_networkAddressTxt_editingFinished()
{
    SettingsHandler::setServerAddress(ui->networkAddressTxt->text());
    initNetworkEvent();
}

void MainWindow::on_networkPortTxt_editingFinished()
{
    SettingsHandler::setServerPort(ui->networkPortTxt->text());
    initNetworkEvent();
}

void MainWindow::on_SerialOutputCmb_currentIndexChanged(int index)
{
    SerialComboboxItem serialInfo = ui->SerialOutputCmb->currentData(Qt::UserRole).value<SerialComboboxItem>();
    selectedSerialPort = serialInfo;
    if (SettingsHandler::getSelectedDevice() == DeviceType::Serial)
    {
        initSerialEvent();
    }
}

void MainWindow::donate()
{
    //QDesktopServices::openUrl(QUrl("https://sourceforge.net/p/qtav/wiki/Donate%20%E6%8D%90%E8%B5%A0/"));
    //QDesktopServices::openUrl(QUrl(QString::fromLatin1("http://www.qtav.org/donate.html")));
}
