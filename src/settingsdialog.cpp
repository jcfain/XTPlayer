#include "settingsdialog.h"
//http://192.168.0.145/toggleContinousTwist
SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
    _serialHandler = new SerialHandler(this);
    _udpHandler = new UdpHandler(this);
    _deoHandler = new DeoHandler(this);
    _whirligigHandler = new WhirligigHandler(this);
    _gamepadHandler = new GamepadHandler(this);
    if(SettingsHandler::getSelectedDevice() == DeviceType::Serial)
    {
        setSelectedDeviceHandler(_serialHandler);
    }
    else if (SettingsHandler::getSelectedDevice() == DeviceType::Network)
    {
        setSelectedDeviceHandler(_udpHandler);
    }
    setModal(false);
    connect(_serialHandler, &SerialHandler::connectionChange, this, &SettingsDialog::on_device_connectionChanged);
    connect(_serialHandler, &SerialHandler::errorOccurred, this, &SettingsDialog::on_device_error);
    connect(_udpHandler, &UdpHandler::connectionChange, this, &SettingsDialog::on_device_connectionChanged);
    connect(_udpHandler, &UdpHandler::errorOccurred, this, &SettingsDialog::on_device_error);
    connect(_deoHandler, &DeoHandler::connectionChange, this, &SettingsDialog::on_deo_connectionChanged);
    connect(_deoHandler, &DeoHandler::errorOccurred, this, &SettingsDialog::on_deo_error);
    connect(_whirligigHandler, &WhirligigHandler::connectionChange, this, &SettingsDialog::on_whirligig_connectionChanged);
    connect(_whirligigHandler, &WhirligigHandler::errorOccurred, this, &SettingsDialog::on_whirligig_error);
    connect(_gamepadHandler, &GamepadHandler::connectionChange, this, &SettingsDialog::on_gamepad_connectionChanged);
    connect(ui.buttonBox, & QDialogButtonBox::clicked, this, &SettingsDialog::on_dialogButtonboxClicked);
}
SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::dispose()
{
    _udpHandler->dispose();
    _serialHandler->dispose();
    _deoHandler->dispose();
    _whirligigHandler->dispose();
    _gamepadHandler->dispose();
    if(_initFuture.isRunning())
    {
        //_initFuture.cancel();
        _initFuture.waitForFinished();
    }
    LogHandler::ExportDebug();
    delete _serialHandler;
    delete _udpHandler;
    delete _deoHandler;
    delete _whirligigHandler;
    delete _gamepadHandler;
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
        setDeviceStatusStyle(ConnectionStatus::Disconnected, DeviceType::Deo);
        initDeoEvent();
    }
    else if(SettingsHandler::getWhirligigEnabled())
    {
        setDeviceStatusStyle(ConnectionStatus::Disconnected, DeviceType::Whirligig);
        initWhirligigEvent();
    }
    if(SettingsHandler::getGamepadEnabled())
    {
        setDeviceStatusStyle(ConnectionStatus::Disconnected, DeviceType::Gamepad);
        _gamepadHandler->init();
    }
}

void SettingsDialog::initLive()
{
    ui.enableMultiplierCheckbox->setChecked(SettingsHandler::getMultiplierEnabled());
    if(HasLaunchPass())
        ui.passwordButton->setText("Change password");

//    auto availableAxis = SettingsHandler::getAvailableAxis();
//    foreach(auto channel, availableAxis->keys())
//    {
//        ChannelModel axis = SettingsHandler::getAxis(channel);
//        QCheckBox* invertedCheckbox = ui.FunscriptSettingsGrid->findChild<QCheckBox*>(axis.FriendlyName);
//        if(invertedCheckbox != nullptr)
//            invertedCheckbox->setChecked(SettingsHandler::getChannelInverseChecked(channel));
//    }
}

void SettingsDialog::setupUi()
{

    loadSerialPorts();
    setDeviceStatusStyle(_outConnectionStatus, DeviceType::Serial);
    setDeviceStatusStyle(_outConnectionStatus, DeviceType::Network);
    setDeviceStatusStyle(_deoConnectionStatus, DeviceType::Deo);
    setDeviceStatusStyle(_whirligigConnectionStatus, DeviceType::Whirligig);
    setDeviceStatusStyle(_gamepadConnectionStatus, DeviceType::Gamepad);
    if(SettingsHandler::getSelectedDevice() == DeviceType::Serial)
    {
        ui.serialOutputRdo->setChecked(true);
    }
    else if (SettingsHandler::getSelectedDevice() == DeviceType::Network)
    {
        ui.networkOutputRdo->setChecked(true);
    }
    if (!_interfaceInitialized)
    {
        _interfaceInitialized = true;

        channelTableViewModel = new ChannelTableViewModel(this);
        ui.channelTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui.channelTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        ChannelTableComboboxDelegate* channelTypeCombobox = new ChannelTableComboboxDelegate(ui.channelTableView);
        QMap<QString, int> axisTypes;
        foreach(auto key, AxisTypes.keys())
            axisTypes.insert(key, (int)AxisTypes.value(key));
        channelTypeCombobox->setData(axisTypes);
        ChannelTableComboboxDelegate* channelDimensionCombobox = new ChannelTableComboboxDelegate(ui.channelTableView);
        QMap<QString, int> axisDimensions;
        foreach(auto key, AxisDimensions.keys())
            axisDimensions.insert(key, (int)AxisDimensions.value(key));
        channelDimensionCombobox->setData(axisDimensions);
        ui.channelTableView->setItemDelegateForColumn(7, channelTypeCombobox);
        ui.channelTableView->setItemDelegateForColumn(6, channelDimensionCombobox);
        ui.channelTableView->setModel(channelTableViewModel);

        ui.SerialOutputCmb->setCurrentText(SettingsHandler::getSerialPort());
        ui.networkAddressTxt->setText(SettingsHandler::getServerAddress());
        ui.networkPortTxt->setText(SettingsHandler::getServerPort());
        ui.deoAddressTxt->setText(SettingsHandler::getDeoAddress());
        ui.deoPortTxt->setText(SettingsHandler::getDeoPort());

        ui.RangeSettingsGrid->setSpacing(5);
        QFont font( "Sans Serif", 8);
        int sliderGridRow = 0;
        int multiplierGridRow = 1;
        int funscriptSettingsGridRow = 0;
        auto availableAxis = SettingsHandler::getAvailableAxis();
        foreach(auto channel, availableAxis->keys())
        {
            ChannelModel axis = SettingsHandler::getAxis(channel);
            if(axis.Type == AxisType::None || axis.Type == AxisType::HalfRange)
                continue;

            int userMin = axis.UserMin;
            int userMax = axis.UserMax;
            QLabel* rangeMinLabel = new QLabel(QString::number(userMin));
            rangeMinLabel->setFont(font);
            rangeMinLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            ui.RangeSettingsGrid->addWidget(rangeMinLabel, sliderGridRow, 0);
            rangeMinLabels.insert(channel, rangeMinLabel);

            QLabel* rangeLabel = new QLabel(axis.FriendlyName + " Range");
            rangeLabel->setFont(font);
            rangeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ui.RangeSettingsGrid->addWidget(rangeLabel, sliderGridRow, 1);

            QLabel* rangeMaxLabel = new QLabel(QString::number(userMax));
            rangeMaxLabel->setFont(font);
            rangeMaxLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui.RangeSettingsGrid->addWidget(rangeMaxLabel, sliderGridRow, 2);
            rangeMaxLabels.insert(channel, rangeMaxLabel);

            RangeSlider* axisRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
            axisRangeSlider->SetRange(axis.Min, axis.Max);
            axisRangeSlider->setLowerValue(userMin);
            axisRangeSlider->setUpperValue(userMax);
            axisRangeSlider->setName(channel);
            axisRangeSlider->SetMinimumRange(1);
            sliderGridRow++;
            ui.RangeSettingsGrid->addWidget(axisRangeSlider, sliderGridRow,0,1,3);
            rangeSliders.insert(channel, axisRangeSlider);
            sliderGridRow++;

            QProgressBar* funscriptProgressbar = new QProgressBar(this);
            funscriptProgressbar->setMinimum(0);
            funscriptProgressbar->setMaximum(100);
            funscriptProgressbar->setMaximumHeight(5);
            ui.RangeSettingsGrid->addWidget(funscriptProgressbar, sliderGridRow,0,1,3);
            axisProgressbars.insert(channel, funscriptProgressbar);
            sliderGridRow++;

            connect(this, &SettingsDialog::onAxisValueChange, this, &SettingsDialog::on_axis_valueChange);
            connect(this, &SettingsDialog::onAxisValueReset, this, &SettingsDialog::on_axis_valueReset);
            connect(axisRangeSlider, QOverload<QString, int>::of(&RangeSlider::lowerValueChanged), this, &SettingsDialog::onRange_valueChanged);
            connect(axisRangeSlider, QOverload<QString, int>::of(&RangeSlider::upperValueChanged), this, &SettingsDialog::onRange_valueChanged);
            // mouse release work around for gamepad recalculation reseting on every valueChange event.
            connect(axisRangeSlider, QOverload<QString>::of(&RangeSlider::mouseRelease), this, &SettingsDialog::onRange_mouseRelease);

            // Multipliers
            if(axis.Dimension != AxisDimension::Heave)
            {
                QCheckBox* multiplierCheckbox = new QCheckBox(this);
                multiplierCheckbox->setText(axis.FriendlyName);
                multiplierCheckbox->setChecked(SettingsHandler::getMultiplierChecked(channel));
                QDoubleSpinBox* multiplierInput = new QDoubleSpinBox(this);
                multiplierInput->setDecimals(3);
                multiplierInput->setSingleStep(0.1f);
                multiplierInput->setMinimum(std::numeric_limits<int>::lowest());
                multiplierInput->setMaximum(std::numeric_limits<int>::max());
                multiplierInput->setValue(SettingsHandler::getMultiplierValue(channel));
                connect(multiplierInput, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                        [channel](float value)
                          {
                            SettingsHandler::setMultiplierValue(channel, value);
                          });
                connect(multiplierCheckbox, &QCheckBox::clicked, this,
                        [channel](bool checked)
                          {
                            SettingsHandler::setMultiplierChecked(channel, checked);
                          });
                QCheckBox* damperCheckbox = new QCheckBox(this);
                damperCheckbox->setText("Speed");
                damperCheckbox->setChecked(SettingsHandler::getDamperChecked(channel));
                QDoubleSpinBox* damperInput = new QDoubleSpinBox(this);
                damperInput->setToolTip("If the funscript travel distance is greater than half(50) and the speed is greater than 1000,\nmultiply the speed by the inputed value.\n4000 * 0.05 = 2000");
                damperInput->setDecimals(1);
                damperInput->setSingleStep(0.1f);
                damperInput->setMinimum(0.1f);
                damperInput->setMaximum(std::numeric_limits<int>::max());
                damperInput->setValue(SettingsHandler::getDamperValue(channel));
                connect(damperInput, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                        [channel](float value)
                          {
                            SettingsHandler::setDamperValue(channel, value);
                          });
                connect(damperCheckbox, &QCheckBox::clicked, this,
                        [channel](bool checked)
                          {
                            SettingsHandler::setDamperChecked(channel, checked);
                          });


                ui.MultiplierSettingsGrid->addWidget(multiplierCheckbox, multiplierGridRow, 0, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
                ui.MultiplierSettingsGrid->addWidget(multiplierInput, multiplierGridRow, 1, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
                QCheckBox* linkCheckbox = new QCheckBox(this);
                auto relatedChannel = ChannelNames.value(axis.RelatedChannel);
                linkCheckbox->setToolTip("This will link the channel to the related axis.\nThis will remove the random calculation and just link\nthe current MFS " + relatedChannel + " funscript value.\nIf there is no " + relatedChannel + " funscript then it will default to random motion.");
                linkCheckbox->setText("Link to MFS " + relatedChannel);
                linkCheckbox->setChecked(SettingsHandler::getLinkToRelatedAxisChecked(channel));
                connect(linkCheckbox, &QCheckBox::clicked, this,
                        [channel](bool checked)
                          {
                            SettingsHandler::setLinkToRelatedAxisChecked(channel, checked);
                          });
                ui.MultiplierSettingsGrid->addWidget(linkCheckbox, multiplierGridRow, 2, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
                ui.MultiplierSettingsGrid->addWidget(damperCheckbox, multiplierGridRow, 3, 1, 1, Qt::AlignRight | Qt::AlignVCenter);
                ui.MultiplierSettingsGrid->addWidget(damperInput, multiplierGridRow, 4, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
                multiplierGridRow++;
            }
            QCheckBox* invertedCheckbox = new QCheckBox(this);
            invertedCheckbox->setText(axis.FriendlyName);
            invertedCheckbox->setChecked(SettingsHandler::getChannelInverseChecked(channel));
            connect(invertedCheckbox, &QCheckBox::clicked, this,
                    [channel](bool checked)
                      {
                        SettingsHandler::setChannelInverseChecked(channel, checked);
                      });
            ui.FunscriptSettingsGrid->addWidget(invertedCheckbox, funscriptSettingsGridRow, 0, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

            funscriptSettingsGridRow++;
        }

        offSetLabel = new QLabel("Sync offset");
        offSetLabel->setFont(font);
        offSetLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        offSetSpinBox = new QSpinBox(this);
        offSetSpinBox->setSuffix("ms");
        offSetSpinBox->setSingleStep(1);
        offSetSpinBox->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        offSetSpinBox->setMinimum(std::numeric_limits<int>::lowest());
        offSetSpinBox->setMaximum(std::numeric_limits<int>::max());
        offSetSpinBox->setValue(SettingsHandler::getoffSet());
        ui.RangeSettingsGrid->addWidget(offSetLabel, sliderGridRow, 0);
        ui.RangeSettingsGrid->addWidget(offSetSpinBox, sliderGridRow + 1,0);

        QPushButton* zeroOutButton = new QPushButton(this);
        zeroOutButton->setText("All axis home");
        connect(zeroOutButton, & QPushButton::clicked, this, &SettingsDialog::on_tCodeHome_clicked);
        ui.RangeSettingsGrid->addWidget(zeroOutButton, sliderGridRow + 1, 1);

        QLabel* xRangeStepLabel = new QLabel(this);
        xRangeStepLabel->setText("Stroke range change step");
        xRangeStepLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        QSpinBox* xRangeStepInput = new QSpinBox(this);
        xRangeStepInput->setMinimum(1);
        xRangeStepInput->setMaximum(100);
        xRangeStepInput->setMinimumWidth(75);
        xRangeStepInput->setSingleStep(50);
        xRangeStepInput->setValue(SettingsHandler::getGamepadSpeedIncrement());
        xRangeStepInput->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        connect(xRangeStepInput, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::xRangeStepInput_valueChanged);
        ui.RangeSettingsGrid->addWidget(xRangeStepLabel, sliderGridRow, 2);
        ui.RangeSettingsGrid->addWidget(xRangeStepInput, sliderGridRow + 1, 2);

        connect(offSetSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onOffSet_valueChanged);

        if(SettingsHandler::getSelectedDevice() == DeviceType::Serial)
        {
            on_serialOutputRdo_clicked();
        }
        else if (SettingsHandler::getSelectedDevice() == DeviceType::Network)
        {
            on_networkOutputRdo_clicked();
        }
        bool deoEnabled = SettingsHandler::getDeoEnabled();
        ui.deoCheckbox->setChecked(deoEnabled);
        ui.deoAddressTxt->setEnabled(deoEnabled);
        ui.deoPortTxt->setEnabled(deoEnabled);
        ui.deoConnectButton->setEnabled(deoEnabled);
        ui.whirligigCheckBox->setChecked(SettingsHandler::getWhirligigEnabled());
        ui.gamePadCheckbox->setChecked(SettingsHandler::getGamepadEnabled());
        ui.gamePadMapGroupbox->setHidden(!SettingsHandler::getGamepadEnabled());
        ui.videoIncrementSpinBox->setValue(SettingsHandler::getVideoIncrement());
        ui.disableTextToSpeechCheckBox->setChecked(SettingsHandler::getDisableSpeechToText());
        //Load user decoder priority. (Too lazy to make a new function sue me...)
        on_cancelPriorityButton_clicked();
        setupGamepadMap();
    }
}
void SettingsDialog::setupGamepadMap()
{
    auto gamepadMap = SettingsHandler::getGamePadMap();
    auto availableAxis = SettingsHandler::getAvailableAxis();
    MediaActions actions;
    int rowIterator = 0;
    int columnIterator = 0;
    int maxRows = 4;//6 total
    foreach(auto button, gamepadMap->keys())
    {
        if (button == "None")
            continue;
        QLabel* mapLabel = new QLabel(this);
        mapLabel->setText(button);
        QComboBox* mapComboBox = new QComboBox(this);
        foreach(auto axis, availableAxis->keys())
        {
            auto channel = availableAxis->value(axis);
            QVariant variant;
            variant.setValue(channel);
            mapComboBox->addItem(channel.FriendlyName, variant);
        }
        foreach(auto action, actions.Values.keys())
        {
            ChannelModel channel;
            channel.AxisName = action;
            channel.FriendlyName = actions.Values.value(action);
            QVariant variant;
            variant.setValue(channel);
            mapComboBox->addItem(channel.FriendlyName, variant);
        }
        auto gameMap = gamepadMap->value(button);

        if (availableAxis->contains(gameMap))
            mapComboBox->setCurrentText(availableAxis->value(gameMap).FriendlyName);
        else
            mapComboBox->setCurrentText(actions.Values.value(gameMap));

        ui.gamePadMapGridLayout->addWidget(mapLabel, rowIterator, columnIterator, Qt::AlignRight);
        ui.gamePadMapGridLayout->addWidget(mapComboBox, rowIterator, columnIterator + 1, Qt::AlignLeft);

        connect(mapComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [mapComboBox, gamepadMap, button](int index)
                  {
                        ChannelModel selectedChannel = mapComboBox->currentData().value<ChannelModel>();
                        SettingsHandler::setGamePadMapButton(button, selectedChannel.AxisName);
                  });
        if (rowIterator <= maxRows)
            rowIterator++;
        else
        {
            rowIterator = 0;
            columnIterator += 2;
        }
    }
    QGridLayout* inverseGrid = new QGridLayout(this);
    QFrame* inverseFrame = new QFrame(this);
    inverseFrame->setLayout(inverseGrid);
    ui.gamePadMapGridLayout->addWidget(inverseFrame, maxRows + 2, 0, 1, columnIterator + 2);

    QLabel* speedLabel = new QLabel(this);
    speedLabel->setText("Speed");
    speedLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    QSpinBox* speedInput = new QSpinBox(this);
    speedInput->setMinimum(1);
    speedInput->setMaximum(std::numeric_limits<int>::max());
    speedInput->setMinimumWidth(75);
    speedInput->setSuffix("ms");
    speedInput->setSingleStep(100);
    speedInput->setValue(SettingsHandler::getGamepadSpeed());
    speedInput->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    connect(speedInput, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::on_speedInput_valueChanged);
    inverseGrid->addWidget(speedLabel, 0, 0, Qt::AlignCenter);
    inverseGrid->addWidget(speedInput, 1, 0, Qt::AlignCenter);

    QLabel* speedIncrementLabel = new QLabel(this);
    speedIncrementLabel->setText("Speed change step");
    speedIncrementLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    QSpinBox* speedIncrmentInput = new QSpinBox(this);
    speedIncrmentInput->setMinimum(1);
    speedIncrmentInput->setMaximum(std::numeric_limits<int>::max());
    speedIncrmentInput->setMinimumWidth(75);
    speedIncrmentInput->setSingleStep(100);
    speedIncrmentInput->setValue(SettingsHandler::getGamepadSpeedIncrement());
    speedIncrmentInput->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    connect(speedIncrmentInput, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::on_speedIncrementInput_valueChanged);
    inverseGrid->addWidget(speedIncrementLabel, 0, 2, Qt::AlignCenter);
    inverseGrid->addWidget(speedIncrmentInput, 1, 2, Qt::AlignCenter);

    QCheckBox* inverseX = new QCheckBox(this);
    inverseX->setText("Inverse Stroke");
    inverseX->setChecked(SettingsHandler::getInverseTcXL0());
    connect(inverseX, &QCheckBox::toggled, this, &SettingsDialog::on_inverseTcXL0_valueChanged);
    inverseGrid->addWidget(inverseX, 4, 0, Qt::AlignCenter);
    QCheckBox* inverseYRoll = new QCheckBox(this);
    inverseYRoll->setText("Inverse Roll");
    inverseYRoll->setChecked(SettingsHandler::getInverseTcYRollR1());
    connect(inverseYRoll, &QCheckBox::toggled, this, &SettingsDialog::on_inverseTcYRollR1_valueChanged);
    inverseGrid->addWidget(inverseYRoll, 4, 1, Qt::AlignCenter);
    QCheckBox* inverseXRoll = new QCheckBox(this);
    inverseXRoll->setText("Inverse Pitch");
    inverseXRoll->setChecked(SettingsHandler::getInverseTcXRollR2());
    connect(inverseXRoll, &QCheckBox::toggled, this, &SettingsDialog::on_inverseTcXRollR2_valueChanged);
    inverseGrid->addWidget(inverseXRoll, 4, 2, Qt::AlignCenter);
}

void SettingsDialog::setAxisProgressBar(QString axis, int value)
{
    emit onAxisValueChange(axis, value);
}

void SettingsDialog::on_axis_valueChange(QString axis, int value)
{
    if (ui.settingsTabWidget->currentWidget() == ui.tcodeTab)
    {
        auto bar = axisProgressbars.value(axis);
        if(bar != nullptr)
            bar->setValue(value);
    }
}

void SettingsDialog::resetAxisProgressBars()
{
    emit onAxisValueReset();
}

void SettingsDialog::on_axis_valueReset()
{
    foreach(auto axisProgressBar, axisProgressbars)
        axisProgressBar->reset();
}

void SettingsDialog::on_inverseTcXL0_valueChanged(bool checked)
{
    SettingsHandler::setInverseTcXL0(checked);
}

void SettingsDialog::on_inverseTcXRollR2_valueChanged(bool checked)
{
    SettingsHandler::setInverseTcXRollR2(checked);
}

void SettingsDialog::on_inverseTcYRollR1_valueChanged(bool checked)
{
    SettingsHandler::setInverseTcYRollR1(checked);
}

void SettingsDialog::on_speedInput_valueChanged(int value)
{
    SettingsHandler::setGamepadSpeed(value);
    SettingsHandler::setLiveGamepadSpeed(value);
}

void SettingsDialog::on_speedIncrementInput_valueChanged(int value)
{
    SettingsHandler::setGamepadSpeedStep(value);
}

void SettingsDialog::xRangeStepInput_valueChanged(int value)
{
    SettingsHandler::setXRangeStep(value);
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
WhirligigHandler* SettingsDialog::getWhirligigHandler()
{
    return _whirligigHandler;
}
GamepadHandler* SettingsDialog::getGamepadHandler()
{
    return _gamepadHandler;
}
bool SettingsDialog::isConnected()
{
    return _outConnectionStatus == ConnectionStatus::Connected;
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
    if (_serialHandler->isRunning())
    {
        _serialHandler->dispose();
    }
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
    setSelectedDeviceHandler(_serialHandler);
    SettingsHandler::setSelectedDevice(DeviceType::Serial);
    _initFuture = QtConcurrent::run(initSerial, _serialHandler, selectedSerialPort);
}

void SettingsDialog::initNetworkEvent()
{
    if (_udpHandler->isRunning())
    {
        _udpHandler->dispose();
    }
    //ui.serialConnectButton->setEnabled(false);
    if (getSelectedDeviceHandler()->isRunning())
    {
        getSelectedDeviceHandler()->dispose();
    }
    if(_initFuture.isRunning())
    {
        _initFuture.cancel();
        _initFuture.waitForFinished();
    }
    setSelectedDeviceHandler(_udpHandler);
    if(!SettingsHandler::getServerAddress().isEmpty() && !SettingsHandler::getServerPort().isEmpty() &&
        SettingsHandler::getServerAddress() != "0" && SettingsHandler::getServerPort() != "0")
    {
        SettingsHandler::setSelectedDevice(DeviceType::Network);
        setDeviceStatusStyle(ConnectionStatus::Connecting, DeviceType::Network);
        ui.networkConnectButton->setEnabled(false);
        NetworkAddress address { SettingsHandler::getServerAddress(), SettingsHandler::getServerPort().toInt() };
        _initFuture = QtConcurrent::run(initNetwork, _udpHandler, address);
    }
}

void SettingsDialog::initDeoEvent()
{
    if (_deoHandler->isConnected())
    {
        _deoHandler->dispose();
    }
    setDeviceStatusStyle(ConnectionStatus::Connecting, DeviceType::Deo);
    if(!SettingsHandler::getDeoAddress().isEmpty() && !SettingsHandler::getDeoPort().isEmpty() &&
        SettingsHandler::getDeoAddress() != "0" && SettingsHandler::getDeoPort() != "0")
    {
        ui.deoConnectButton->setEnabled(false);
        NetworkAddress address { SettingsHandler::getDeoAddress(), SettingsHandler::getDeoPort().toInt() };
        _deoHandler->init(address);
        //_initDeoFuture = QtConcurrent::run(initDeo, _deoHandler, address);
    }
}

void SettingsDialog::initWhirligigEvent()
{
    if (_whirligigHandler->isConnected())
    {
        _whirligigHandler->dispose();
    }
    setDeviceStatusStyle(ConnectionStatus::Connecting, DeviceType::Whirligig);
    if(!SettingsHandler::getWhirligigAddress().isEmpty() && !SettingsHandler::getWhirligigPort().isEmpty() &&
        SettingsHandler::getWhirligigAddress() != "0" && SettingsHandler::getWhirligigPort() != "0")
    {
        ui.whirligigConnectButton->setEnabled(false);
        NetworkAddress address { SettingsHandler::getWhirligigAddress(), SettingsHandler::getWhirligigPort().toInt() };
        _whirligigHandler->init(address);
    }
}

void initSerial(SerialHandler* serialHandler, SerialComboboxItem serialInfo)
{
    serialHandler->init(serialInfo.portName);
}

void initNetwork(UdpHandler* udpHandler, NetworkAddress address)
{
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
    else if (deviceType == DeviceType::Whirligig)
    {
        ui.whirligigStatuslbl->setText(statusUnicode + " " + message);
        ui.whirligigStatuslbl->setFont(font);
        ui.whirligigStatuslbl->setStyleSheet("color: " + statusColor);
    }
    else if (deviceType == DeviceType::Gamepad)
    {
        ui.gamepadStatusLbl->setText(statusUnicode + " " + message);
        ui.gamepadStatusLbl->setFont(font);
        ui.gamepadStatusLbl->setStyleSheet("color: " + statusColor);
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

void SettingsDialog::onRange_valueChanged(QString name, int value)
{
    RangeSlider* slider = rangeSliders.value(name);
    rangeMinLabels.value(name)->setText(QString::number(slider->GetLowerValue()));
    rangeMaxLabels.value(name)->setText(QString::number(slider->GetUpperValue()));
    if ((!_videoHandler->isPlaying() || _videoHandler->isPaused() || SettingsHandler::getLiveActionPaused()) && !_deoHandler->isPlaying() && getSelectedDeviceHandler()->isRunning())
    {
        char tcodeValueString[4];
        sprintf(tcodeValueString, "%03d", value);
        getSelectedDeviceHandler()->sendTCode(name + QString(tcodeValueString) + "S1000");
    }
}

void SettingsDialog::onRange_mouseRelease(QString name)
{
    RangeSlider* slider = rangeSliders.value(name);
    SettingsHandler::setChannelUserMin(name, slider->GetLowerValue());
    SettingsHandler::setChannelUserMax(name, slider->GetUpperValue());
}

void SettingsDialog::onOffSet_valueChanged(int value)
{
    SettingsHandler::setoffSet(value);
}

void SettingsDialog::on_deo_connectionChanged(ConnectionChangedSignal event)
{
    _deoConnectionStatus = event.status;
    if (event.status == ConnectionStatus::Error)
    {
        ui.deoConnectButton->setEnabled(true);
        setDeviceStatusStyle(event.status, event.deviceType, event.message);
    }
    else
    {
        if (event.status == ConnectionStatus::Connected || event.status == ConnectionStatus::Connecting)
        {
            ui.deoConnectButton->setEnabled(false);
        }
        else if(SettingsHandler::getDeoEnabled())
        {
            ui.deoConnectButton->setEnabled(true);
        }
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
    _outConnectionStatus = event.status;
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

void SettingsDialog::on_whirligig_connectionChanged(ConnectionChangedSignal event)
{
    _whirligigConnectionStatus = event.status;
    if (event.status == ConnectionStatus::Error)
    {
        ui.whirligigConnectButton->setEnabled(true);
        setDeviceStatusStyle(event.status, event.deviceType, event.message);
    }
    else
    {
        if (event.status == ConnectionStatus::Connected || event.status == ConnectionStatus::Connecting)
        {
            ui.whirligigConnectButton->setEnabled(false);
        }
        else if(SettingsHandler::getDeoEnabled())
        {
            ui.whirligigConnectButton->setEnabled(true);
        }
        setDeviceStatusStyle(event.status, event.deviceType);
    }
    emit whirligigDeviceConnectionChange({event.deviceType, event.status, event.message});
}
void SettingsDialog::on_whirligig_error(QString error)
{
    emit whirligigDeviceError(error);
}

void SettingsDialog::on_gamepad_connectionChanged(ConnectionChangedSignal event)
{
    _gamepadConnectionStatus = event.status;
    setDeviceStatusStyle(event.status, event.deviceType);
    SettingsHandler::setLiveGamepadConnected(event.status == ConnectionStatus::Connected);
    emit gamepadConnectionChange(event);
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
    ui.serialConnectButton->setEnabled(true);
    SerialComboboxItem serialInfo = ui.SerialOutputCmb->currentData(Qt::UserRole).value<SerialComboboxItem>();
    selectedSerialPort = serialInfo;
}

void SettingsDialog::on_networkAddressTxt_editingFinished()
{
    ui.networkConnectButton->setEnabled(true);
    SettingsHandler::setServerAddress(ui.networkAddressTxt->text());
}

void SettingsDialog::on_networkPortTxt_editingFinished()
{
    ui.networkConnectButton->setEnabled(true);
    SettingsHandler::setServerPort(ui.networkPortTxt->text());
}

void SettingsDialog::on_deoAddressTxt_editingFinished()
{
    ui.deoConnectButton->setEnabled(true);
    SettingsHandler::setDeoAddress(ui.deoAddressTxt->text());
}

void SettingsDialog::on_deoPortTxt_editingFinished()
{
    ui.deoConnectButton->setEnabled(true);
    SettingsHandler::setDeoPort(ui.deoPortTxt->text());
}

void SettingsDialog::on_enableMultiplierCheckbox_clicked(bool checked)
{
    SettingsHandler::setMultiplierEnabled(checked);
}

void SettingsDialog::on_serialConnectButton_clicked()
{
    auto portName = selectedSerialPort.portName;
    if(portName.isEmpty())
    {
        LogHandler::Dialog("No portname specified", XLogLevel::Critical);
        return;
    }
    else if(ui.SerialOutputCmb->count() == 0)
    {
        LogHandler::Dialog("No ports on machine", XLogLevel::Critical);
        return;
    }
    else if(!boolinq::from(serialPorts).any([portName](const SerialComboboxItem &x) { return x.portName == portName; }))
    {
        LogHandler::Dialog("Port: "+ portName + " not found", XLogLevel::Critical);
        return;
    }
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
    if(SettingsHandler::getDeoEnabled())
    {
        if(SettingsHandler::getDeoAddress() != "" && SettingsHandler::getDeoPort() != "" &&
         SettingsHandler::getDeoAddress() != "0" && SettingsHandler::getDeoPort() != "0")
        {
            initDeoEvent();
        }
        else
        {
            LogHandler::Dialog("Invalid deo vr address!", XLogLevel::Warning);
        }
    }
}

void SettingsDialog::on_deoCheckbox_clicked(bool checked)
{
    if(checked && ui.whirligigCheckBox->isChecked())
    {
        ui.whirligigCheckBox->setChecked(!checked);
        on_whirligigCheckBox_clicked(!checked);
    }
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
    if(!checked)
        LogHandler::ExportDebug();
}

void SettingsDialog::on_resetAllButton_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "WARNING!", "Are you sure you want to reset ALL settings?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        SettingsHandler::Default();
        int reply;
        auto playlists = SettingsHandler::getPlaylists();
        if(playlists.count() > 0)
        {
            reply = QMessageBox::question(this, "WARNING!", "You have one or more playlists.\nDo you wish to keep these?",
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes)
            {
                SettingsHandler::PersistSelectSettings();
            }
        }
        reply = QMessageBox::question(this, "Restart Application?", "Changes will take effect on application restart.\n\n"
                                                                    "Restart this application now?\n\n"
                                                                    "Uninstall will remove ALL settings\nINCLUDING PLAYLISTS\nfrom this PC and close the application\n",
                                      "Restart", "Uninstall", "Quit", 0, 2);
        if (reply == 0)
        {
            QApplication::quit();
            QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
        }
        else if (reply == 1)
        {
            SettingsHandler::Clear();
            QApplication::quit();
        }
        else if (reply == 2)
        {
            QApplication::quit();
        }
    }
}

void SettingsDialog::on_gamePadCheckbox_clicked(bool checked)
{
    SettingsHandler::setGamepadEnabled(checked);
    ui.gamePadMapGroupbox->setHidden(!checked);
    if (checked)
    {
        _gamepadHandler->init();
    }
    else
    {
        _gamepadHandler->dispose();
    }
}

void SettingsDialog::on_videoIncrementSpinBox_valueChanged(int value)
{
    SettingsHandler::setVideoIncrement(value);
}

void SettingsDialog::on_disableTextToSpeechCheckBox_clicked(bool checked)
{
    SettingsHandler::setDisableSpeechToText(checked);
}

void SettingsDialog::on_invertFunscriptXCheckBox_clicked(bool checked)
{
    FunscriptHandler::setInverted(checked);
}

void SettingsDialog::on_whirligigCheckBox_clicked(bool checked)
{
    if(checked && ui.deoCheckbox->isChecked())
    {
        ui.deoCheckbox->setChecked(!checked);
        on_deoCheckbox_clicked(!checked);
    }
    SettingsHandler::setWhirligigEnabled(checked);
    ui.whirligigConnectButton->setEnabled(checked);
    if (!checked)
        _whirligigHandler->dispose();
}

void SettingsDialog::on_whirligigConnectButton_clicked()
{
    if(SettingsHandler::getWhirligigEnabled())
    {
        if(SettingsHandler::getWhirligigAddress() != "" && SettingsHandler::getWhirligigPort() != "" &&
         SettingsHandler::getWhirligigAddress() != "0" && SettingsHandler::getWhirligigPort() != "0")
        {
            initWhirligigEvent();
        }
        else
        {
            LogHandler::Dialog("Invalid whirligig address!", XLogLevel::Warning);
        }
    }
}


void SettingsDialog::on_dialogButtonboxClicked(QAbstractButton* button)
{
    if (ui.buttonBox->buttonRole(button) == QDialogButtonBox::ResetRole)
    {
        on_resetAllButton_clicked();
    }
    else
    {
        hide();
    }
}
void SettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{

}

#include <QInputDialog>
void SettingsDialog::on_channelAddButton_clicked()
{
    bool ok;
    ChannelModel channel = AddChannelDialog::getNewChannel(this, &ok);
    if (ok)
    {
        SettingsHandler::addAxis(channel);
        channelTableViewModel->setMap();
    }
}

void SettingsDialog::on_channelDeleteButton_clicked()
{
    QItemSelectionModel *select = ui.channelTableView->selectionModel();
    if (select->hasSelection())
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "WARNING!", "Are you sure you want to delete the selected items?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            auto selectedRows = select->selectedRows();
            QStringList channelsToDelete;
            foreach(auto row, selectedRows)
            {
                const auto model = row.model();
                const auto channelData = ((ChannelTableViewModel*)model)->getRowData(row.row());
                if (channelData == nullptr || channelData->AxisName == axisNames.None)
                    continue;
                channelsToDelete << channelData->AxisName;
            }
            foreach(auto channel, channelsToDelete)
                SettingsHandler::deleteAxis(channel);
            channelTableViewModel->setMap();
        }
    }
}

void SettingsDialog::on_axisDefaultButton_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "WARNING!", "Are you sure you want to reset the channel map?\nThis will reset ALL range and multiplier settings!\nApp will restart on yes.",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        SettingsHandler::SetupAvailableAxis();
        channelTableViewModel->setMap();
        QApplication::quit();
        QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
    }
}

void SettingsDialog::on_savePriorityButton_clicked()
{
    QStringList stringList;
    QList<DecoderModel> models;
    bool atLeastOneChecked = false;
    for (int i = 0; i < ui.decoderListWidget->count(); ++i)
    {
        bool checked = ui.decoderListWidget->item(i)->checkState() == Qt::CheckState::Checked;
        if(checked)
            atLeastOneChecked = true;
        models.append({ui.decoderListWidget->item(i)->text(), checked});
        stringList.append(ui.decoderListWidget->item(i)->text());
    }
    if(atLeastOneChecked)
    {
        SettingsHandler::setDecoderPriority(models);
        _videoHandler->setDecoderPriority();
    }
    else
    {
        LogHandler::Dialog("At least one decoder must be checked!", XLogLevel::Critical);
    }
}

void SettingsDialog::on_cancelPriorityButton_clicked()
{
    ui.decoderListWidget->clear();
    QStringList stringList;
    QList<DecoderModel> models = SettingsHandler::getDecoderPriority();
    foreach (auto model, models)
        stringList.append(model.Name);
    ui.decoderListWidget->addItems(stringList);
    for (int i = 0; i < ui.decoderListWidget->count(); ++i)
        ui.decoderListWidget->item(i)->setCheckState(models[i].Enabled ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
}

void SettingsDialog::on_tCodeHome_clicked()
{
    emit TCodeHomeClicked();
}

void SettingsDialog::on_defaultPriorityButton_clicked()
{
    SettingsHandler::SetupDecoderPriority();
    on_cancelPriorityButton_clicked();
}

void SettingsDialog::on_libraryExclusionsBtn_clicked()
{
    _libraryExclusions = new LibraryExclusions(this);
    _libraryExclusions->show();
}

void SettingsDialog::on_passwordButton_clicked()
{
    QString hashedPass = SettingsHandler::GetHashedPass();
    if(hashedPass.isEmpty())
    {
         bool ok;
         QString text = QInputDialog::getText(this, tr("Set password"),
                                              tr("Launch password:"), QLineEdit::PasswordEchoOnEdit,
                                              "", &ok);
         if (ok && !text.isEmpty())
         {
             SettingsHandler::SetHashedPass(encryptPass(text));
             LogHandler::Dialog("Password set.", XLogLevel::Information);
             ui.passwordButton->setText("Change password");
         }
    }
    else
    {
         bool ok;
         QString text = QInputDialog::getText(this, tr("Current password"),
                                              tr("Current password:"), QLineEdit::Password,
                                              "", &ok);
         if (ok && !text.isEmpty())
         {
             if(CheckPass(text))
             {
                 QString text = QInputDialog::getText(this, tr("Change password"),
                                                      tr("New password (Leave blank to remove protection):"), QLineEdit::PasswordEchoOnEdit,
                                                      "", &ok);
                 if (ok)
                 {
                     if(text.isEmpty())
                     {
                         SettingsHandler::SetHashedPass(nullptr);
                        ui.passwordButton->setText("Set password");
                     }
                     else
                     {
                        SettingsHandler::SetHashedPass(encryptPass(text));
                        ui.passwordButton->setText("Change password");
                     }
                     LogHandler::Dialog("Password changed!", XLogLevel::Information);
                 }
             }
             else
             {
                 LogHandler::Dialog("Password incorrect!", XLogLevel::Warning);
             }
         }
    }
}

PasswordResponse SettingsDialog::GetLaunchPass()
{
     bool ok;
     QString text = QInputDialog::getText(this, tr("STOP!"),
                                          tr("Password:"), QLineEdit::Password,
                                          "", &ok);
     if (ok && !text.isEmpty())
     {
         return CheckPass(text);
     }
     return PasswordResponse::CANCEL ;
}

bool SettingsDialog::HasLaunchPass()
{
    return !SettingsHandler::GetHashedPass().isEmpty();
}

PasswordResponse SettingsDialog::CheckPass(QString pass)
{
    //QString encrypted = encryptPass(pass);
    QString stored = decryptPass(SettingsHandler::GetHashedPass());
    return pass == stored ? PasswordResponse::CORRECT : PasswordResponse::INCORRECT;
}

QString SettingsDialog::encryptPass(QString pass)
{
    SimpleCrypt crypto(Q_UINT64_C(0xcafbb6143ff01257)); //some random number

    //Encryption
    return crypto.encryptToString(pass);
}

QString SettingsDialog::decryptPass(QString pass)
{
    SimpleCrypt crypto(Q_UINT64_C(0xcafbb6143ff01257)); //some random number

    //Encryption
    return crypto.decryptToString(pass);
}

void SettingsDialog::on_exportButton_clicked()
{
    SettingsHandler::Export(this);
}

void SettingsDialog::on_importButton_clicked()
{
    SettingsHandler::Import(this);
}
