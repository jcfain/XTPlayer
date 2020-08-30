#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include<ui_settings.h>
#include <QDialog>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QSpinBox>
#include "CustomControls/rangeslider.h"
#include "lib/handler/settingshandler.h"
#include "lib/struct/SerialComboboxItem.h"
#include "lib/handler/serialhandler.h"
#include "lib/handler/videohandler.h"
#include "lib/handler/udphandler.h"

extern void initSerial(SerialHandler* serialHandler, SerialComboboxItem serialInfo);
extern void initNetwork(UdpHandler* serialHandler, NetworkAddress address);

class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog();

    void init(VideoHandler* videoHandler);
    UdpHandler* getNetworkHandler();
    SerialHandler* getSerialHandler();
    void setSelectedDeviceHandler(DeviceHandler* device);
    DeviceHandler* getSelectedDeviceHandler();
    bool isConnected();
    void initDeviceRetry();

signals:
    void deviceError(QString error);
    void deviceConnectionChange(ConnectionChangedSignal event);

private slots:
    void on_serialOutputRdo_clicked();

    void on_networkOutputRdo_clicked();

    void on_serialRefreshBtn_clicked();

    void onXRange_valueChanged(int value);
    void onYRollRange_valueChanged(int value);
    void onXRollRange_valueChanged(int value);
    void onTwistRange_valueChanged(int value);
    void onOffSet_valueChanged(int value);
    void on_device_connectionChanged(ConnectionChangedSignal event);
    void on_device_error(QString error);

    void on_SerialOutputCmb_currentIndexChanged(int index);

    void on_networkAddressTxt_editingFinished();

    void on_networkPortTxt_editingFinished();

    void on_buttonBox_clicked(QAbstractButton *button);

    void on_xRollMultiplierCheckBox_clicked();

    void on_xRollMultiplierSpinBox_valueChanged(double arg1);

    void on_yRollMultiplierCheckBox_clicked();

    void on_yRollMultiplierSpinBox_valueChanged(double arg1);

    void on_twistMultiplierCheckBox_clicked();

    void on_twistMultiplierSpinBox_valueChanged(double arg1);

    void on_vibMultiplierSpinBox_valueChanged(double arg1);

    void on_vibMultiplierCheckBox_clicked();

    void on_serialConnectButton_clicked();

    void on_networkConnectButton_clicked();

private:
    Ui::SettingsDialog ui;
    void loadSerialPorts();
    void initSerialEvent();
    void initNetworkEvent();
    void setupUi();

    bool _interfaceInitialized = false;
    ConnectionStatus _connectionStatus = ConnectionStatus::Disconnected;
    QList<SerialComboboxItem> serialPorts;
    SerialComboboxItem selectedSerialPort;
    DeviceHandler* selectedDeviceHandler;
    VideoHandler* _videoHandler;
    SerialHandler* _serialHandler;
    UdpHandler* _udpHandler;
    QFuture<void> _initFuture;
    QLabel* xRangeLabel;
    QLabel* xRangeMinLabel;
    QLabel* xRangeMaxLabel;
    QLabel* yRollRangeLabel;
    QLabel* yRollRangeMinLabel;
    QLabel* yRollRangeMaxLabel;
    QLabel* xRollRangeLabel;
    QLabel* xRollRangeMinLabel;
    QLabel* xRollRangeMaxLabel;
    QLabel* twistRangeLabel;
    QLabel* twistRangeMinLabel;
    QLabel* twistRangeMaxLabel;
    QLabel* offSetLabel;
    RangeSlider* xRangeSlider;
    RangeSlider* yRollRangeSlider;
    RangeSlider* xRollRangeSlider;
    RangeSlider* twistRangeSlider;
    QSpinBox* offSetSpinBox;

    void setDeviceStatusStyle(ConnectionStatus status, DeviceType deviceType, QString message = "");
};

#endif // SETTINGSDIALOG_H
