#include "settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
    _serialHandler = new SerialHandler(this);
    _udpHandler = new UdpHandler(this);
    _deoHandler = new DeoHandler(this);
    if(SettingsHandler::getSelectedDevice() == DeviceType::Serial)
    {
        setSelectedDeviceHandler(_serialHandler);
    }
    else if (SettingsHandler::getSelectedDevice() == DeviceType::Network)
    {
        setSelectedDeviceHandler(_udpHandler);
    }
    ui.deoAddressTxt->setHidden(true);
    ui.deoCheckbox->setHidden(true);
    ui.deoConnectButton->setHidden(true);
    ui.deoPortTxt->setHidden(true);
    ui.deoStatuslbl->setHidden(true);

    connect(_serialHandler, &SerialHandler::connectionChange, this, &SettingsDialog::on_device_connectionChanged);
    connect(_serialHandler, &SerialHandler::errorOccurred, this, &SettingsDialog::on_device_error);
    connect(_udpHandler, &UdpHandler::connectionChange, this, &SettingsDialog::on_device_connectionChanged);
    connect(_udpHandler, &UdpHandler::errorOccurred, this, &SettingsDialog::on_device_error);
    connect(_deoHandler, &DeoHandler::connectionChange, this, &SettingsDialog::on_deo_connectionChanged);
    connect(_deoHandler, &DeoHandler::errorOccurred, this, &SettingsDialog::on_deo_error);
}
SettingsDialog::~SettingsDialog()
{
}
void SettingsDialog::dispose()
{
    _udpHandler->dispose();
    _serialHandler->dispose();
    _deoHandler->dispose();
    if(_initFuture.isRunning())
    {
        _initFuture.cancel();
        _initFuture.waitForFinished();
    }
    delete _serialHandler;
    delete _udpHandler;
}
void SettingsDialog::init(VideoHandler* videoHandler)
{
    _videoHandler = videoHandler;
    setupUi();
    if(SettingsHandler::getSelectedDevice() == DeviceType::Serial)
    {
        initSerialEvent();
    }
    else if (SettingsHandler::getSelectedDevice() == DeviceType::Network)
    {
        initNetworkEvent();
    }
    if(SettingsHandler::getDeoEnabled())
    {
        initDeoEvent();
    }
}

void SettingsDialog::setupUi()
{

    loadSerialPorts();
    if (!_interfaceInitialized)
    {
        _interfaceInitialized = true;
        setDeviceStatusStyle(_connectionStatus, DeviceType::Serial);
        setDeviceStatusStyle(_connectionStatus, DeviceType::Network);
        ui.SerialOutputCmb->setCurrentText(SettingsHandler::getSerialPort());
        ui.networkAddressTxt->setText(SettingsHandler::getServerAddress());
        ui.networkPortTxt->setText(SettingsHandler::getServerPort());
        ui.deoAddressTxt->setText(SettingsHandler::getDeoAddress());
        ui.deoPortTxt->setText(SettingsHandler::getDeoPort());
        ui.xRollMultiplierCheckBox->setChecked(SettingsHandler::getXRollMultiplierChecked());
        ui.xRollMultiplierSpinBox->setValue(SettingsHandler::getXRollMultiplierValue());
        ui.yRollMultiplierCheckBox->setChecked(SettingsHandler::getYRollMultiplierChecked());
        ui.yRollMultiplierSpinBox->setValue(SettingsHandler::getYRollMultiplierValue());
        ui.twistMultiplierCheckBox->setChecked(SettingsHandler::getTwistMultiplierChecked());
        ui.twistMultiplierSpinBox->setValue(SettingsHandler::getTwistMultiplierValue());

        QFont font( "Sans Serif", 7);

        xRangeMinLabel = new QLabel(QString::number(SettingsHandler::getXMin()));
        xRangeMinLabel->setFont(font);
        xRangeMinLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        ui.RangeSettingsGrid->addWidget(xRangeMinLabel, 0,0);
        xRangeLabel = new QLabel("X Range");
        xRangeLabel->setFont(font);
        xRangeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui.RangeSettingsGrid->addWidget(xRangeLabel, 0,1);
        xRangeMaxLabel = new QLabel(QString::number(SettingsHandler::getXMax()));
        xRangeMaxLabel->setFont(font);
        xRangeMaxLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui.RangeSettingsGrid->addWidget(xRangeMaxLabel, 0,2);
        xRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
        xRangeSlider->SetRange(1, 999);
        xRangeSlider->setLowerValue(SettingsHandler::getXMin());
        xRangeSlider->setUpperValue(SettingsHandler::getXMax());
        ui.RangeSettingsGrid->addWidget(xRangeSlider, 1,0,1,3);

        yRollRangeMinLabel = new QLabel(QString::number(SettingsHandler::getYRollMin()));
        yRollRangeMinLabel->setFont(font);
        yRollRangeMinLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        ui.RangeSettingsGrid->addWidget(yRollRangeMinLabel, 2,0);
        yRollRangeLabel = new QLabel("Y Roll Range");
        yRollRangeLabel->setFont(font);
        yRollRangeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui.RangeSettingsGrid->addWidget(yRollRangeLabel, 2,1);
        yRollRangeMaxLabel = new QLabel(QString::number(SettingsHandler::getYRollMax()));
        yRollRangeMaxLabel->setFont(font);
        yRollRangeMaxLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui.RangeSettingsGrid->addWidget(yRollRangeMaxLabel, 2,2);
        yRollRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
        yRollRangeSlider->SetRange(1, 999);
        yRollRangeSlider->setLowerValue(SettingsHandler::getYRollMin());
        yRollRangeSlider->setUpperValue(SettingsHandler::getYRollMax());
        ui.RangeSettingsGrid->addWidget(yRollRangeSlider, 3,0,1,3);

        xRollRangeMinLabel = new QLabel(QString::number(SettingsHandler::getXRollMin()));
        xRollRangeMinLabel->setFont(font);
        xRollRangeMinLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        ui.RangeSettingsGrid->addWidget(xRollRangeMinLabel, 4,0);
        xRollRangeLabel = new QLabel("X Roll Range");
        xRollRangeLabel->setFont(font);
        xRollRangeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui.RangeSettingsGrid->addWidget(xRollRangeLabel, 4,1);
        xRollRangeMaxLabel = new QLabel(QString::number(SettingsHandler::getXRollMax()));
        xRollRangeMaxLabel->setFont(font);
        xRollRangeMaxLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui.RangeSettingsGrid->addWidget(xRollRangeMaxLabel, 4,2);
        xRollRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
        xRollRangeSlider->SetRange(1, 999);
        xRollRangeSlider->setLowerValue(SettingsHandler::getXRollMin());
        xRollRangeSlider->setUpperValue(SettingsHandler::getXRollMax());
        ui.RangeSettingsGrid->addWidget(xRollRangeSlider, 5,0,1,3);

        twistRangeMinLabel = new QLabel(QString::number(SettingsHandler::getTwistMin()));
        twistRangeMinLabel->setFont(font);
        twistRangeMinLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        ui.RangeSettingsGrid->addWidget(twistRangeMinLabel, 6,0);
        twistRangeLabel = new QLabel("Twist Range");
        twistRangeLabel->setFont(font);
        twistRangeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui.RangeSettingsGrid->addWidget(twistRangeLabel, 6,1);
        twistRangeMaxLabel = new QLabel(QString::number(SettingsHandler::getTwistMin()));
        twistRangeMaxLabel->setFont(font);
        twistRangeMaxLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui.RangeSettingsGrid->addWidget(twistRangeMaxLabel, 6,2);
        twistRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
        twistRangeSlider->SetRange(1, 999);
        twistRangeSlider->setLowerValue(SettingsHandler::getTwistMin());
        twistRangeSlider->setUpperValue(SettingsHandler::getTwistMax());
        ui.RangeSettingsGrid->addWidget(twistRangeSlider, 7,0,1,3);

        offSetLabel = new QLabel("Sync offset");
        offSetLabel->setFont(font);
        offSetLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        ui.RangeSettingsGrid->addWidget(offSetLabel, 8,1);
        offSetSpinBox = new QSpinBox(this);
        offSetSpinBox->setSuffix("ms");
        offSetSpinBox->setSingleStep(1);
        offSetSpinBox->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        offSetSpinBox->setMinimum(std::numeric_limits<int>::lowest());
        offSetSpinBox->setMaximum(std::numeric_limits<int>::max());
        offSetSpinBox->setValue(SettingsHandler::getoffSet());
        ui.RangeSettingsGrid->addWidget(offSetSpinBox, 9,1,1,1);

        ui.vibMultiplierSpinBox->setMinimum(std::numeric_limits<int>::lowest());
        ui.vibMultiplierSpinBox->setMaximum(std::numeric_limits<int>::max());
        ui.xRollMultiplierSpinBox->setMinimum(std::numeric_limits<int>::lowest());
        ui.xRollMultiplierSpinBox->setMaximum(std::numeric_limits<int>::max());
        ui.yRollMultiplierSpinBox->setMinimum(std::numeric_limits<int>::lowest());
        ui.yRollMultiplierSpinBox->setMaximum(std::numeric_limits<int>::max());
        ui.twistMultiplierSpinBox->setMinimum(std::numeric_limits<int>::lowest());
        ui.twistMultiplierSpinBox->setMaximum(std::numeric_limits<int>::max());

        connect(xRangeSlider, &RangeSlider::lowerValueChanged, this, &SettingsDialog::onXRange_valueChanged);
        connect(xRangeSlider, &RangeSlider::upperValueChanged, this, &SettingsDialog::onXRange_valueChanged);
        connect(yRollRangeSlider, &RangeSlider::lowerValueChanged, this, &SettingsDialog::onYRollRange_valueChanged);
        connect(yRollRangeSlider, &RangeSlider::upperValueChanged, this, &SettingsDialog::onYRollRange_valueChanged);
        connect(xRollRangeSlider, &RangeSlider::lowerValueChanged, this, &SettingsDialog::onXRollRange_valueChanged);
        connect(xRollRangeSlider, &RangeSlider::upperValueChanged, this, &SettingsDialog::onXRollRange_valueChanged);
        connect(twistRangeSlider, &RangeSlider::lowerValueChanged, this, &SettingsDialog::onTwistRange_valueChanged);
        connect(twistRangeSlider, &RangeSlider::upperValueChanged, this, &SettingsDialog::onTwistRange_valueChanged);
        connect(offSetSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onOffSet_valueChanged);

        if(SettingsHandler::getSelectedDevice() == DeviceType::Serial)
        {
            ui.serialOutputRdo->setChecked(true);
            on_serialOutputRdo_clicked();
        }
        else if (SettingsHandler::getSelectedDevice() == DeviceType::Network)
        {
            ui.networkOutputRdo->setChecked(true);
            on_networkOutputRdo_clicked();
        }
        ui.deoCheckbox->setChecked(SettingsHandler::getDeoEnabled());
        on_deoCheckbox_clicked(SettingsHandler::getDeoEnabled());
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

DeoHandler* SettingsDialog::getDeoHandler()
{
    return _deoHandler;
}

bool SettingsDialog::isConnected()
{
    return _connectionStatus == ConnectionStatus::Connected;
}

void SettingsDialog::setSelectedDeviceHandler(DeviceHandler* device)
{
    selectedDeviceHandler = device;

}

DeviceHandler* SettingsDialog::getSelectedDeviceHandler()
{
    return selectedDeviceHandler;
}

void SettingsDialog::initDeviceRetry()
{
    if(SettingsHandler::getSelectedDevice() == DeviceType::Serial)
    {
        initSerialEvent();
    }
    else if (SettingsHandler::getSelectedDevice() == DeviceType::Network)
    {
        initNetworkEvent();
    }
}

void SettingsDialog::initDeoRetry()
{
    initDeoEvent();
}

void SettingsDialog::initSerialEvent()
{
    if (!_serialHandler->isRunning())
    {
        ui.serialConnectButton->setEnabled(false);
        //ui.networkConnectButton->setEnabled(false);
        setDeviceStatusStyle(ConnectionStatus::Connecting, DeviceType::Serial);
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
        ui.networkConnectButton->setEnabled(false);
        //ui.serialConnectButton->setEnabled(false);
        setDeviceStatusStyle(ConnectionStatus::Connecting, DeviceType::Network);
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
        if(SettingsHandler::getServerAddress() != "" && SettingsHandler::getServerPort() != "" &&
            SettingsHandler::getServerAddress() != "0" && SettingsHandler::getServerPort() != "0")
        {
            NetworkAddress address { SettingsHandler::getServerAddress(), SettingsHandler::getServerPort().toInt() };
            _initFuture = QtConcurrent::run(initNetwork, _udpHandler, address);
        }
    }
}

void SettingsDialog::initDeoEvent()
{
    if (!_deoHandler->isConnected())
    {
        setDeviceStatusStyle(ConnectionStatus::Connecting, DeviceType::Deo);
        if(_initDeoFuture.isRunning())
        {
            _initDeoFuture.cancel();
            _initDeoFuture.waitForFinished();
        }
        if(SettingsHandler::getDeoAddress() != "" && SettingsHandler::getDeoPort() != "" &&
            SettingsHandler::getDeoAddress() != "0" && SettingsHandler::getDeoPort() != "0")
        {
            NetworkAddress address { SettingsHandler::getDeoAddress(), SettingsHandler::getDeoPort().toInt() };
            _deoHandler->init(address);
            //_initDeoFuture = QtConcurrent::run(initDeo, _deoHandler, address);
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

//void initDeo(DeoHandler* deoHandler, NetworkAddress address)
//{
//    if(!deoHandler->isConnected())
//        deoHandler->init(address);
//}

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
    else if (deviceType == DeviceType::Deo)
    {
        ui.deoStatuslbl->setText(statusUnicode + " " + message);
        ui.deoStatuslbl->setFont(font);
        ui.deoStatuslbl->setStyleSheet("color: " + statusColor);
    }
}

void SettingsDialog::on_serialOutputRdo_clicked()
{
    ui.SerialOutputCmb->setEnabled(true);;
    ui.networkAddressTxt->setEnabled(false);
    ui.networkPortTxt->setEnabled(false);
    ui.networkConnectButton->setEnabled(false);
    ui.serialConnectButton->setEnabled(true);
    ui.serialRefreshBtn->setEnabled(true);
}

void SettingsDialog::on_networkOutputRdo_clicked()
{
    ui.SerialOutputCmb->setEnabled(false);
    ui.networkAddressTxt->setEnabled(true);
    ui.networkPortTxt->setEnabled(true);
    ui.serialConnectButton->setEnabled(false);
    ui.networkConnectButton->setEnabled(true);
    ui.serialRefreshBtn->setEnabled(false);
}

void SettingsDialog::on_serialRefreshBtn_clicked()
{
    loadSerialPorts();
}

void SettingsDialog::onXRange_valueChanged(int value)
{
    SettingsHandler::setXMin(xRangeSlider->GetLowerValue());
    SettingsHandler::setXMax(xRangeSlider->GetUpperValue());
    xRangeMinLabel->setText(QString::number(xRangeSlider->GetLowerValue()));
    xRangeMaxLabel->setText(QString::number(xRangeSlider->GetUpperValue()));
    if (!_videoHandler->isPlaying() && getSelectedDeviceHandler()->isRunning())
    {
        getSelectedDeviceHandler()->sendTCode("L0" + QString::number(value) + "S1000");
    }
}

void SettingsDialog::onYRollRange_valueChanged(int value)
{
    SettingsHandler::setYRollMin(yRollRangeSlider->GetLowerValue());
    SettingsHandler::setYRollMax(yRollRangeSlider->GetUpperValue());
    yRollRangeMinLabel->setText(QString::number(yRollRangeSlider->GetLowerValue()));
    yRollRangeMaxLabel->setText(QString::number(yRollRangeSlider->GetUpperValue()));
    if (!_videoHandler->isPlaying() && getSelectedDeviceHandler()->isRunning())
    {
        getSelectedDeviceHandler()->sendTCode("R1" + QString::number(value) + "S1000");
    }
}

void SettingsDialog::onXRollRange_valueChanged(int value)
{
    SettingsHandler::setXRollMin(xRollRangeSlider->GetLowerValue());
    SettingsHandler::setXRollMax(xRollRangeSlider->GetUpperValue());
    xRollRangeMinLabel->setText(QString::number(xRollRangeSlider->GetLowerValue()));
    xRollRangeMaxLabel->setText(QString::number(xRollRangeSlider->GetUpperValue()));
    if (!_videoHandler->isPlaying() && getSelectedDeviceHandler()->isRunning())
    {
        getSelectedDeviceHandler()->sendTCode("R2" + QString::number(value) + "S1000");
    }
}

void SettingsDialog::onTwistRange_valueChanged(int value)
{
    SettingsHandler::setTwistMin(xRollRangeSlider->GetLowerValue());
    SettingsHandler::setTwistMax(xRollRangeSlider->GetUpperValue());
    twistRangeMinLabel->setText(QString::number(twistRangeSlider->GetLowerValue()));
    twistRangeMaxLabel->setText(QString::number(twistRangeSlider->GetUpperValue()));
    if (!_videoHandler->isPlaying() && getSelectedDeviceHandler()->isRunning())
    {
        getSelectedDeviceHandler()->sendTCode("R0" + QString::number(value) + "S1000");
    }
}

void SettingsDialog::onOffSet_valueChanged(int value)
{
    SettingsHandler::setoffSet(value);
}

void SettingsDialog::on_deo_connectionChanged(ConnectionChangedSignal event)
{
    if (event.status == ConnectionStatus::Error)
    {
        setDeviceStatusStyle(event.status, event.deviceType, event.message);
    }
    else
    {
        setDeviceStatusStyle(event.status, event.deviceType);
    }
    emit deoDeviceConnectionChange({event.deviceType, event.status, event.message});
}

void SettingsDialog::on_deo_error(QString error)
{
    emit deoDeviceError(error);
}
void SettingsDialog::on_device_connectionChanged(ConnectionChangedSignal event)
{
    _connectionStatus = event.status;
    if(event.deviceType == DeviceType::Serial)
    {
        SettingsHandler::setSerialPort(ui.SerialOutputCmb->currentText());
    }
    else if (event.deviceType == DeviceType::Network)
    {
    }
    if (event.status == ConnectionStatus::Error)
    {
        if (event.deviceType == DeviceType::Serial)
        {
            ui.serialConnectButton->setEnabled(true);
        }
        else if (event.deviceType == DeviceType::Network)
        {
            ui.networkConnectButton->setEnabled(true);
        }
        setDeviceStatusStyle(event.status, event.deviceType, event.message);
        getSelectedDeviceHandler()->dispose();
    }
    else if(event.status == ConnectionStatus::Disconnected)
    {
        if (event.deviceType == DeviceType::Serial)
        {
            ui.serialConnectButton->setEnabled(true);
        }
        else if (event.deviceType == DeviceType::Network)
        {
            ui.networkConnectButton->setEnabled(true);
        }
        setDeviceStatusStyle(event.status, event.deviceType);
    }
    else if(event.status == ConnectionStatus::Connecting)
    {
        if (event.deviceType == DeviceType::Serial)
        {
            ui.serialConnectButton->setEnabled(false);
        }
        else if (event.deviceType == DeviceType::Network)
        {
            ui.networkConnectButton->setEnabled(false);
        }
        setDeviceStatusStyle(event.status, event.deviceType);
    }
    else if(event.status == ConnectionStatus::Connected)
    {
        if (event.deviceType == DeviceType::Serial)
        {
            ui.serialConnectButton->setEnabled(false);
        }
        else if (event.deviceType == DeviceType::Network)
        {
            ui.networkConnectButton->setEnabled(false);
        }
        setDeviceStatusStyle(event.status, event.deviceType);
    }
    emit deviceConnectionChange({event.deviceType, event.status, event.message});
}

void SettingsDialog::on_device_error(QString error)
{
    if (SettingsHandler::getSelectedDevice() == DeviceType::Serial)
    {
        ui.serialConnectButton->setEnabled(true);
    }
    else if (SettingsHandler::getSelectedDevice() == DeviceType::Network)
    {
        ui.networkConnectButton->setEnabled(true);
    }
    emit deviceError(error);
}

void SettingsDialog::on_SerialOutputCmb_currentIndexChanged(int index)
{
    SerialComboboxItem serialInfo = ui.SerialOutputCmb->currentData(Qt::UserRole).value<SerialComboboxItem>();
    selectedSerialPort = serialInfo;
}

void SettingsDialog::on_networkAddressTxt_editingFinished()
{
    SettingsHandler::setServerAddress(ui.networkAddressTxt->text());
}

void SettingsDialog::on_networkPortTxt_editingFinished()
{
    SettingsHandler::setServerPort(ui.networkPortTxt->text());
}

void SettingsDialog::on_deoAddressTxt_editingFinished()
{
    SettingsHandler::setDeoAddress(ui.deoAddressTxt->text());
}

void SettingsDialog::on_deoPortTxt_editingFinished()
{
    SettingsHandler::setDeoPort(ui.deoPortTxt->text());
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

void SettingsDialog::on_vibMultiplierSpinBox_valueChanged(double value)
{
    SettingsHandler::setVibMultiplierValue(value);
}

void SettingsDialog::on_vibMultiplierCheckBox_clicked()
{
    SettingsHandler::setVibMultiplierChecked(ui.vibMultiplierCheckBox->isChecked());
}

void SettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    hide();
}

void SettingsDialog::on_serialConnectButton_clicked()
{
    initSerialEvent();
}

void SettingsDialog::on_networkConnectButton_clicked()
{
    if(SettingsHandler::getServerAddress() != "" && SettingsHandler::getServerPort() != "" &&
     SettingsHandler::getServerAddress() != "0" && SettingsHandler::getServerPort() != "0")
    {
        initNetworkEvent();
    }
    else
    {
        LogHandler::Dialog("Invalid network address!", XLogLevel::Critical);
    }
}

void SettingsDialog::on_deoConnectButton_clicked()
{
    if(SettingsHandler::getDeoAddress() != "" && SettingsHandler::getDeoPort() != "" &&
     SettingsHandler::getDeoAddress() != "0" && SettingsHandler::getDeoPort() != "0")
    {
        initDeoEvent();
    }
    else
    {
        LogHandler::Dialog("Invalid deo vr address!", XLogLevel::Critical);
    }
}

void SettingsDialog::on_deoCheckbox_clicked(bool checked)
{
    SettingsHandler::setDeoEnabled(checked);
    ui.deoAddressTxt->setEnabled(checked);
    ui.deoPortTxt->setEnabled(checked);
    ui.deoConnectButton->setEnabled(checked);
    if (!checked)
        _deoHandler->dispose();
}

void SettingsDialog::on_checkBox_clicked(bool checked)
{
    LogHandler::UserDebug(checked);
}
