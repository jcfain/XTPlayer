#include "mainwindow.h"
#include "ui_mainwindow.h"
int preFullScreenWidth;
int preFullScreenHeight;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    SettingsHandler::Load();


    on_load_library(SettingsHandler::selectedLibrary);
    player = new QMediaPlayer(this);
    vw = new XVideoWidget(this);
    player->setVideoOutput(vw);
    vw->show();

    ui->MediaGrid->addWidget(vw);
    ui->VolumeSlider->setValue(SettingsHandler::playerVolume);
    player->setVolume(SettingsHandler::playerVolume);

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


    connect(player, &QMediaPlayer::durationChanged, ui->SeekSlider, &QSlider::setMaximum);
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_media_positionChanged);
    //connect(player, static_cast<void(QMediaPlayer::*)(QMediaPlayer::Error )>(&QMediaPlayer::error), this, &MainWindow::on_media_error);
    //connect(player, &QMediaPlayer::error, this, &MainWindow::on_media_error);

    //connect(vw,SIGNAL(mouseDoubleClickEvent(QMouseEvent * event)), this, SLOT(mouse_double_click_event(QMouseEvent * event)));
    connect(vw, &XVideoWidget::doubleClicked, this, &MainWindow::media_double_click_event);
}

MainWindow::~MainWindow()
{
    SettingsHandler::Save();
    delete ui;
    delete player;
    delete vw;
}

void on_media_error(QMediaPlayer::Error error)
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
    if (!message.isNull())
    {
        QMessageBox messageBox;
        messageBox.critical(0,"Error","An error has occured !");
        messageBox.setFixedSize(500,200);
    }
}

void MainWindow::on_load_library(QString path)
{
    if (!path.isNull()) {
        QDir directory(path);
        if (directory.exists()) {
            ui->LibraryList->clear();
            QStringList videos = directory.entryList(QStringList() << "*.mp4" << "*.MP4", QDir::Files);
            foreach(QString filename, videos)
            {
                MainWindow::videos.push_front(filename);
                ui->LibraryList->addItem(filename);
            }
        }
    }
}

void MainWindow::on_libray_path_select(QString path)
{
    this->on_load_library(path);
}

void MainWindow::on_actionSelect_library_triggered()
{
    QString selectedLibrary = QFileDialog::getExistingDirectory(this, tr("Choose library"), ".", QFileDialog::ReadOnly);
    if (!selectedLibrary.isNull()) {
        on_libray_path_select(selectedLibrary);

        SettingsHandler::selectedLibrary = selectedLibrary;
    }
}

void MainWindow::on_LibraryList_itemClicked(QListWidgetItem *item)
{

}

void MainWindow::on_LibraryList_itemDoubleClicked(QListWidgetItem *item)
{
    auto selectedFileTemp = SettingsHandler::selectedLibrary + QDir::separator() + item->text();
    player->stop();
    SettingsHandler::selectedFile = selectedFileTemp;
    QFile file(SettingsHandler::selectedFile);
    if (file.exists())
    {

        QUrl url = QUrl::fromLocalFile(SettingsHandler::selectedFile);
        player->setMedia(url);
        player->play();
    }
}


bool MainWindow::isPlayingFile(QString file)
{
    return player->PlayingState == QMediaPlayer::State::PlayingState && SettingsHandler::selectedFile.compare(file) > -1;
}


void MainWindow::on_VolumeSlider_valueChanged(int value)
{
    player->setVolume(value);
    SettingsHandler::playerVolume = value;
}

void MainWindow::on_PlayBtn_clicked()
{
    if(player->PlayingState != QMediaPlayer::State::PlayingState) {
        player->play();
    }
}

void MainWindow::on_PauseBtn_clicked()
{
    if(player->PlayingState == QMediaPlayer::State::PlayingState) {
        player->pause();
    } else {
        player->play();
    }
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
//#include <QScreen>
void MainWindow::on_fullScreenBtn_clicked()
{
    if(player->state() == QMediaPlayer::PlayingState) {
        if(!vw->isFullScreen()) {
            preFullScreenWidth = vw->width();
            preFullScreenHeight = vw->height();
            vw->setParent(this, Qt::Tool);
            //ui->MediaGrid->removeWidget(vw);
            //QSize rec = MainWindow::screen()->availableSize();
            //vw->resize(rec.width(), rec.height());
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

void MainWindow::on_SeekSlider_sliderMoved(int position)
{
    player->setPosition(position);
}

void MainWindow::on_media_positionChanged(qint64 position)
{
    ui->lblCurrentDuration->setText( second_to_minutes(position / 1000).append("/").append( second_to_minutes( (player->duration())/1000 ) ) );
}


void MainWindow::media_double_click_event(QMouseEvent * event)
{
    if ( event->button() == Qt::LeftButton )
    {
        MainWindow::on_fullScreenBtn_clicked();
    }
}


QString MainWindow::second_to_minutes(int seconds)
{
    int sec = seconds;
    QString mn = QString::number( (sec ) / 60);
    int _tmp_mn  = mn.toInt() * 60;
    QString sc= QString::number( (seconds- _tmp_mn  ) % 60 );

    return (mn.length() == 1 ? "0" + mn : mn ) + ":" + (sc.length() == 1 ? "0" + sc : sc);
}
