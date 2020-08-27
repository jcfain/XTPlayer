#include "settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
    _serialHandler = new SerialHandler(this);
    _udpHandler = new UdpHandler(this);
    if(SettingsHandler::getSelectedDevice() == DeviceType::Serial)
    {
        setSelectedDeviceHandler(_serialHandler);
    }
    else if (SettingsHandler::getSelectedDevice() == DeviceType::Network)
    {
        setSelectedDeviceHandler(_udpHandler);
    }

    connect(_serialHandler, &SerialHandler::connectionChange, this, &SettingsDialog::on_device_connectionChanged);
    connect(_serialHandler, &SerialHandler::errorOccurred, this, &SettingsDialog::on_device_error);
    connect(_udpHandler, &UdpHandler::connectionChange, this, &SettingsDialog::on_device_connectionChanged);
    connect(_udpHandler, &UdpHandler::errorOccurred, this, &SettingsDialog::on_device_error);
}
SettingsDialog::~SettingsDialog()
{
    _udpHandler->dispose();
    _serialHandler->dispose();
    if(_initFuture.isRunning())
    {
        _initFuture.cancel();
        _initFuture.waitForFinished();
    }
}

void SettingsDialog::init(VideoHandler* videoHandler)
{
    _videoHandler = videoHandler;
    if(SettingsHandler::getSelectedDevice() == DeviceType::Serial)
    {
        initSerialEvent();
    }
    else if (SettingsHandler::getSelectedDevice() == DeviceType::Network)
    {
        initNetworkEvent();
    }
}

void SettingsDialog::setupUi()
{

    loadSerialPorts();
    if (!_interfaceInitialized)
    {
        _interfaceInitialized = true;
        setDeviceStatusStyle(ConnectionStatus::Disconnected, DeviceType::Serial);
        setDeviceStatusStyle(ConnectionStatus::Disconnected, DeviceType::Network);
        ui.SerialOutputCmb->setCurrentText(SettingsHandler::getSerialPort());
        ui.networkAddressTxt->setText(SettingsHandler::getServerAddress());
        ui.networkPortTxt->setText(SettingsHandler::getServerPort());
        ui.xRollMultiplierCheckBox->setChecked(SettingsHandler::getXRollMultiplierChecked());
        ui.xRollMultiplierSpinBox->setValue(SettingsHandler::getXRollMultiplierValue());
        ui.yRollMultiplierCheckBox->setChecked(SettingsHandler::getYRollMultiplierChecked());
        ui.yRollMultiplierSpinBox->setValue(SettingsHandler::getYRollMultiplierValue());
        ui.twistMultiplierCheckBox->setChecked(SettingsHandler::getTwistMultiplierChecked());
        ui.twistMultiplierSpinBox->setValue(SettingsHandler::getTwistMultiplierValue());
        if(SettingsHandler::getSelectedDevice() == DeviceType::Serial)
        {
            ui.serialOutputRdo->setChecked(true);
        }
        else if (SettingsHandler::getSelectedDevice() == DeviceType::Network)
        {
            ui.networkOutputRdo->setChecked(true);
        }


        QFont font( "Sans Serif", 7);

        xRangeLabel = new QLabel("X Range");
        xRangeLabel->setFont(font);
        ui.RangeSettingsGrid->addWidget(xRangeLabel, 0,0);
        xRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
        xRangeSlider->setBackGroundEnabledColor(QColorConstants::Red);
        xRangeSlider->SetRange(1, 999);
        xRangeSlider->setLowerValue(SettingsHandler::getXMin());
        xRangeSlider->setUpperValue(SettingsHandler::getXMax());
        ui.RangeSettingsGrid->addWidget(xRangeSlider, 1,0);

        yRollRangeLabel = new QLabel("Y Roll Range");
        yRollRangeLabel->setFont(font);
        ui.RangeSettingsGrid->addWidget(yRollRangeLabel, 2,0);
        yRollRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
        yRollRangeSlider->setBackGroundEnabledColor(QColorConstants::Red);
        yRollRangeSlider->SetRange(1, 999);
        yRollRangeSlider->setLowerValue(SettingsHandler::getYRollMin());
        yRollRangeSlider->setUpperValue(SettingsHandler::getYRollMax());
        ui.RangeSettingsGrid->addWidget(yRollRangeSlider, 3,0);

        xRollRangeLabel = new QLabel("X Roll Range");
        xRollRangeLabel->setFont(font);
        ui.RangeSettingsGrid->addWidget(xRollRangeLabel, 4,0);
        xRollRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
        xRollRangeSlider->setBackGroundEnabledColor(QColorConstants::Red);
        xRollRangeSlider->SetRange(1, 999);
        xRollRangeSlider->setLowerValue(SettingsHandler::getXRollMin());
        xRollRangeSlider->setUpperValue(SettingsHandler::getXRollMax());
        ui.RangeSettingsGrid->addWidget(xRollRangeSlider, 5,0);

        offSetLabel = new QLabel("Offset: " + QString::number(SettingsHandler::getoffSet()));
        offSetLabel->setFont(font);
        ui.RangeSettingsGrid->addWidget(offSetLabel, 6,0);
        offSetSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::RightHandle, nullptr);
        offSetSlider->setBackGroundEnabledColor(QColorConstants::Red);
        offSetSlider->SetRange(1, 2000);
        offSetSlider->setUpperValue(SettingsHandler::getoffSetMap());
        ui.RangeSettingsGrid->addWidget(offSetSlider, 7,0);

        connect(xRangeSlider, &RangeSlider::lowerValueChanged, this, &SettingsDialog::onXRange_valueChanged);
        connect(xRangeSlider, &RangeSlider::upperValueChanged, this, &SettingsDialog::onXRange_valueChanged);
        connect(yRollRangeSlider, &RangeSlider::lowerValueChanged, this, &SettingsDialog::onYRollRange_valueChanged);
        connect(yRollRangeSlider, &RangeSlider::upperValueChanged, this, &SettingsDialog::onYRollRange_valueChanged);
        connect(xRollRangeSlider, &RangeSlider::lowerValueChanged, this, &SettingsDialog::onXRollRange_valueChanged);
        connect(xRollRangeSlider, &RangeSlider::upperValueChanged, this, &SettingsDialog::onXRollRange_valueChanged);
        connect(offSetSlider, &RangeSlider::lowerValueChanged, this, &SettingsDialog::onOffSet_valueChanged);

        if(SettingsHandler::getSelectedDevice() == DeviceType::Network)
        {
            on_networkOutputRdo_clicked();
        }
    }
}

UdpHandler* SettingsDialog::getNetworkHandler()
{
    return _udpHandler;
}

SerialHandler* SettingsDialog::getSerialHandler()
{
    return _serialHandler;
}
bool SettingsDialog::isConnected()
{
    return _deviceConnected;
}
void SettingsDialog::setSelectedDeviceHandler(DeviceHandler* device)
{
    selectedDeviceHandler = device;

}
DeviceHandler* SettingsDialog::getSelectedDeviceHandler()
{
    return selectedDeviceHandler;
}

void SettingsDialog::initSerialEvent()
{
    if (!_serialHandler->isRunning())
    {
        if (getSelectedDeviceHandler()->isRunning())
        {
            getSelectedDeviceHandler()->dispose();
        }
        if(_initFuture.isRunning())
        {
            _initFuture.cancel();
            _initFuture.waitForFinished();
        }
        SettingsHandler::setSelectedDevice(DeviceType::Serial);
        setSelectedDeviceHandler(_serialHandler);
        _initFuture = QtConcurrent::run(initSerial, _serialHandler, selectedSerialPort);
    }
}


void SettingsDialog::initNetworkEvent()
{
    if (!_udpHandler->isRunning())
    {
        if (getSelectedDeviceHandler()->isRunning())
        {
            getSelectedDeviceHandler()->dispose();
        }
        if(_initFuture.isRunning())
        {
            _initFuture.cancel();
            _initFuture.waitForFinished();
        }
        SettingsHandler::setSelectedDevice(DeviceType::Network);
        setSelectedDeviceHandler(_udpHandler);
        if(SettingsHandler::getServerAddress() != "" && SettingsHandler::getServerPort() != "")
        {
            NetworkAddress address { ui.networkAddressTxt->text(),  ui.networkPortTxt->text().toInt() };
            _initFuture = QtConcurrent::run(initNetwork, _udpHandler, address);
        }
    }
}


void initSerial(SerialHandler* serialHandler, SerialComboboxItem serialInfo)
{
    if(!serialHandler->isRunning())
        serialHandler->init(serialInfo.portName);
}

void initNetwork(UdpHandler* udpHandler, NetworkAddress address)
{
    if(!udpHandler->isRunning())
        udpHandler->init(address);
}

void SettingsDialog::loadSerialPorts()
{
    ui.SerialOutputCmb->clear();
    serialPorts.clear();
    SerialHandler serialHandler;
    serialPorts = serialHandler.getPorts();
    foreach(SerialComboboxItem item , serialPorts)
    {
        QVariant itemVarient;
        itemVarient.setValue(item);
        ui.SerialOutputCmb->addItem(item.friendlyName, itemVarient);
    }
}

void SettingsDialog::setDeviceStatusStyle(ConnectionStatus status, DeviceType deviceType, QString message)
{
    QString statusUnicode = "\u2717";
    QString statusColor = "red";
    QFont font( "Sans Serif", 12);
    if (status == ConnectionStatus::Connected)
    {
        statusUnicode = "\u2713";
        statusColor = "green";
    }
    else if (status == ConnectionStatus::Connecting)
    {
        statusUnicode = "\u231B";
        statusColor = "yellow";
    }
    if (deviceType == DeviceType::Serial)
    {
        ui.serialStatuslbl->setText(statusUnicode + " " + message);
        ui.serialStatuslbl->setFont(font);
        ui.serialStatuslbl->setStyleSheet("color: " + statusColor);
    }
    else if (deviceType == DeviceType::Network)
    {
        ui.networkStatuslbl->setText(statusUnicode + " " + message);
        ui.networkStatuslbl->setFont(font);
        ui.networkStatuslbl->setStyleSheet("color: " + statusColor);
    }
}

void SettingsDialog::on_serialOutputRdo_clicked()
{
    ui.SerialOutputCmb->setEnabled(true);;
    ui.networkAddressTxt->setEnabled(false);
    ui.networkPortTxt->setEnabled(false);
    initSerialEvent();
}

void SettingsDialog::on_networkOutputRdo_clicked()
{
    ui.SerialOutputCmb->setEnabled(false);
    ui.networkAddressTxt->setEnabled(true);
    ui.networkPortTxt->setEnabled(true);
    initNetworkEvent();
}

void SettingsDialog::on_serialRefreshBtn_clicked()
{
    loadSerialPorts();
}

void SettingsDialog::onXRange_valueChanged(int value)
{
    SettingsHandler::setXMin(xRangeSlider->GetLowerValue());
    SettingsHandler::setXMax(xRangeSlider->GetUpperValue());
    if (!_videoHandler->isPlaying() && getSelectedDeviceHandler()->isRunning())
    {
        getSelectedDeviceHandler()->sendTCode("L0" + QString::number(value) + "S1000");
    }
}

void SettingsDialog::onYRollRange_valueChanged(int value)
{
    SettingsHandler::setYRollMin(yRollRangeSlider->GetLowerValue());
    SettingsHandler::setYRollMax(yRollRangeSlider->GetUpperValue());
    if (!_videoHandler->isPlaying() && getSelectedDeviceHandler()->isRunning())
    {
        getSelectedDeviceHandler()->sendTCode("R1" + QString::number(value) + "S1000");
    }
}

void SettingsDialog::onXRollRange_valueChanged(int value)
{
    SettingsHandler::setXRollMin(xRollRangeSlider->GetLowerValue());
    SettingsHandler::setXRollMax(xRollRangeSlider->GetUpperValue());
    if (!_videoHandler->isPlaying() && getSelectedDeviceHandler()->isRunning())
    {
        getSelectedDeviceHandler()->sendTCode("R2" + QString::number(value) + "S1000");
    }
}

void SettingsDialog::onOffSet_valueChanged(int value)
{
    SettingsHandler::setoffSet(offSetSlider->GetUpperValue());
    offSetLabel->setText("Offset: " + QString::number(SettingsHandler::getoffSet()));
}


void SettingsDialog::on_device_connectionChanged(ConnectionChangedSignal event)
{
    _deviceConnected = event.status == ConnectionStatus::Connected;
    if(event.deviceType == DeviceType::Serial)
    {
        SettingsHandler::setSerialPort(ui.SerialOutputCmb->currentText());
        ui.serialStatuslbl->setText(event.message);
    }
    else if (event.deviceType == DeviceType::Network)
    {
    }
    if (event.status == ConnectionStatus::Error)
        setDeviceStatusStyle(event.status, event.deviceType, event.message);
    else
        setDeviceStatusStyle(event.status, event.deviceType);
    emit deviceConnectionChange({event.deviceType, event.status, event.message});
}

void SettingsDialog::on_device_error(QString error)
{
    emit deviceError(error);
}

void SettingsDialog::on_SerialOutputCmb_currentIndexChanged(int index)
{
    SerialComboboxItem serialInfo = ui.SerialOutputCmb->currentData(Qt::UserRole).value<SerialComboboxItem>();
    selectedSerialPort = serialInfo;
    if (SettingsHandler::getSelectedDevice() == DeviceType::Serial)
    {
        on_serialOutputRdo_clicked();
    }
}

void SettingsDialog::on_networkAddressTxt_editingFinished()
{
    SettingsHandler::setServerPort(ui.networkPortTxt->text());
    on_networkOutputRdo_clicked();
}

void SettingsDialog::on_networkPortTxt_editingFinished()
{
    SettingsHandler::setServerAddress(ui.networkAddressTxt->text());
    on_networkOutputRdo_clicked();
}

void SettingsDialog::on_xRollMultiplierCheckBox_clicked()
{
    SettingsHandler::setXRollMultiplierChecked(ui.xRollMultiplierCheckBox->isChecked());
}

void SettingsDialog::on_xRollMultiplierSpinBox_valueChanged(double value)
{
    SettingsHandler::setXRollMultiplierValue(value);
}

void SettingsDialog::on_yRollMultiplierCheckBox_clicked()
{
    SettingsHandler::setYRollMultiplierChecked(ui.yRollMultiplierCheckBox->isChecked());
}

void SettingsDialog::on_yRollMultiplierSpinBox_valueChanged(double value)
{
    SettingsHandler::setYRollMultiplierValue(value);
}

void SettingsDialog::on_twistMultiplierCheckBox_clicked()
{
    SettingsHandler::setTwistMultiplierChecked(ui.twistMultiplierCheckBox->isChecked());
}

void SettingsDialog::on_twistMultiplierSpinBox_valueChanged(double value)
{
    SettingsHandler::setTwistMultiplierValue(value);
}

void SettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    //if (button->Cl)
    hide();
}
