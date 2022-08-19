#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWindow>
#include <QLabel>
#include <QFileDialog>
#include <QProgressBar>
#include <QListWidgetItem>
#include <QMessageBox>
#include <iostream>
#include <QtGlobal>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QDesktopServices>
#include <QDirIterator>
#include <qfloat16.h>
#include <QFuture>
#include <QScreen>
#include <QMovie>
#include <QActionGroup>
#include <QToolTip>
#include <QTextToSpeech>
#include <QScroller>
#include <QInputDialog>
#include <QSplashScreen>
#include <QModelIndex>

#include "xtengine.h"
#include "lib/struct/playlistviewmodel.h"
#include "lib/struct/librarylistviewmodel.h"
#include "lib/struct/librarysortfilterproxymodel.h"
#include "settingsdialog.h"
#include "librarywindow.h"
#include "addplaylistdialog.h"
#include "libraryItemSettingsDialog.h"
#include "noMatchingScriptDialog.h"
#include "playercontrols.h"
#include "welcomedialog.h"
#include "dlnascriptlinks.h"
#include "xlibrarylist.h"
#include "lib/handler/videohandler.h"
#include "CustomControls/rangeslider.h"
#include "lib/handler/outputdevicehandler.h"
#include "lib/handler/inputdevicehandler.h"
#include "lib/handler/xvideopreviewwidget.h"
#include "lib/struct/LibraryListItem27.h"
#include "lib/struct/SerialComboboxItem.h"
#include "lib/struct/ConnectionChangedSignal.h"
//#include <QtCompress/qzipwriter.h>
//#include <QtCompress/qzipreader.h>

#include "lib/lookup/AxisNames.h"
#include "lib/tool/xmath.h"
#include "lib/lookup/enum.h"
#include "lib/lookup/MediaActions.h"
#include "lib/handler/loghandler.h"
#include "lib/handler/settingshandler.h"



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QStringList, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionSelect_library_triggered();

    void on_LibraryList_itemClicked(QModelIndex index );

    void on_LibraryList_itemDoubleClicked(QModelIndex index );

    void on_VolumeSlider_valueChanged(int value);

    void on_PlayBtn_clicked();

    void stopMedia();

    void on_MuteBtn_toggled(bool checked);

    void on_fullScreenBtn_clicked();
    void on_mainwindow_change(QEvent* event);
    void on_mainwindow_splitterMove(int pos, int index);

    void onLibraryList_ContextMenuRequested(const QPoint &pos);
    void onLibraryWindowed_Clicked();
    void onLibraryWindowed_Closed();
    void on_togglePaused(bool paused);
    void playFileFromContextMenu();
    void playFileWithCustomScript();
    void playFileWithAudioSync();
    void regenerateThumbNail();
    void setThumbNailFromCurrent();
    void lockThumb();
    void unlockThumb();
    void onFunscriptSearchResult(QString mediaPath, QString funscriptPath, qint64 mediaDuration);
    void on_sendTCode(QString tcode);
    void on_gamepad_connectionChanged(ConnectionChangedSignal event);
    void on_actionAbout_triggered();
    void on_action75_triggered();
    void on_action100_triggered();
    void on_action125_triggered();
    void on_action150_triggered();
    void on_action175_triggered();
    void on_action200_triggered();
    void on_actionCustom_triggered();
    void on_actionNameAsc_triggered();
    void on_actionNameDesc_triggered();
    void on_actionRandom_triggered();
    void on_actionCreatedAsc_triggered();
    void on_actionCreatedDesc_triggered();
    void on_actionTypeAsc_triggered();
    void on_actionTypeDesc_triggered();
    void dispose();

    void on_actionDonate_triggered();

    void on_actionSettings_triggered();

    void on_skipForwardButton_clicked();

    void on_skipBackButton_clicked();

    void on_actionThumbnail_triggered();

    void on_actionList_triggered();
    void on_seekslider_hover(int position, int time);
    void on_seekslider_leave();

    void onLoopRange_valueChanged(int position, int startLoop, int endLoop);

    void on_actionChange_theme_triggered();

    void on_actionChange_current_deo_script_triggered();

    void on_settingsButton_clicked();

    void on_loopToggleButton_toggled(bool checked);

    void onPrepareLibraryLoad();
    void onLibraryLoadingStatusChange(QString message);
    void libraryListSetIconSize(QSize newSize);

    void on_audioLevel_Change(int decibelL, int decibelR);

    void onPasswordIncorrect();
    void on_scriptNotFound(QString message);
    void on_noScriptsFound(QString message);
    void on_playVideo(LibraryListItem27 selectedFileListItem, QString funscript = nullptr, bool audioSync = false);

    void on_actionReload_theme_triggered();

    void on_actionStored_DLNA_links_triggered();
    void onText_to_speech(QString message);
    void onPlaylistLoaded(LibraryListItem27 autoPlayItem = LibraryListItem27());
    void onBackFromPlaylistLoaded();
    void onRandomizeComplete();

signals:
    void keyPressed(QKeyEvent * event);
    void keyReleased(QKeyEvent * event);
    void change(QEvent * event);
//    void prepareLibraryLoad();
//    void libraryLoaded();
//    void libraryNotFound();
    void libraryIconResized(QSize newSize);
//    void libraryLoadingStatus(bool loading, QString state = nullptr);

//    void scriptNotFound(QString message);
//    void setLoading(bool loading);
    void playVideo(LibraryListItem27 selectedFileListItem, QString funscript = nullptr, bool audioSync = false);
    void stopAndPlayVideo(LibraryListItem27 selectedFileListItem, QString funscript = nullptr, bool audioSync = false);
    void playlistLoaded(LibraryListItem27 autoPlayItem = LibraryListItem27());
    void libraryItemFound(LibraryListItem27 item);
    void backFromPlaylistLoaded();
    void randomizeComplete();
protected:
    virtual void keyPressEvent(QKeyEvent *event) override
    {
        emit keyPressed(event);
    }
    virtual void keyReleaseEvent(QKeyEvent *event) override
    {
        emit keyReleased(event);
    }
    virtual void changeEvent(QEvent *event) override
    {
        emit change(event);
    }

private:
    //Hack because QTAV calls stopped and start out of order
    bool _mediaStopped = true;
    PasswordResponse _isPasswordIncorrect = PasswordResponse::INCORRECT;

    Ui::MainWindow *ui;
    XTEngine xtEngine;

    QMutex mutex;
    QString XTPVersion = "0.32a";
    float XTPVersionNum = 0.32f;
    SettingsDialog* _xSettings;
    WelcomeDialog* _welcomeDialog = 0;
    DLNAScriptLinks* _dlnaScriptLinksDialog;
    QFrame* _controlsHomePlaceHolderFrame;
    QGridLayout* _controlsHomePlaceHolderGrid;
    QFrame* _mediaFrame;
    QGridLayout* _mediaGrid;
    PlayerControls* _playerControlsFrame;
    LibraryWindow* libraryWindow;
    QSplashScreen* loadingSplash;
    QTextToSpeech* textToSpeech;
    XVideoPreviewWidget* _videoPreviewWidget;
    QFuture<void> loadingLibraryFuture;
    bool loadingLibraryStop = false;
    QFuture<void> _waitForStopFuture;
    bool _waitForStopFutureCancel = false;
    QProgressBar* bar;
    VideoHandler* videoHandler;
    bool _isMaximized = false;
    bool _isFullScreen = false;
    QSize _videoSize;
    QSize _appSize;
    QSize _defaultAppSize;
    QPoint _appPos;
    bool deviceConnected;
    QLabel* connectionStatusLabel;
    QLabel* gamepadConnectionStatusLabel;
    QLabel* libraryLoadingLabel;
    QLabel* libraryLoadingInfoLabel;
    QMovie* libraryLoadingMovie;
    QPushButton* retryConnectionButton;
    QLabel* deoConnectionStatusLabel;
    QPushButton* deoRetryConnectionButton;
    QLabel* vrConnectionStatusLabel;
    QPushButton* vrRetryConnectionButton;
    QLabel* xtpWebStatusLabel;
    XLibraryList* libraryList;
    PlaylistViewModel* _playListViewModel;
    LibrarySortFilterProxyModel* _librarySortFilterProxyModel;
    QPushButton* backLibraryButton;
    QPushButton* randomizeLibraryButton;
    QPushButton* windowedLibraryButton;
    QPushButton* savePlaylistButton;
    QPushButton* editPlaylistButton;
    QPushButton* cancelEditPlaylistButton;
    QLineEdit* libraryFilterLineEdit;
    QActionGroup* libraryViewGroup;
    QActionGroup* libraryThumbSizeGroup;
    QAction* action75_Size;
    QAction* action100_Size;
    QAction* action125_Size;
    QAction* action150_Size;
    QAction* action175_Size;
    QAction* action200_Size;
    QAction* actionCustom_Size;
    QActionGroup* librarySortGroup;
    QAction* actionNameAsc_Sort;
    QAction* actionNameDesc_Sort;
    QAction* actionRandom_Sort;
    QAction* actionCreatedAsc_Sort;
    QAction* actionCreatedDesc_Sort;
    QAction* actionTypeAsc_Sort;
    QAction* actionTypeDesc_Sort;
    qint64 thumbCaptureTime;
    QWidget *normalWindowWidget;
    QWidget *fullScreenWidget;
    QGridLayout *fullScreenLayout;
    QFrame* playerControlsPlaceHolder;
    QGridLayout* placeHolderControlsGrid;
    QFrame* playerLibraryPlaceHolder;
    QGridLayout* placeHolderLibraryGrid;
    bool libraryOverlay = false;
    bool funscriptFileSelectorOpen = false;
    bool thumbProcessIsRunning = false;
    bool vrScriptSelectorCanceled = false;
    bool vrScriptSelectorRunning = false;
    QString lastVRScriptPath;
    int thumbNailSearchIterator = 0;
    QMutex _eventLocker;
    bool _editPlaylistMode = false;
    bool _libraryDockMode = false;
    QString _lastKeyboardTCode;

    void on_settingsMessageRecieve(QString message, XLogLevel logLevel);
//    void saveSingleThumb(LibraryListWidgetItem* qListWidgetItem, qint64 position = 0);
//    void saveThumb(LibraryListWidgetItem* qListWidgetItem, qint64 position = 0, bool vrMode = false);
    void onSaveNewThumbLoading(LibraryListItem27 item);
    void onSaveNewThumb(LibraryListItem27 item, bool vrMode, QString thumbFile);
    void onSaveThumbError(LibraryListItem27 item, bool vrMode, QString errorMessage);
//    void startThumbProcess(bool vrMode = false);
//    void stopThumbProcess();
//    void saveNewThumbs(bool vrMode = false);

//    LibraryListItem27 createLibraryListItem27FromFunscript(QString funscript);
    bool isLibraryLoading();
    QString selectedPlaylistName;
    int playingLibraryListIndex;
    LibraryListItem27 playingLibraryListItem;
    int selectedLibraryListIndex;
    LibraryListItem27 selectedLibraryListItem;

    LibraryListItem27 getSelectedLibraryListItem();
    void onLibraryNotFound();
//    void on_load_library(QString path, bool vrMode);
    void openWelcomeDialog();
    void closeWelcomeDialog();
    void backToMainLibrary();
    void loadPlaylistIntoLibrary(QString playlistName, bool autoPlay = false);
    void media_double_click_event(QMouseEvent * event);
    QString mSecondFormat(int seconds);
    bool isPlayingFile(QString file);
    void savePlaylist();
    void editPlaylist();
    void renamePlaylist();
    void cancelEditPlaylist();
    void removeFromPlaylist();
    void deleteSelectedPlaylist();
    QString getPlaylistName(bool newPlaylist = true);
    void loadTheme(QString cssFilePath);
    void sortLibraryList(LibrarySortMode sortMode);

    void changeLibraryDisplayMode(LibraryView value);

    void deviceHome();
    void deviceSwitchedHome();
    void onSetLibraryLoaded();
    void onSetLibraryLoading();
    void toggleLibraryLoading(bool loading);
    void mediaAction(QString action, QString actionText);
    void toggleFullScreen();
    void toggleLoop();
    void hideControls();
    void showControls();
    void hideLibrary();
    void showLibrary();
    void setLibraryToolBar();
    void updateThumbSizeUI(int size);
    void updateLibrarySortUI(LibrarySortMode mode);
    void setThumbSize(int size);
    void resizeThumbs(int size);
    void changeDeoFunscript();
    void turnOffAudioSync();

    void stopAndPlayMedia(LibraryListItem27 selectedFileListItem, QString customScript = nullptr, bool audioSync = false);
    //void waitForMediaTostop(LibraryListItem27 selectedFileListItem, QString customScript = nullptr, bool audioSync = false, bool autoPlay = true);
    void initNetworkEvent();
    void initSerialEvent();
    void skipForward();
    void skipToMoneyShot();
    void skipToNextAction();
    void skipBack();
    void rewind();
    void fastForward();
    LibraryListItem27 setCurrentLibraryRow(int row);
//    void syncFunscript();
//    void syncVRFunscript();

    bool eventFilter(QObject *obj, QEvent *event) override;

    bool isPlaylistMode();
    void addSelectedLibraryItemToPlaylist(QString playlistName, LibraryListItem27 item);
    void setVolumeIcon(int volume);
    void on_seekSlider_sliderMoved(int position);
    void on_key_press(QKeyEvent* event);
    void on_controls_mouse_enter(QMouseEvent* event);
    void on_media_positionChanged(qint64 position);
    void media_single_click_event(QMouseEvent * event);
    void on_media_statusChanged(XMediaStatus status);
    void on_media_start();
    void on_media_stop();
    void on_standaloneFunscript_start();
    void on_standaloneFunscript_stop();
    void on_output_device_connectionChanged(ConnectionChangedSignal event);
    void on_input_device_connectionChanged(ConnectionChangedSignal event);
    void donate();
    void showInGraphicalShell(QString path);
    void onSetMoneyShot(LibraryListItem27 selectedLibraryListItem27, qint64 currentPosition);
    void onAddBookmark(LibraryListItem27 LibraryListItem27, QString name, qint64 currentPosition);
    void processMetaData(LibraryListItem27 LibraryListItem27);
    void updateMetaData(LibraryListItem27 LibraryListItem27);
    void processVRMetaData(QString videoPath, QString funscriptPath, qint64 duration);
};
extern void startThumbProcess(MainWindow* mainWindow);
#endif // MAINWINDOW_H
