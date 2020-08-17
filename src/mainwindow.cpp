#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDirIterator>
#include <qfloat16.h>

LibraryListItem selectedFileListItem;
int selectedFileListIndex;
int preFullScreenWidth;
int preFullScreenHeight;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    SettingsHandler::Load();

    //keyPress = new KeyPress();
    //keyPress->show();
    serialHandler = new SerialHandler();
    loadSerialPorts();
    player = new QMediaPlayer(this);
    vw = new XVideoWidget(this);
    player->setVideoOutput(vw);
    vw->show();
    ui->MediaGrid->addWidget(vw);

    on_load_library(SettingsHandler::selectedLibrary);
    ui->VolumeSlider->setValue(SettingsHandler::playerVolume);
    player->setVolume(SettingsHandler::playerVolume);
    ui->SerialOutputCmb->setCurrentText(SettingsHandler::serialPort);
    ui->networkAddressTxt->setText(SettingsHandler::serverAddress);
    ui->networkPortTxt->setText(SettingsHandler::serverPort);
    if(SettingsHandler::selectedDevice == DeviceType::Serial)
    {
        ui->serialOutputRdo->setChecked(true);
    }
    else if (SettingsHandler::selectedDevice == DeviceType::Network)
    {
        ui->networkOutputRdo->setChecked(true);
    }

    QFont font( "Sans Serif", 7);

    xRangeLabel = new QLabel("X Range");
    xRangeLabel->setFont(font);
    ui->RangeSettingsGrid->addWidget(xRangeLabel, 0,0);
    xRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
    xRangeSlider->SetRange(1, 999);
    ui->RangeSettingsGrid->addWidget(xRangeSlider, 1,0);

    yRollRangeLabel = new QLabel("Y Roll Range");
    yRollRangeLabel->setFont(font);
    ui->RangeSettingsGrid->addWidget(yRollRangeLabel, 2,0);
    yRollRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
    yRollRangeSlider->SetRange(1, 999);
    ui->RangeSettingsGrid->addWidget(yRollRangeSlider, 3,0);

    xRollRangeLabel = new QLabel("X Roll Range");
    xRollRangeLabel->setFont(font);
    ui->RangeSettingsGrid->addWidget(xRollRangeLabel, 4,0);
    xRollRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
    xRollRangeSlider->SetRange(1, 999);
    ui->RangeSettingsGrid->addWidget(xRollRangeSlider, 5,0);

    SpeedLabel = new QLabel("Speed");
    SpeedLabel->setFont(font);
    ui->RangeSettingsGrid->addWidget(SpeedLabel, 6,0);
    SpeedSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::LeftHandle, nullptr);
    SpeedSlider->SetRange(1000, 4000);
    ui->RangeSettingsGrid->addWidget(SpeedSlider, 7,0);


    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_media_positionChanged);
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::on_media_statusChanged);
    connect(ui->SeekSlider, &QSlider::sliderMoved, this, &MainWindow::on_seekSlider_sliderMoved);
    connect(player, static_cast<void(QMediaPlayer::*)(QMediaPlayer::Error )>(&QMediaPlayer::error), this, &MainWindow::on_media_error);

    connect(vw, &XVideoWidget::doubleClicked, this, &MainWindow::media_double_click_event);
    //connect(vw, &XVideoWidget::singleClicked, this, &MainWindow::media_single_click_event);
    //connect(this, &MainWindow::keyPressed, this, &MainWindow::on_key_press);
    connect(vw, &XVideoWidget::keyPressed, this, &MainWindow::on_key_press);
    connect(serialHandler, &SerialHandler::connectionChange, this, &MainWindow::on_device_connectionChanged);
    connect(serialHandler, &SerialHandler::errorOccurred, this, &MainWindow::on_device_error);
}

MainWindow::~MainWindow()
{
    SettingsHandler::Save();
    delete ui;
    delete player;
    delete vw;
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

void MainWindow::on_media_error(QMediaPlayer::Error error)
{
    QString message;
    switch(error)
    {
        case QMediaPlayer::Error::FormatError:
            message = "The format of a media resource isn't (fully) supported. Playback may still be possible, but without an audio or video component.";
            break;
        case QMediaPlayer::Error::ResourceError:
            message = "A media resource couldn't be resolved.";
            break;
        case QMediaPlayer::Error::NetworkError:
            message = "A network error occurred.";
            break;
        case QMediaPlayer::Error::AccessDeniedError:
            message = "There are not the appropriate permissions to play a media resource.";
            break;
        case QMediaPlayer::Error::ServiceMissingError:
            message = "A valid playback service was not found, playback cannot proceed.";
            break;
        case QMediaPlayer::Error::MediaIsPlaylist:
            message = "Media is a playlist";
            break;
        case QMediaPlayer::Error::NoError:
            break;
    }
    if (message != Q_NULLPTR)
    {
        LogHandler::Dialog(message, LogLevel::Critical);
    }
}

void MainWindow::on_load_library(QString path)
{
    if (!path.isNull()) {
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
                if (SettingsHandler::selectedFunscriptLibrary == Q_NULLPTR) {
                    scriptPath = videoPathTemp.remove(videoPathTemp.lastIndexOf('.'), videoPathTemp.length() -  1) + ".funscript";
                } else {
                    scriptPath = SettingsHandler::selectedFunscriptLibrary + QDir::separator() + scriptFile;
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
           LogHandler::Dialog("Lobrary path '" + path + "' does not exist", LogLevel::Critical);
        }
    }
    else
    {
       LogHandler::Dialog("Please select a library from the menu!", LogLevel::Critical);
    }
}

void MainWindow::on_libray_path_select(QString path)
{
    this->on_load_library(path);
}

void MainWindow::on_actionSelect_library_triggered()
{
    QString selectedLibrary = QFileDialog::getExistingDirectory(this, tr("Choose library"), ".", QFileDialog::ReadOnly);
    if (selectedLibrary != Q_NULLPTR) {
        on_libray_path_select(selectedLibrary);

        SettingsHandler::selectedLibrary = selectedLibrary;
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
    player->stop();
    QFile file(selectedFileListItem.path);
    if (file.exists())
    {
        if(funscriptHandler->load(selectedFileListItem.script))
        {
            SettingsHandler::selectedFile = selectedFileListItem.path;
            QUrl url = QUrl::fromLocalFile(selectedFileListItem.path);
            player->setMedia(url);
            player->play();
            selectedFileListIndex = ui->LibraryList->currentRow();
        }
        else
        {
            LogHandler::Dialog("Error loading '" + selectedFileListItem.script + "'!", LogLevel::Critical);
        }
    }
    else {
        LogHandler::Dialog("File '" + selectedFileListItem.path + "' does not exist!", LogLevel::Critical);
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
    if(player->state() == QMediaPlayer::PlayingState) {
        player->pause();
    } else {
        player->play();
    }
}

void MainWindow::toggleFullScreen()
{
    if(player->state() == QMediaPlayer::PlayingState) {
        if(!vw->isFullScreen()) {
            preFullScreenWidth = vw->width();
            preFullScreenHeight = vw->height();
            vw->setParent(this, Qt::Tool);
            vw->showFullScreen();
        }
        else {
            vw->setParent(this, Qt::Widget);
            ui->MediaGrid->addWidget(vw);
            vw->resize(preFullScreenWidth, preFullScreenHeight);
            vw->showNormal();
        }
    }
}

void MainWindow::on_VolumeSlider_valueChanged(int value)
{
    player->setVolume(value);
    SettingsHandler::playerVolume = value;
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
    if(player->PlayingState == QMediaPlayer::State::PlayingState) {
        player->stop();
    }
}

void MainWindow::on_MuteBtn_toggled(bool checked)
{
    player->setMuted(checked);
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
    qint64 playerPosition = XMath::mapRange(static_cast<qint64>(position), 0, 100, 0, player->duration());
    player->setPosition(playerPosition);
}

void MainWindow::on_media_positionChanged(qint64 position)
{
    ui->lblCurrentDuration->setText( second_to_minutes(position / 1000).append("/").append( second_to_minutes( (player->duration())/1000 ) ) );
    qint64 duration = player->duration();
    if (duration > 0)
    {
        serialHandler->sendTCode(tcodeHandler->funscriptToTCode(funscriptHandler->getPosition(position)));
        qint64 sliderPosition = XMath::mapRange(position, 0, duration, 0, 100);
        ui->SeekSlider->setValue(static_cast<int>(sliderPosition));
    }
}

void MainWindow::on_media_statusChanged(QMediaPlayer::MediaStatus status)
{
    switch(status)
    {
        case QMediaPlayer::MediaStatus::EndOfMedia:
            ++selectedFileListIndex;
            if(selectedFileListIndex < videos.length())
            {
                ui->LibraryList->setCurrentRow(selectedFileListIndex);
                selectedFileListItem = ui->LibraryList->selectedItems()[0]->data(Qt::UserRole).value<LibraryListItem>();
                on_PlayBtn_clicked();
            }
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
    if(event.deviceType == DeviceType::Serial)
    {
        ui->serialStatuslbl->setText(event.message);
    }
    else if (event.deviceType == DeviceType::Network)
    {
        ui->networkStatuslbl->setText(event.message);
    }
}

void MainWindow::on_device_error(QString error)
{
    LogHandler::Dialog(error, LogLevel::Critical);
}

QString MainWindow::second_to_minutes(int seconds)
{
    int sec = seconds;
    QString mn = QString::number( (sec ) / 60);
    int _tmp_mn  = mn.toInt() * 60;
    QString sc= QString::number( (seconds - _tmp_mn  ) % 60 );

    return (mn.length() == 1 ? "0" + mn : mn ) + ":" + (sc.length() == 1 ? "0" + sc : sc);
}

void MainWindow::on_SerialOutputCmb_currentIndexChanged(int index)
{
    if (SettingsHandler::selectedDevice == DeviceType::Serial)
    {
        SerialComboboxItem serialInfo = ui->SerialOutputCmb->currentData(Qt::UserRole).value<SerialComboboxItem>();
        SettingsHandler::serialPort = serialInfo.friendlyName;
        serialHandler->init(serialInfo);
    }
}

void MainWindow::on_serialOutputRdo_clicked()
{
    SettingsHandler::selectedDevice = DeviceType::Serial;
}

void MainWindow::on_networkOutputRdo_clicked()
{
    SettingsHandler::selectedDevice = DeviceType::Network;
}

void MainWindow::on_networkAddressTxt_editingFinished()
{
    SettingsHandler::serverAddress = ui->networkAddressTxt->text();
}

void MainWindow::on_networkPortTxt_editingFinished()
{
    SettingsHandler::serverPort = ui->networkPortTxt->text();
}
