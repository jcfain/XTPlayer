#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include<ui_settings.h>
#include <QDialog>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QSpinBox>
#include <QProgressBar>
#include <QCryptographicHash>
#include <QInputDialog>
#include "CustomControls/rangeslider.h"
#include "inputmapwidget.h"
#include "lib/struct/SerialComboboxItem.h"
#include "lib/struct/channeltableviewmodel.h"
#include "lib/struct/channeltablecomboboxdelegate.h"
#include "lib/struct/DecoderModel.h"
#include "lib/lookup/MediaActions.h"
#include "lib/lookup/xvideorenderer.h"
#include "lib/tool/simplecrypt.h"
#include "lib/handler/settingshandler.h"
#include "lib/handler/serialhandler.h"
#include "lib/handler/videohandler.h"
#include "lib/handler/udphandler.h"
#include "lib/handler/deohandler.h"
#include "lib/handler/whirligighandler.h"
#include "lib/handler/xtpwebhandler.h"
#include "lib/handler/gamepadhandler.h"
#include "lib/handler/funscripthandler.h"
#include "lib/handler/httphandler.h"
#include "lib/handler/synchandler.h"
#include "lib/handler/connectionhandler.h"
#include "libraryexclusions.h"
#include "addchanneldialog.h"


class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog();

    void init(VideoHandler* videoHandler, SyncHandler* syncHandler, TCodeHandler* tcodeHandler, ConnectionHandler* connectionHandler);
    void initLive();
    void dispose();
    void setAxisProgressBar(QString axis, int value);
    void resetAxisProgressBars();
    void latestYoutubeDownloaded();
    PasswordResponse CheckPass(QString pass);
    PasswordResponse GetLaunchPass();
    bool HasLaunchPass();

    void Export(QWidget* parent);
    void Import(QWidget* parent);
    void requestRestart(QWidget* parent);
    void askRestart(QWidget* parent, QString message);
    void quit(bool restart);
    void restart();

    void reject() override;

signals:
    void TCodeHomeClicked();
    void onAxisValueChange(QString axis, int value);
    void onAxisValueReset();
    void onOpenWelcomeDialog();
    void messageSend(QString message, XLogLevel loglevel);
    void skipToMoneyShot();
    void skipToNextAction();

public slots:
    void on_xtpWeb_initInputDevice(DeviceName deviceName, bool checked);
    void on_xtpWeb_initOutputDevice(DeviceName deviceName, bool checked);

private slots:
    void on_serialOutputRdo_clicked();

    void on_networkOutputRdo_clicked();
    void setUpTCodeAxis();

    void on_serialRefreshBtn_clicked();

    void onRange_valueChanged(QString name, int value);
    void onRange_mouseRelease(QString name);
    void onOffSet_valueChanged(int value);
    void onOffSetStep_valueChanged(int value);
    void onRangeModifierStep_valueChanged(int value);

    void on_SerialOutputCmb_currentIndexChanged(int index);

    void on_networkAddressTxt_editingFinished();

    void on_networkPortTxt_editingFinished();

    void on_serialConnectButton_clicked();

    void on_networkConnectButton_clicked();

    void on_deoConnectButton_clicked();

    void on_deoAddressTxt_editingFinished();

    void on_deoPortTxt_editingFinished();

    void on_deoCheckbox_clicked(bool checked);

    void on_checkBox_clicked(bool checked);

    void on_resetAllButton_clicked();

    void on_gamePadCheckbox_clicked(bool checked);

    void on_videoIncrement_valueChanged(int value);

    void on_speedInput_valueChanged(int value);
    void on_speedIncrementInput_valueChanged(int value);
    void xRangeStepInput_valueChanged(int value);

    void lubePulseEnabled_valueChanged(bool value);
    void lubeFrequency_valueChanged(int value);
    void lubeAmount_valueChanged(int value);

    void on_disableTextToSpeechCheckBox_clicked(bool checked);

    void on_invertFunscriptXCheckBox_clicked(bool checked);

    void on_whirligigCheckBox_clicked(bool checked);

    void on_whirligigConnectButton_clicked();

    void on_enableMultiplierCheckbox_clicked(bool checked);
    void on_dialogButtonboxClicked(QAbstractButton* button);

    void on_channelAddButton_clicked();

    void on_channelDeleteButton_clicked();

    void on_axisDefaultButton_clicked();

    void on_tCodeHome_clicked();

    void on_axis_valueChange(QString axis, int value);
    void on_axis_valueReset();

    void on_libraryExclusionsBtn_clicked();

    void on_passwordButton_clicked();

    void on_exportButton_clicked();

    void on_importButton_clicked();

    void on_thumbDirButton_clicked();

    void on_thumbsDirDefaultButton_clicked();

    void on_disableVRScriptNotFoundCheckbox_stateChanged(int arg1);

    void on_disableNoScriptFoundInLibrary_stateChanged(int arg1);

    void on_tCodeVSComboBox_currentIndexChanged(int index);

    void on_hideWelcomeDialog_clicked(bool checked);

    void on_launchWelcomeDialog_clicked();

    void on_disableTCodeValidationCheckbox_clicked(bool checked);

    void on_close_loading_dialog();

    void on_settingsChange(bool dirty);

    void on_showLoneFunscriptsInLibraryCheckbox_clicked(bool checked);

    void on_skipStandAloneFunscriptsInMainLibraryPlaylist_clicked(bool checked);

    void on_skipToMoneyShotPlaysFunscriptCheckbox_clicked(bool checked);

    void on_browseSkipToMoneyShotFunscriptButton_clicked(bool checked);

    void on_skipToMoneyShotSkipsVideo_clicked(bool checked);

    void on_skipToMoneyShotStandAloneLoopCheckBox_clicked(bool checked);

    void on_enableHttpServerCheckbox_clicked(bool checked);

    void on_browseHttpRootButton_clicked();

    void on_browseVRLibraryButton_clicked();

    void on_httpPort_valueChanged(int value);

    void on_chunkSizeDouble_valueChanged(double value);

    void on_httpRootLineEdit_textEdited(const QString &arg1);

    void on_vrLibraryLineEdit_textEdited(const QString &arg1);

    void on_finscriptModifierSpinBox_valueChanged(int arg1);

    void on_xtpWebHandlerCheckbox_clicked(bool checked);

    void on_useMediaDirectoryCheckbox_clicked(bool checked);

    void on_webSocketPort_valueChanged(int arg1);

    void on_httpPortSpinBox_editingFinished();

    void on_webSocketPortSpinBox_editingFinished();

    void on_httpThumbQualitySpinBox_editingFinished();

    void on_gamepad_connectionChanged(ConnectionChangedSignal event);
    void on_input_device_connectionChanged(ConnectionChangedSignal event);
    void on_output_device_connectionChanged(ConnectionChangedSignal event);

private:

    Ui::SettingsDialog ui;
    void loadSerialPorts();
    void initSerialEvent();
    void initNetworkEvent();
    void initDeoEvent();
    void initWhirligigEvent();
    void initXTPWebEvent();
    void initInputDevice();
    void setupUi();
    void setupGamepadMap();
    void setUpMultiplierUi(bool enabled);
    void enableOrDisableDeviceConnectionUI(DeviceName deviceName);
    QString encryptPass(QString pass);
    QString decryptPass(QString pass);

    QList<QWidget*> _multiplierWidgets;

    LibraryExclusions* _libraryExclusions;
    bool _interfaceInitialized = false;
    QList<SerialComboboxItem> serialPorts;
    SerialComboboxItem selectedSerialPort;
    VideoHandler* _videoHandler;
    SyncHandler* _syncHandler;
    ConnectionHandler* _connectionHandler;
    TCodeHandler* _tcodeHandler;
    InputMapWidget* _inputMapWidget;

    ChannelTableViewModel* channelTableViewModel;

    QMap<QString, QLabel*> rangeMinLabels;
    QMap<QString, QLabel*> rangeLabels;
    QMap<QString, QLabel*> rangeMaxLabels;
    QMap<QString, RangeSlider*> rangeSliders;
    QMap<QString, QProgressBar*> axisProgressbars;

    QLabel* offSetLabel;
    QSpinBox* offSetSpinBox;
    QPushButton* saveAllBtn;
    QPushButton* closeBtn;
    QPushButton* saveBtn;
    bool _hasVideoPlayed = false;
    bool _requiresRestart = false;

    void setDeviceStatusStyle(ConnectionStatus status, DeviceName deviceName, QString message = "");

signals:
    void loadingDialogClose();
};

#endif // SETTINGSDIALOG_H
