#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <ui_settings.h>
#include <QDialog>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QSpinBox>
#include <QProgressBar>
#include <QInputDialog>
#include <QDesktopServices>
#include <QClipboard>
#include <QNetworkInterface>
#include "lib/struct/SerialComboboxItem.h"
#include "lib/handler/crypthandler.h"
#include "lib/handler/connectionhandler.h"

#include "inputmapwidget.h"
#include "rangeslider.h"
#include "channeltableviewmodel.h"
#include "videohandler.h"
#include "libraryexclusions.h"


class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog();

    void init(VideoHandler* videoHandler, ConnectionHandler* connectionHandler);
    void initLive();
    void dispose();
    void setAxisProgressBar(QString axis, int value);
    void resetAxisProgressBars();
    void latestYoutubeDownloaded();

    void Export(QWidget* parent);
    void Import(QWidget* parent);
    void requestRestart(QWidget* parent);
    void askRestart(QWidget* parent, QString message = "Some changes made requires a restart.\nWould you like to restart now?");
    void quit(bool restart);
    void restart();

    void reject() override;

signals:
    void TCodeHomeClicked();
    void onAxisValueChange(QString axis, int value);
    void onAxisValueReset();
    void onOpenWelcomeDialog();
    void messageSend(QString message, XLogLevel loglevel);
    void updateLibrary();
    void cleanUpThumbsDirectory();
    void disableHeatmapToggled(bool disabled);

public slots:
    void on_xtpWeb_initInputDevice(DeviceName deviceName, bool checked);
    void on_xtpWeb_initOutputDevice(DeviceName deviceName, bool checked);
    void on_gamepad_connectionChanged(ConnectionChangedSignal event);
    void on_input_device_connectionChanged(ConnectionChangedSignal event);
    void on_output_device_connectionChanged(ConnectionChangedSignal event);
    void set_channelProfilesComboBox_value(const QString &arg1);
    void onCleanUpThumbsDirectoryComplete();
    void onCleanUpThumbsDirectoryStopped();

private slots:
    void on_serialOutputRdo_clicked();

    void on_networkOutputRdo_clicked();
    void setUpTCodeChannelProfiles();
    void setUpTCodeChannelUI();

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
    void on_openDeoPDFButton_clicked();

    void on_showVRInLibraryViewCheckbox_clicked(bool checked);

    void on_useWebSocketsCheckbox_clicked(bool checked);

    void on_webAddressCopyButton_clicked();
    void set_requires_restart(bool enabled);

    void on_rememberWindowSettingsChk_clicked(bool checked);

    void on_dubugButton_clicked();

    void on_disableTimeLinePreviewChk_clicked(bool checked);

    void on_MFSDiscoveryDisabledCheckBox_clicked(bool checked);

    void on_channelProfilesComboBox_textChange(const QString &arg1);

    void on_addChannelProfileButton_clicked();

    void on_deleteProfileButton_clicked();

    void on_defultSelectedProfile_clicked();

    void on_allProfilesDefaultButton_clicked();

    void on_hideMediaWithoutFunscriptsCheckbox_clicked(bool checked);

    void on_cleanupThumbsPushButton_clicked();

    void on_disableFunscriptHeatmapheckBox_clicked(bool checked);

    void on_webPasswordButton_clicked();

private:

    Ui::SettingsDialog ui;
    void save(QSettings* settingsToSaveTo = 0);
    void loadSerialPorts();
    void initInputDevice();
    void setupUi();
    void setupGamepadMap();
    void setUpMultiplierUi(bool enabled);
    void enableOrDisableDeviceConnectionUI(DeviceName deviceName);
    void updateIPAddress();
    void askHowToResetChannelProfileDefaults();

    QList<QWidget*> _multiplierWidgets;

    LibraryExclusions* _libraryExclusions;
    bool _interfaceInitialized = false;
    QList<SerialComboboxItem> serialPorts;
    SerialComboboxItem selectedSerialPort;
    VideoHandler* _videoHandler;
    ConnectionHandler* _connectionHandler;
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
