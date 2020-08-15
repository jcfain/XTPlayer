#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QFileDialog>
#include <QProgressBar>
#include <QListWidgetItem>
#include <QMessageBox>
#include <iostream>
#include <QMediaPlayer>
#include "CustomControls/xvideowidget.h"
//#include "CustomControls/xmediaplayer.h"
#include "CustomControls/RangeSlider.h"
#include "lib/handler/settingshandler.h"
#include "lib/handler/funscripthandler.h"
#include "lib/struct/LibraryListItem.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionSelect_library_triggered();

    void on_LibraryList_itemClicked(QListWidgetItem *item);

    void on_LibraryList_itemDoubleClicked(QListWidgetItem *item);

    void on_VolumeSlider_valueChanged(int value);

    void on_PlayBtn_clicked();

    void on_PauseBtn_clicked();

    void on_StopBtn_clicked();

    void on_MuteBtn_toggled(bool checked);

    void on_fullScreenBtn_clicked();

    void on_SeekSlider_sliderMoved(int position);

private:
    Ui::MainWindow *ui;
    QMediaPlayer* player;
    XVideoWidget* vw;
    QProgressBar* bar;
    QLabel* xRangeLabel;
    QLabel* yRollRangeLabel;
    QLabel* xRollRangeLabel;
    QLabel* SpeedLabel;
    RangeSlider* xRangeSlider;
    RangeSlider* yRollRangeSlider;
    RangeSlider* xRollRangeSlider;
    RangeSlider* SpeedSlider;
    QWindow* fullscreenWindow;
    FunscriptHandler* funscriptHandler;

    QVector<QString> videos;
    void on_load_library(QString path);
    void on_libray_path_select(QString path);
    void on_media_positionChanged(qint64 position);
    void media_double_click_event(QMouseEvent * event);
    void on_media_error(QMediaPlayer::Error error);
    QString second_to_minutes(int seconds);
    bool isPlayingFile(QString file) ;
};
#endif // MAINWINDOW_H
