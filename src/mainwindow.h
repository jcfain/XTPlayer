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
#include <QtAV>
#include <QtAVWidgets>
#include <QtGlobal>
#include "CustomControls/xvideowidget.h"
//#include "CustomControls/xmediaplayer.h"
#include "CustomControls/RangeSlider.h"
#include "lib/handler/settingshandler.h"
#include "lib/handler/loghandler.h"
#include "lib/handler/funscripthandler.h"
#include "lib/handler/tcodehandler.h"
#include "lib/handler/serialhandler.h"
#include "lib/struct/LibraryListItem.h"
#include "lib/struct/SerialComboboxItem.h"
#include "lib/struct/ConnectionChangedSignal.h"
#include "lib/tool/xmath.h"
#include "lib/lookup/enum.h"

Q_DECLARE_METATYPE(LibraryListItem);
Q_DECLARE_METATYPE(SerialComboboxItem);

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

using namespace QtAV;

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


    void on_SerialOutputCmb_currentIndexChanged(int index);

    void on_serialRefreshBtn_clicked();

    void on_serialOutputRdo_clicked();


    void on_networkOutputRdo_clicked();

    void on_networkAddressTxt_editingFinished();

    void on_networkPortTxt_editingFinished();

signals:
    void keyPressed(QKeyEvent * event);
protected:
    virtual void keyPressEvent(QKeyEvent *event) override
    {
        emit keyPressed(event);
    }

private:
    Ui::MainWindow *ui;
    //QMediaPlayer* player;
    AVPlayer* player;
    //XVideoWidget* vw;
    VideoOutput* vw;
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
    TCodeHandler* tcodeHandler;
    SerialHandler* serialHandler;
    QVector<SerialComboboxItem> serialPorts;

    QVector<QString> videos;
    void on_load_library(QString path);
    void on_libray_path_select(QString path);
    void media_double_click_event(QMouseEvent * event);
    QString second_to_minutes(int seconds);
    bool isPlayingFile(QString file);
    void togglePause();
    void toggleFullScreen();
    void loadSerialPorts();
    void playFile(LibraryListItem selectedFileListItem);

    void on_seekSlider_sliderMoved(int position);
    void on_key_press(QKeyEvent* event);
    void on_media_error(QMediaPlayer::Error error);
    void on_media_positionChanged(qint64 position);
    void media_single_click_event(QMouseEvent * event);
    void on_media_statusChanged(MediaStatus status);
    void on_device_connectionChanged(ConnectionChangedSignal event);
    void on_device_error(QString error);
    void donate();
    //void on_output_radioChanged(QRadio event);
};
#endif // MAINWINDOW_H
