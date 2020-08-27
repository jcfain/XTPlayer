#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include<ui_settings.h>
#include <QDialog>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include "CustomControls/RangeSlider.h"
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
    void setupUi();
    UdpHandler* getNetworkHandler();
    SerialHandler* getSerialHandler();
    void setSelectedDeviceHandler(DeviceHandler* device);
    DeviceHandler* getSelectedDeviceHandler();
    bool isConnected();

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
    void onOffSet_valueChanged(int value);
    void on_device_connectionChanged(ConnectionChangedSignal event);
    void on_device_error(QString error);

    void on_SerialOutputCmb_currentIndexChanged(int index);

    void on_networkAddressTxt_editingFinished();

    void on_networkPortTxt_editingFinished();

    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::SettingsDialog ui;
    void loadSerialPorts();
    void initSerialEvent();
    void initNetworkEvent();

    bool _interfaceInitialized;
    bool _deviceConnected = false;
    QList<SerialComboboxItem> serialPorts;
    SerialComboboxItem selectedSerialPort;
    DeviceHandler* selectedDeviceHandler;
    VideoHandler* _videoHandler;
    SerialHandler* _serialHandler;
    UdpHandler* _udpHandler;
    QFuture<void> _initFuture;
    QLabel* xRangeLabel;
    QLabel* yRollRangeLabel;
    QLabel* xRollRangeLabel;
    QLabel* offSetLabel;
    RangeSlider* volumeSlider;
    RangeSlider* seekSlider;
    RangeSlider* xRangeSlider;
    RangeSlider* yRollRangeSlider;
    RangeSlider* xRollRangeSlider;
    RangeSlider* offSetSlider;

    void setDeviceStatusStyle(ConnectionStatus status, DeviceType deviceType, QString message = "");
};

#endif // SETTINGSDIALOG_H
