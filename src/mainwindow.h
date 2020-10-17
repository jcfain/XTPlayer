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
#include <QtAVWidgets>
#include <QtGlobal>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QDesktopServices>
#include <QDirIterator>
#include <qfloat16.h>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QScreen>
#include <QMovie>
#include <QActionGroup>
#include <QToolTip>
#include <QTextToSpeech>
#include "settingsdialog.h"
#include "lib/handler/videohandler.h"
#include "CustomControls/rangeslider.h"
#include "lib/handler/settingshandler.h"
#include "lib/handler/loghandler.h"
#include "lib/handler/funscripthandler.h"
#include "lib/handler/tcodehandler.h"
#include "lib/handler/devicehandler.h"
#include "lib/handler/vrdevicehandler.h"
#include "lib/struct/LibraryListItem.h"
#include "lib/struct/SerialComboboxItem.h"
#include "lib/struct/ConnectionChangedSignal.h"
#include "lib/struct/librarylistwidgetitem.h"
#include "lib/lookup/AxisNames.h"
#include "lib/lookup/MediaActions.h"
#include "lib/tool/xmath.h"
#include "lib/lookup/enum.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


extern void syncFunscript(VideoHandler* player, SettingsDialog* xSettings, TCodeHandler* tcodeHandler, FunscriptHandler* funscriptHandler, QList<FunscriptHandler*> funscriptHandlers);
extern void syncVRFunscript(VRDeviceHandler* vrPlayer, VideoHandler* xPlayer, SettingsDialog* xSettings, TCodeHandler* tcodeHandler, FunscriptHandler* funscriptHandler);

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QStringList, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionSelect_library_triggered();

    void on_LibraryList_itemClicked(QListWidgetItem *item);

    void on_LibraryList_itemDoubleClicked(QListWidgetItem *item);

    void on_VolumeSlider_valueChanged(int value);

    void on_PlayBtn_clicked();

    void on_StopBtn_clicked();

    void on_MuteBtn_toggled(bool checked);

    void on_fullScreenBtn_clicked();
    void on_mainwindow_change(QEvent* event);

    void onLibraryList_ContextMenuRequested(const QPoint &pos);
    void onVideoHandler_togglePaused(bool paused);
    void playFileFromContextMenu();
    void playFileWithCustomScript();
    void regenerateThumbNail();
    void setThumbNailFromCurrent();
    void onVRMessageRecieved(VRPacket packet);
    void on_gamepad_sendTCode(QString tcode);
    void on_gamepad_sendAction(QString action);
    void on_gamepad_connectionChanged(ConnectionChangedSignal event);
    void on_actionAbout_triggered();
    void on_action75_triggered();
    void on_action100_triggered();
    void on_action125_triggered();
    void on_action150_triggered();
    void on_action175_triggered();
    void on_action200_triggered();
    void on_actionNameAsc_triggered();
    void on_actionNameDesc_triggered();
    void on_actionRandom_triggered();
    void on_actionCreatedAsc_triggered();
    void on_actionCreatedDesc_triggered();
    void dispose();

    void on_actionDonate_triggered();

    void on_actionSettings_triggered();

    void on_skipForwardButton_clicked();

    void on_skipBackButton_clicked();

    void on_actionThumbnail_triggered();

    void on_actionList_triggered();
    void on_seekslider_hover(int position, int time);
    void on_seekslider_leave();

    void on_SeekSlider_valueChanged(int value);
    void onLoopRange_valueChanged(int value);

    void on_actionChange_theme_triggered();

    void on_actionChange_current_deo_script_triggered();

    void on_settingsButton_clicked();

    void on_loopToggleButton_toggled(bool checked);

signals:
    void keyPressed(QKeyEvent * event);
    void change(QEvent * event);
    void sendTCode(QString tcode);
protected:
    virtual void keyPressEvent(QKeyEvent *event) override
    {
        emit keyPressed(event);
    }
    virtual void changeEvent(QEvent *event) override
    {
        emit change(event);
    }

private:
    Ui::MainWindow *ui;
    SettingsDialog* _xSettings;
    AxisNames _axisNames;
    QTextToSpeech* textToSpeech;
    VideoPreviewWidget* videoPreviewWidget;
    QFuture<void> funscriptFuture;
    QFuture<void> funscriptVRSyncFuture;
    QProgressBar* bar;
    VideoHandler* videoHandler;
    FunscriptHandler* funscriptHandler;
    QList<FunscriptHandler*> funscriptHandlers;
    TCodeHandler* tcodeHandler;
    bool _isMaximized = false;
    bool _isFullScreen = false;
    QSize _videoSize;
    QSize _appSize;
    QSize _defaultAppSize;
    QSize _currentThumbSize;
    QPoint _appPos;
    QMovie* _movie;
    int playingVideoListIndex;
    bool deviceConnected;
    bool autoLoopOn = false;
    QLabel* connectionStatusLabel;
    QLabel* gamepadConnectionStatusLabel;
    QPushButton* retryConnectionButton;
    QLabel* deoConnectionStatusLabel;
    QPushButton* deoRetryConnectionButton;
    QLabel* whirligigConnectionStatusLabel;
    QPushButton* whirligigRetryConnectionButton;
    int voulumeBeforeMute;
    QActionGroup* libraryViewGroup;
    QActionGroup* libraryThumbSizeGroup;
    QAction* action75_Size;
    QAction* action100_Size;
    QAction* action125_Size;
    QAction* action150_Size;
    QAction* action175_Size;
    QAction* action200_Size;
    QActionGroup* librarySortGroup;
    QAction* actionNameAsc_Sort;
    QAction* actionNameDesc_Sort;
    QAction* actionRandom_Sort;
    QAction* actionCreatedAsc_Sort;
    QAction* actionCreatedDesc_Sort;
    qint64 thumbCaptureTime;
    QWidget *normalWindowWidget;
    QWidget *fullScreenWidget;
    QGridLayout *fullScreenLayout;
    QFrame* playerControlsPlaceHolder;
    QGridLayout* placeHolderControlsGrid;
    bool funscriptFileSelectorOpen = false;
    bool stopThumbProcess = false;
    bool vrScriptSelectorCanceled = false;
    QString vrScriptSelectedCanceledPath;
    int thumbNailSearchIterator = 0;


    QList<LibraryListWidgetItem*> libraryItems;
    void on_load_library(QString path);
    void on_libray_path_select(QString path);
    void media_double_click_event(QMouseEvent * event);
    QString mSecondFormat(int seconds);
    bool isPlayingFile(QString file);

    void mediaAction(QString action);
    void toggleFullScreen();
    void toggleLoop();
    void toggleMediaControlStatus();
    void hideControls();
    void showControls();
    void saveThumb(const QString& videoFile, const QString& thumbFile, QListWidgetItem* qListWidgetItem, qint64 position = 0);
    void saveNewThumbs();
    void updateThumbSizeUI(int size);
    void updateLibrarySortUI(LibrarySortMode mode);
    void setThumbSize(int size);
    void changeDeoFunscript();

    void playVideo(LibraryListItem selectedFileListItem, QString funscript = nullptr);
    void initNetworkEvent();
    void initSerialEvent();
    void skipForward();
    void skipBack();
    void rewind();
    void fastForward();

    bool eventFilter(QObject *obj, QEvent *event) override;

    void setVolumeIcon(int volume);
    void on_seekSlider_sliderMoved(int position);
    void on_key_press(QKeyEvent* event);
    void on_controls_mouse_enter(QMouseEvent* event);
    void on_media_positionChanged(qint64 position);
    void media_single_click_event(QMouseEvent * event);
    void on_media_statusChanged(QtAV::MediaStatus status);
    void on_media_start();
    void on_media_stop();
    void on_device_connectionChanged(ConnectionChangedSignal event);
    void on_device_error(QString error);
    void on_deo_device_connectionChanged(ConnectionChangedSignal event);
    void on_deo_device_error(QString error);
    void on_whirligig_device_connectionChanged(ConnectionChangedSignal event);
    void on_whirligig_device_error(QString error);
    void donate();
};
#endif // MAINWINDOW_H
