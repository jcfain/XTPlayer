#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QFileDialog>
#include <QProgressBar>
#include <QListWidgetItem>
#include <QMessageBox>
#include <iostream>
#include <QtAV>
#include <QtGlobal>
#include <QFuture>
#include <QDesktopServices>
#include "lib/handler/videohandler.h"
#include "CustomControls/RangeSlider.h"
#include "lib/handler/settingshandler.h"
#include "lib/handler/loghandler.h"
#include "lib/handler/funscripthandler.h"
#include "lib/handler/tcodehandler.h"
#include "lib/handler/serialhandler.h"
#include "lib/handler/udphandler.h"
#include "lib/handler/devicehandler.h"
#include "lib/struct/LibraryListItem.h"
#include "lib/struct/SerialComboboxItem.h"
#include "lib/struct/ConnectionChangedSignal.h"
#include "lib/tool/xmath.h"
#include "lib/lookup/enum.h"

Q_DECLARE_METATYPE(LibraryListItem);
Q_DECLARE_METATYPE(SerialComboboxItem);
//Q_DECLARE_METATYPE(DeviceType);
//Q_DECLARE_METATYPE(ConnectionStatus);


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


extern void syncFunscript(AVPlayer* player, SerialHandler* serialHandler, UdpHandler* udphandler, TCodeHandler* tcodeHandler, FunscriptHandler* funscriptHandler);
extern void initSerial(SerialHandler* serialHandler, SerialComboboxItem serialInfo);
extern void initNetwork(UdpHandler* serialHandler, NetworkAddress address);

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

    void onXRange_valueChanged(int value);
    void onYRollRange_valueChanged(int value);
    void onXRollRange_valueChanged(int value);
    void onOffSet_valueChanged(int value);

    void on_actionAbout_triggered();

    void on_actionDonate_triggered();

signals:
    void keyPressed(QKeyEvent * event);
    void sendTCode(QString tcode);
protected:
    virtual void keyPressEvent(QKeyEvent *event) override
    {
        emit keyPressed(event);
    }

private:
    Ui::MainWindow *ui;
    DeviceHandler* selectedDevice;
    QFuture<void> initFuture;
    QFuture<void> funscriptFuture;
    QProgressBar* bar;
    VideoHandler* videoHandler;
    QLabel* xRangeLabel;
    QLabel* yRollRangeLabel;
    QLabel* xRollRangeLabel;
    QLabel* offSetLabel;
    RangeSlider* xRangeSlider;
    RangeSlider* yRollRangeSlider;
    RangeSlider* xRollRangeSlider;
    RangeSlider* offSetSlider;
    QWidget* fullscreenWindow;
    FunscriptHandler* funscriptHandler;
    TCodeHandler* tcodeHandler;
    SerialHandler* serialHandler;
    UdpHandler* udpHandler;
    QList<SerialComboboxItem> serialPorts;
    SerialComboboxItem selectedSerialPort;
    QSize videoSize;
    QSize appSize;
    QPoint appPos;
    LibraryListItem selectedFileListItem;
    int selectedFileListIndex;
    bool deviceConnected;

    QList<QString> videos;
    void on_load_library(QString path);
    void on_libray_path_select(QString path);
    void media_double_click_event(QMouseEvent * event);
    QString second_to_minutes(int seconds);
    bool isPlayingFile(QString file);
    void togglePause();
    void toggleFullScreen();
    void toggleUI();
    void toggleControls();
    void setDeviceStatusStyle(ConnectionStatus status, DeviceType deviceType, QString message = "");

    void loadSerialPorts();
    void playFile(LibraryListItem selectedFileListItem);
    void initNetworkEvent();
    void initSerialEvent();

    void on_seekSlider_sliderMoved(int position);
    void on_key_press(QKeyEvent* event);
    void on_video_mouse_enter(QEvent* event);
    void on_media_positionChanged(qint64 position);
    void media_single_click_event(QMouseEvent * event);
    void on_media_statusChanged(QtAV::MediaStatus status);
    void on_media_start();
    void on_media_stop();
    void on_device_connectionChanged(ConnectionChangedSignal event);
    void on_device_error(QString error);
    void donate();
};
#endif // MAINWINDOW_H
