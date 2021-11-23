#include "settingsdialog.h"
//http://192.168.0.145/toggleContinousTwist
SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
    // Disable the videoRendererComboBox cause it causes issues with the full screen player controls
    //hasVideoPlayed = true;
    //ui.videoRendererComboBox->setToolTip("WARNING: may cause issues!");
    //ui.ConnectionSettings->setContentsMargins(20,20,20,20);
    _serialHandler = new SerialHandler(this);
    _udpHandler = new UdpHandler(this);
    _deoHandler = new DeoHandler(this);
    _xtpWebHandler = new XTPWebHandler(this);
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
    connect(_xtpWebHandler, &XTPWebHandler::connectionChange, this, &SettingsDialog::on_xtpWeb_connectionChanged);
    connect(_xtpWebHandler, &XTPWebHandler::errorOccurred, this, &SettingsDialog::on_xtpWeb_error);
    connect(_gamepadHandler, &GamepadHandler::connectionChange, this, &SettingsDialog::on_gamepad_connectionChanged);
    connect(ui.buttonBox, & QDialogButtonBox::clicked, this, &SettingsDialog::on_dialogButtonboxClicked);
    connect(this, &SettingsDialog::loadingDialogClose, this, &SettingsDialog::on_close_loading_dialog);
    connect(&SettingsHandler::instance(), &SettingsHandler::settingsChanged, this, &SettingsDialog::on_settingsChange);
}
SettingsDialog::~SettingsDialog()
{
}
void SettingsDialog::on_settingsChange(bool dirty)
{
    saveAllBtn->setEnabled(dirty);
    closeBtn->setEnabled(dirty);
    saveBtn->setEnabled(dirty);
}
void SettingsDialog::dispose()
{
    _udpHandler->dispose();
    _serialHandler->dispose();
    _deoHandler->dispose();
    _whirligigHandler->dispose();
    _gamepadHandler->dispose();
    _xtpWebHandler->dispose();
    if(_initFuture.isRunning())
    {
        //_initFuture.cancel();
        _initFuture.waitForFinished();
    }
    LogHandler::ExportDebug();
    if(_httpHandler)
        delete _httpHandler;
    delete _serialHandler;
    delete _udpHandler;
    delete _deoHandler;
    delete _whirligigHandler;
    delete _xtpWebHandler;;
    delete _gamepadHandler;
}

void SettingsDialog::init(VideoHandler* videoHandler)
{
    _videoHandler = videoHandler;
    if(SettingsHandler::getEnableHttpServer())
        _httpHandler = new HttpHandler(videoHandler, this);
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
    else if(SettingsHandler::getEnableHttpServer() && SettingsHandler::getXTPWebSyncEnabled())
    {
        setDeviceStatusStyle(ConnectionStatus::Disconnected, DeviceType::XTPWeb);
        initXTPWebEvent();
    }
    if(SettingsHandler::getGamepadEnabled())
    {
        setDeviceStatusStyle(ConnectionStatus::Disconnected, DeviceType::Gamepad);
        _gamepadHandler->init();
    }
}

void SettingsDialog::initLive()
{
//    if(_videoHandler->isPlaying())
//        _hasVideoPlayed = true;
//    ui.videoRendererComboBox->setEnabled(!_hasVideoPlayed);
    ui.enableMultiplierCheckbox->setChecked(SettingsHandler::getMultiplierEnabled());
    setUpMultiplierUi(SettingsHandler::getMultiplierEnabled());
    ui.disableNoScriptFoundInLibrary->setChecked(SettingsHandler::getDisableNoScriptFound());
    if(HasLaunchPass())
        ui.passwordButton->setText("Change password");
    ui.hideWelcomeDialog->setChecked(SettingsHandler::getHideWelcomeScreen());
    ui.finscriptModifierSpinBox->setValue(FunscriptHandler::getModifier());
//    auto availableAxis = SettingsHandler::getAvailableAxis();
//    foreach(auto channel, availableAxis->keys())
//    {
//        ChannelModel axis = SettingsHandler::getAxis(channel);
//        QCheckBox* invertedCheckbox = ui.FunscriptSettingsGrid->findChild<QCheckBox*>(axis.FriendlyName);
//        if(invertedCheckbox != nullptr)
//            invertedCheckbox->setChecked(SettingsHandler::getChannelInverseChecked(channel));
//    }
}

void SettingsDialog::reject()
{
    if(_requiresRestart)
    {
        _requiresRestart = false;
        SettingsHandler::askRestart(this, "Some changes made requires a restart.\nWould you like to restart now?");
    }
    QDialog::reject();
}

void SettingsDialog::on_dialogButtonboxClicked(QAbstractButton* button)
{
    if (ui.buttonBox->buttonRole(button) == QDialogButtonBox::ResetRole)
    {
        on_resetAllButton_clicked();
    }
    else if (ui.buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
    {
        LogHandler::Loading(this, "Saving settings...");
        QtConcurrent::run([this] ()
        {
            SettingsHandler::Save();
            emit loadingDialogClose();
        });
    }
    else if (ui.buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
    {
        if(SettingsHandler::getSettingsChanged())
        {
            LogHandler::Loading(this, "Saving settings...");
            QtConcurrent::run([this] ()
            {
                SettingsHandler::Save();
                emit loadingDialogClose();
            });
        }
    }
    if(_requiresRestart && (ui.buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole || ui.buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole))
    {
        _requiresRestart = false;
        SettingsHandler::askRestart(this, "Some changes made requires a restart.\nWould you like to restart now?");
    }
    if (ui.buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
    {
        QDialog::reject();
    }
}

void SettingsDialog::setupUi()
{

    loadSerialPorts();
    setDeviceStatusStyle(_outDeviceConnectionStatus, DeviceType::Serial);
    setDeviceStatusStyle(_outDeviceConnectionStatus, DeviceType::Network);
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
        saveAllBtn = ui.buttonBox->button(QDialogButtonBox::SaveAll);
        saveAllBtn->setAutoDefault(false);
        saveAllBtn->setDefault(false);
        saveAllBtn->setEnabled(false);
        QPushButton* restireDefaultsBtn = ui.buttonBox->button(QDialogButtonBox::RestoreDefaults);
        restireDefaultsBtn->setAutoDefault(false);
        restireDefaultsBtn->setDefault(false);
        saveBtn = ui.buttonBox->button(QDialogButtonBox::Apply);
        saveBtn->setAutoDefault(false);
        saveBtn->setDefault(false);
        saveBtn->setEnabled(false);
        saveBtn->setText("Save all and close");
        closeBtn = ui.buttonBox->button(QDialogButtonBox::Close);
        closeBtn->setText("Save later");
        closeBtn->setAutoDefault(false);
        closeBtn->setDefault(false);
        closeBtn->setEnabled(false);
        // TCode version
        foreach(auto version, SettingsHandler::SupportedTCodeVersions.keys())
        {
            QVariant variant;
            variant.setValue(version);
            ui.tCodeVersionComboBox->addItem(SettingsHandler::SupportedTCodeVersions.value(version), variant);
        }
        auto versionTCode = SettingsHandler::getSelectedTCodeVersion();
        ui.tCodeVersionComboBox->setCurrentText(SettingsHandler::getSelectedTCodeVersion());
        connect(ui.tCodeVersionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::on_tCodeVSComboBox_currentIndexChanged);
        connect(&SettingsHandler::instance(), &SettingsHandler::tcodeVersionChanged, this, &SettingsDialog::setUpTCodeAxis);

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

        foreach(auto renderer, XVideoRendererMap.keys())
        {
            ui.videoRendererComboBox->addItem(renderer);
        }
        ui.videoRendererComboBox->setToolTip("WARNING!: May cause issues with interface!\nDue to a bug, this can only be changed before ANY video has been played.");
        ui.videoRendererComboBox->setCurrentText(XVideoRendererReverseMap.value(SettingsHandler::getSelectedVideoRenderer()));
        connect(ui.videoRendererComboBox, &QComboBox::currentTextChanged, this, &SettingsDialog::on_videoRenderer_textChanged);

        ui.disableTCodeValidationCheckbox->setChecked(SettingsHandler::getDisableSerialTCodeValidation());

        ui.RangeSettingsGrid->setSpacing(5);

        setUpTCodeAxis();

        if(SettingsHandler::getSelectedDevice() == DeviceType::Serial)
        {
            ui.serialOutputRdo->setChecked(true);
        }
        else if (SettingsHandler::getSelectedDevice() == DeviceType::Network)
        {
            ui.networkOutputRdo->setChecked(true);
        }
        enableOrDisableDeviceConnectionUI((DeviceType)SettingsHandler::getSelectedDevice());
        bool deoEnabled = SettingsHandler::getDeoEnabled();
        bool whiriligigEnabled = SettingsHandler::getWhirligigEnabled();
        bool xtpWebEnabled = SettingsHandler::getXTPWebSyncEnabled();
        if(deoEnabled)
        {
            ui.deoCheckbox->setChecked(deoEnabled);
            on_deoCheckbox_clicked(deoEnabled);
        } else if(whiriligigEnabled)
        {
            ui.whirligigCheckBox->setChecked(whiriligigEnabled);
            on_whirligigCheckBox_clicked(whiriligigEnabled);
        }
        else if(xtpWebEnabled)
        {
            ui.xtpWebHandlerCheckbox->setChecked(xtpWebEnabled);
            on_xtpWebHandlerCheckbox_clicked(xtpWebEnabled);
        }

        ui.gamePadCheckbox->setChecked(SettingsHandler::getGamepadEnabled());
        ui.gamePadMapGroupbox->setHidden(!SettingsHandler::getGamepadEnabled());
        ui.videoIncrementSpinBox->setValue(SettingsHandler::getVideoIncrement());
        ui.disableTextToSpeechCheckBox->setChecked(SettingsHandler::getDisableSpeechToText());
        ui.disableVRScriptNotFoundCheckbox->setChecked(SettingsHandler::getDisableVRScriptSelect());
        //Load user decoder priority. (Too lazy to make a new function sue me...)
        on_cancelPriorityButton_clicked();

        connect(ui.SerialOutputCmb, &QComboBox::currentTextChanged, this, [](const QString value)
        {
            SettingsHandler::setSerialPort(value);
        });
        connect(ui.videoIncrementSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::on_videoIncrement_valueChanged);


        ui.showLoneFunscriptsInLibraryCheckbox->setChecked(SettingsHandler::getHideStandAloneFunscriptsInLibrary());
        ui.skipStandAloneFunscriptsInMainLibraryPlaylist->setChecked(SettingsHandler::getSkipPlayingStandAloneFunscriptsInLibrary());

        auto skipToMoneyShotPlaysFunscript = SettingsHandler::getSkipToMoneyShotPlaysFunscript();
        ui.skipToMoneyShotPlaysFunscriptCheckbox->setChecked(skipToMoneyShotPlaysFunscript);
        ui.skipToMoneyShotFunscriptLineEdit->setText(SettingsHandler::getSkipToMoneyShotFunscript());
        ui.skipToMoneyShotFunscriptLineEdit->setDisabled(true);
        ui.skipToMoneyShotSkipsVideo->setChecked(SettingsHandler::getSkipToMoneyShotSkipsVideo());
        ui.skipToMoneyShotSkipsVideo->setEnabled(skipToMoneyShotPlaysFunscript);
        ui.browseSkipToMoneyShotFunscriptButton->setEnabled(skipToMoneyShotPlaysFunscript);
        ui.skipToMoneyShotStandAloneLoopCheckBox->setChecked(SettingsHandler::getSkipToMoneyShotStandAloneLoop());
        ui.skipToMoneyShotStandAloneLoopCheckBox->setEnabled(skipToMoneyShotPlaysFunscript);

        ui.enableHttpServerCheckbox->setChecked(SettingsHandler::getEnableHttpServer());
        ui.httpServerOptions->setVisible(SettingsHandler::getEnableHttpServer());
        ui.httpRootLineEdit->setText(SettingsHandler::getHttpServerRoot());
        ui.vrLibraryLineEdit->setText(SettingsHandler::getVRLibrary());
        ui.chunkSizeDoubleSpinBox->setValue(SettingsHandler::getHTTPChunkSize() / 1048576);
        ui.httpPortSpinBox->setValue(SettingsHandler::getHTTPPort());
        connect(ui.chunkSizeDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SettingsDialog::on_chunkSizeDouble_valueChanged);
        connect(ui.httpPortSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::on_httpPort_valueChanged);


        ui.offsetSpinbox->setMinimum(std::numeric_limits<int>::lowest());
        ui.offsetSpinbox->setMaximum(std::numeric_limits<int>::max());
        ui.offsetSpinbox->setValue(SettingsHandler::getoffSet());
        connect(ui.offsetSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onOffSet_valueChanged);

        ui.offsetSpinboxStep->setMinimum(std::numeric_limits<int>::lowest());
        ui.offsetSpinboxStep->setMaximum(std::numeric_limits<int>::max());
        ui.offsetSpinboxStep->setValue(SettingsHandler::getFunscriptOffsetStep());
        connect(ui.offsetSpinboxStep, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onOffSetStep_valueChanged);

        ui.rangeModifierStepSpinBox->setMinimum(std::numeric_limits<int>::lowest());
        ui.rangeModifierStepSpinBox->setMaximum(std::numeric_limits<int>::max());
        ui.rangeModifierStepSpinBox->setValue(SettingsHandler::getFunscriptModifierStep());
        connect(ui.rangeModifierStepSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::onRangeModifierStep_valueChanged);



        _interfaceInitialized = true;
    }
}

void SettingsDialog::setupGamepadMap()
{
    if(_interfaceInitialized)
    {
        QLayoutItem *child;
        while ((child = ui.gamePadMapGridLayout->takeAt(0)) != 0)
        {
            //setParent is NULL, preventing the interface from disappearing after deletion.
            if(child->widget())
            {
                child->widget()->setParent(NULL);
            }

            delete child;
        }
    }
    auto gamepadMap = SettingsHandler::getGamePadMap();
    auto availableAxis = SettingsHandler::getAvailableAxis();
    auto tcodeChannels = TCodeChannelLookup::GetSelectedVersionMap();
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
        mapComboBox->setObjectName(button);
        foreach(auto axis, tcodeChannels.keys())
        {
            auto channel = availableAxis->value(TCodeChannelLookup::ToString(axis));
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
                [this, mapComboBox, gamepadMap, button](int index)
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

    QPushButton* resetGamepadMap = new QPushButton("Reset gamepad map", this);
    connect(resetGamepadMap, &QPushButton::clicked, this,
            [this, gamepadMap, availableAxis]()
              {
                    QMessageBox::StandardButton reply;
                    reply = QMessageBox::question(this, "WARNING!", "Are you sure you want to reset the gamepad map?",
                                                  QMessageBox::Yes|QMessageBox::No);
                    if (reply == QMessageBox::Yes)
                    {
                        MediaActions actions;
                        SettingsHandler::SetGamepadMapDefaults();
                        foreach(auto button, gamepadMap->keys())
                        {
                            if (button == "None")
                                continue;
                            auto mapComboBox = ui.gamePadMapGroupbox->findChild<QComboBox*>(button);
                            auto gameMap = gamepadMap->value(button);
                            if (availableAxis->contains(gameMap))
                                mapComboBox->setCurrentText(availableAxis->value(gameMap).FriendlyName);
                            else
                                mapComboBox->setCurrentText(actions.Values.value(gameMap));
                        }
                    }
              });
    inverseGrid->addWidget(resetGamepadMap, 1, 1, Qt::AlignCenter);

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
    inverseGrid->addWidget(inverseX, 3, 0, Qt::AlignCenter);
    QCheckBox* inverseYRoll = new QCheckBox(this);
    inverseYRoll->setText("Inverse Roll");
    inverseYRoll->setChecked(SettingsHandler::getInverseTcYRollR1());
    connect(inverseYRoll, &QCheckBox::toggled, this, &SettingsDialog::on_inverseTcYRollR1_valueChanged);
    inverseGrid->addWidget(inverseYRoll, 3, 1, Qt::AlignCenter);
    QCheckBox* inverseXRoll = new QCheckBox(this);
    inverseXRoll->setText("Inverse Pitch");
    inverseXRoll->setChecked(SettingsHandler::getInverseTcXRollR2());
    connect(inverseXRoll, &QCheckBox::toggled, this, &SettingsDialog::on_inverseTcXRollR2_valueChanged);
    inverseGrid->addWidget(inverseXRoll, 3, 2, Qt::AlignCenter);
}
QList<QWidget*> _multiplierWidgets;
void SettingsDialog::setUpTCodeAxis()
{
    if(_interfaceInitialized)
    {
        QLayoutItem *child;
        while ((child = ui.RangeSettingsGrid->takeAt(0)) != 0)
        {
            //setParent is NULL, preventing the interface from disappearing after deletion.
            if(child->widget())
            {
                child->widget()->setParent(NULL);
            }

            delete child;
        }
        while ((child = ui.MultiplierSettingsGrid->takeAt(0)) != 0)
        {
            if(child->widget())
            {
                child->widget()->setParent(NULL);
            }

            delete child;
        }
    }
     QFont font( "Sans Serif", 8);
     int sliderGridRow = 0;
     int multiplierGridRow = 1;
     int funscriptSettingsGridRow = 0;
     auto tcodeChannels = TCodeChannelLookup::GetSelectedVersionMap();
     _multiplierWidgets.clear();
     qDeleteAll(_multiplierWidgets);
     foreach(auto channel, tcodeChannels.keys())
     {
         QString channelName = TCodeChannelLookup::ToString(channel);
         ChannelModel axis = SettingsHandler::getAxis(channelName);
         if(axis.Type == AxisType::None || axis.Type == AxisType::HalfRange)
             continue;

         int userMin = axis.UserMin;
         int userMid = axis.UserMid;
         int userMax = axis.UserMax;
         QLabel* rangeMinLabel = new QLabel(QString::number(userMin));
         rangeMinLabel->setObjectName(axis.AxisName+"RangeMinLabel");
         rangeMinLabel->setFont(font);
         rangeMinLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
         ui.RangeSettingsGrid->addWidget(rangeMinLabel, sliderGridRow, 0);
         rangeMinLabels.insert(channelName, rangeMinLabel);

         QLabel* rangeLabel = new QLabel(axis.FriendlyName + " Range mid: " + QString::number(userMid));
         rangeLabel->setObjectName(axis.AxisName+"RangeLabel");
         rangeLabel->setFont(font);
         rangeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
         ui.RangeSettingsGrid->addWidget(rangeLabel, sliderGridRow, 1);
         rangeLabels.insert(channelName, rangeLabel);

         QLabel* rangeMaxLabel = new QLabel(QString::number(userMax));
         rangeMaxLabel->setObjectName(axis.AxisName+"RangeMaxLabel");
         rangeMaxLabel->setFont(font);
         rangeMaxLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
         ui.RangeSettingsGrid->addWidget(rangeMaxLabel, sliderGridRow, 2);
         rangeMaxLabels.insert(channelName, rangeMaxLabel);

         RangeSlider* axisRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, this);
         axisRangeSlider->setObjectName(axis.AxisName+"RangeSlider");
         axisRangeSlider->SetRange(axis.Min, axis.Max);
         axisRangeSlider->setLowerValue(userMin);
         axisRangeSlider->setUpperValue(userMax);
         axisRangeSlider->SetMinimumRange(1);
         axisRangeSlider->setName(channelName);// Required
         sliderGridRow++;
         ui.RangeSettingsGrid->addWidget(axisRangeSlider, sliderGridRow,0,1,3);
         rangeSliders.insert(channelName, axisRangeSlider);
         sliderGridRow++;

         QProgressBar* funscriptProgressbar = new QProgressBar(this);
         funscriptProgressbar->setObjectName(axis.AxisName+"FunscriptStatus");
         funscriptProgressbar->setMinimum(0);
         funscriptProgressbar->setMaximum(100);
         funscriptProgressbar->setMaximumHeight(5);
         ui.RangeSettingsGrid->addWidget(funscriptProgressbar, sliderGridRow,0,1,3);
         axisProgressbars.insert(channelName, funscriptProgressbar);
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
             multiplierCheckbox->setChecked(SettingsHandler::getMultiplierChecked(channelName));
             QDoubleSpinBox* multiplierInput = new QDoubleSpinBox(this);
             multiplierInput->setDecimals(3);
             multiplierInput->setSingleStep(0.1f);
             multiplierInput->setMinimum(std::numeric_limits<int>::lowest());
             multiplierInput->setMaximum(std::numeric_limits<int>::max());
             multiplierInput->setValue(SettingsHandler::getMultiplierValue(channelName));
             connect(multiplierInput, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                     [this, channelName](float value)
                       {
                         SettingsHandler::setMultiplierValue(channelName, value);
                       });
             connect(multiplierCheckbox, &QCheckBox::clicked, this,
                     [this, channelName](bool checked)
                       {
                         SettingsHandler::setMultiplierChecked(channelName, checked);
                       });
             QCheckBox* damperCheckbox = new QCheckBox(this);
             damperCheckbox->setText("Speed");
             damperCheckbox->setChecked(SettingsHandler::getDamperChecked(channelName));
             QDoubleSpinBox* damperInput = new QDoubleSpinBox(this);
             damperInput->setToolTip("Multiply the speed by the value.\n4000 * 0.5 = 2000");
             damperInput->setDecimals(1);
             damperInput->setSingleStep(0.1f);
             damperInput->setMinimum(0.1f);
             damperInput->setMaximum(std::numeric_limits<int>::max());
             damperInput->setValue(SettingsHandler::getDamperValue(channelName));
             connect(damperInput, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                     [this, channelName](float value)
                       {
                         SettingsHandler::setDamperValue(channelName, value);
                       });
             connect(damperCheckbox, &QCheckBox::clicked, this,
                     [this, channelName](bool checked)
                       {
                         SettingsHandler::setDamperChecked(channelName, checked);
                       });


             ui.MultiplierSettingsGrid->addWidget(multiplierCheckbox, multiplierGridRow, 0, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
             ui.MultiplierSettingsGrid->addWidget(multiplierInput, multiplierGridRow, 1, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
             QCheckBox* linkCheckbox = new QCheckBox(this);
             auto relatedChannel = SettingsHandler::getAxis(axis.RelatedChannel);
             linkCheckbox->setToolTip("This will link the channel to the related axis.\nThis will remove the random calculation and just link\nthe current MFS " + relatedChannel.FriendlyName + " funscript value.\nIf there is no " + relatedChannel.FriendlyName + " funscript then it will default to random motion.");
             linkCheckbox->setText("Link to MFS: ");
             linkCheckbox->setChecked(SettingsHandler::getLinkToRelatedAxisChecked(channelName));
             connect(linkCheckbox, &QCheckBox::clicked, this,
                     [this, channelName](bool checked)
                       {
                         SettingsHandler::setLinkToRelatedAxisChecked(channelName, checked);
                       });

             QComboBox* linkToAxisCombobox = new QComboBox(this);
             foreach(auto axis, tcodeChannels.keys())
             {
                 auto channel = SettingsHandler::getAxis(TCodeChannelLookup::ToString(axis));
                 if(channel.AxisName == channelName || channel.Type == AxisType::HalfRange || channel.AxisName == TCodeChannelLookup::None())
                     continue;
                 QVariant variant;
                 variant.setValue(channel);
                 linkToAxisCombobox->addItem(channel.FriendlyName, variant);
             }
             linkToAxisCombobox->setCurrentText(relatedChannel.FriendlyName);
             connect(linkToAxisCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                     [this, channelName, linkToAxisCombobox, linkCheckbox](int value)
                       {
                            auto relatedChannel = linkToAxisCombobox->currentData().value<ChannelModel>();
                            linkCheckbox->setToolTip("This will link the channel to the related axis.\nThis will remove the random calculation and just link\nthe current MFS (Multi-funscript) " + relatedChannel.FriendlyName + " funscript value.\nIf there is no " + relatedChannel.FriendlyName + " funscript then it will default to random motion.");
                            SettingsHandler::setLinkToRelatedAxis(channelName, relatedChannel.AxisName);
                       });

             ui.MultiplierSettingsGrid->addWidget(linkCheckbox, multiplierGridRow, 2, 1, 1, Qt::AlignRight | Qt::AlignVCenter);
             ui.MultiplierSettingsGrid->addWidget(linkToAxisCombobox, multiplierGridRow, 3, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
             ui.MultiplierSettingsGrid->addWidget(damperCheckbox, multiplierGridRow, 4, 1, 1, Qt::AlignRight | Qt::AlignVCenter);
             ui.MultiplierSettingsGrid->addWidget(damperInput, multiplierGridRow, 5, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

             _multiplierWidgets.append(multiplierCheckbox);
             _multiplierWidgets.append(multiplierInput);
             _multiplierWidgets.append(linkCheckbox);
             _multiplierWidgets.append(linkToAxisCombobox);
             _multiplierWidgets.append(damperCheckbox);
             _multiplierWidgets.append(damperInput);

             multiplierGridRow++;
         }
         QCheckBox* invertedCheckbox = new QCheckBox(this);
         invertedCheckbox->setText(axis.FriendlyName);
         invertedCheckbox->setChecked(SettingsHandler::getChannelInverseChecked(channelName));
         connect(invertedCheckbox, &QCheckBox::clicked, this,
                 [this, channelName](bool checked)
                   {
                     SettingsHandler::setChannelInverseChecked(channelName, checked);
                   });
         ui.FunscriptSettingsGrid->addWidget(invertedCheckbox, funscriptSettingsGridRow, 0, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

         funscriptSettingsGridRow++;
     }

     setUpMultiplierUi(SettingsHandler::getMultiplierEnabled());


     QPushButton* zeroOutButton = new QPushButton(this);
     zeroOutButton->setText("All axis home");
     connect(zeroOutButton, & QPushButton::clicked, this, &SettingsDialog::on_tCodeHome_clicked);
     ui.RangeSettingsGrid->addWidget(zeroOutButton, sliderGridRow + 1, 0);

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

     setupGamepadMap();
}

void SettingsDialog::setUpMultiplierUi(bool enabled)
{
    foreach(auto widget, _multiplierWidgets)
        widget->setHidden(!enabled);
}

void SettingsDialog::setLibraryLoaded(bool loaded, QList<LibraryListWidgetItem*> cachedLibraryItems, QList<LibraryListWidgetItem*> vrLibraryItems)
{
    if(_httpHandler)
        _httpHandler->setLibraryLoaded(loaded, cachedLibraryItems, vrLibraryItems);
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
XTPWebHandler* SettingsDialog::getXTPWebHandler()
{
    return _xtpWebHandler;
}
GamepadHandler* SettingsDialog::getGamepadHandler()
{
    return _gamepadHandler;
}
bool SettingsDialog::isDeviceConnected()
{
    return selectedDeviceHandler->isConnected();
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

void SettingsDialog::initXTPWebEvent()
{
    if (_xtpWebHandler->isConnected())
    {
        _xtpWebHandler->dispose();
    }
    _xtpWebHandler->init(_httpHandler);
    _connectedVRHandler = _xtpWebHandler;
    setDeviceStatusStyle(ConnectionStatus::Connecting, DeviceType::XTPWeb);
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
    else if (deviceType == DeviceType::XTPWeb)
    {
        ui.xtpWebStatuslbl->setText(statusUnicode + " " + message);
        ui.xtpWebStatuslbl->setFont(font);
        ui.xtpWebStatuslbl->setStyleSheet("color: " + statusColor);
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
    enableOrDisableDeviceConnectionUI(DeviceType::Serial);
    SettingsHandler::setSelectedDevice(DeviceType::Serial);
}

void SettingsDialog::on_networkOutputRdo_clicked()
{
    enableOrDisableDeviceConnectionUI(DeviceType::Network);
    SettingsHandler::setSelectedDevice(DeviceType::Network);
}
void SettingsDialog::enableOrDisableDeviceConnectionUI(DeviceType deviceType)
{
    if(deviceType == DeviceType::Network)
    {
        ui.SerialOutputCmb->setEnabled(false);
        ui.networkAddressTxt->setEnabled(true);
        ui.networkPortTxt->setEnabled(true);
        ui.serialConnectButton->setEnabled(false);
        ui.networkConnectButton->setEnabled(true);
        ui.serialRefreshBtn->setEnabled(false);
    }
    else if(deviceType == DeviceType::Serial)
    {
        ui.SerialOutputCmb->setEnabled(true);;
        ui.networkAddressTxt->setEnabled(false);
        ui.networkPortTxt->setEnabled(false);
        ui.networkConnectButton->setEnabled(false);
        ui.serialConnectButton->setEnabled(true);
        ui.serialRefreshBtn->setEnabled(true);
    }
}
void SettingsDialog::on_serialRefreshBtn_clicked()
{
    loadSerialPorts();
}

void SettingsDialog::onRange_valueChanged(QString name, int value)
{
    RangeSlider* slider = rangeSliders.value(name);
    auto channel = SettingsHandler::getAxis(name);
    auto mainLabel = rangeLabels.value(name);
    int max = slider->GetUpperValue();
    int min = slider->GetLowerValue();
    rangeMinLabels.value(name)->setText(QString::number(min));
    rangeMaxLabels.value(name)->setText(QString::number(max));
    mainLabel->setText(channel.FriendlyName + " mid: " + QString::number(XMath::middle(min, max)));
    if ((!_videoHandler->isPlaying() || _videoHandler->isPaused() || SettingsHandler::getLiveActionPaused()) && !_deoHandler->isPlaying() && getSelectedDeviceHandler()->isRunning())
    {
        getSelectedDeviceHandler()->sendTCode(name + QString::number(value).rightJustified(SettingsHandler::getTCodePadding(), '0')+ "S1000");
    }
}

void SettingsDialog::onRange_mouseRelease(QString name)
{
    RangeSlider* slider = rangeSliders.value(name);
    int max = slider->GetUpperValue();
    int min = slider->GetLowerValue();
    SettingsHandler::setChannelUserMin(name, min);
    SettingsHandler::setChannelUserMax(name, max);
    SettingsHandler::setChannelUserMid(name, XMath::middle(min, max));
}

void SettingsDialog::onOffSet_valueChanged(int value)
{
    SettingsHandler::setoffSet(value);
}

void SettingsDialog::onOffSetStep_valueChanged(int value)
{
    SettingsHandler::setFunscriptOffsetStep(value);
}

void SettingsDialog::onRangeModifierStep_valueChanged(int value)
{
    SettingsHandler::setFunscriptModifierStep(value);
}

void SettingsDialog::on_deo_connectionChanged(ConnectionChangedSignal event)
{
    _deoConnectionStatus = event.status;
    if (event.status == ConnectionStatus::Error)
    {
        _connectedVRHandler = nullptr;
        ui.deoConnectButton->setEnabled(true);
        setDeviceStatusStyle(event.status, event.deviceType, event.message);
    }
    else
    {
        if(_whirligigHandler->isConnected())
        {
            _whirligigHandler->dispose();
        }
        if(_xtpWebHandler->isConnected())
        {
            _xtpWebHandler->dispose();
        }
        if (event.status == ConnectionStatus::Connected || event.status == ConnectionStatus::Connecting)
        {
            ui.deoConnectButton->setEnabled(false);
        }
        else if(SettingsHandler::getDeoEnabled())
        {
            ui.deoConnectButton->setEnabled(true);
        }
        setDeviceStatusStyle(event.status, event.deviceType);
        _connectedVRHandler = _deoHandler;
    }
    emit deoDeviceConnectionChange({event.deviceType, event.status, event.message});
}

void SettingsDialog::on_deo_error(QString error)
{
    _connectedVRHandler = nullptr;
    emit deoDeviceError(error);
}
void SettingsDialog::on_device_connectionChanged(ConnectionChangedSignal event)
{
    _outDeviceConnectionStatus = event.status;
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
        _connectedVRHandler = nullptr;
    }
    else
    {
        if(_deoHandler->isConnected())
        {
            _deoHandler->dispose();
        }
        if(_xtpWebHandler->isConnected())
        {
            _xtpWebHandler->dispose();
        }
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
    _connectedVRHandler = _whirligigHandler;
    emit whirligigDeviceConnectionChange({event.deviceType, event.status, event.message});
}

void SettingsDialog::on_whirligig_error(QString error)
{
    emit whirligigDeviceError(error);
}

void SettingsDialog::on_xtpWeb_connectionChanged(ConnectionChangedSignal event)
{
    _xtpWebConnectionStatus = event.status;
    if (event.status == ConnectionStatus::Error)
    {
        setDeviceStatusStyle(event.status, event.deviceType, event.message);
        _connectedVRHandler = nullptr;
    }
    else
    {
        if(_deoHandler->isConnected())
        {
            _deoHandler->dispose();
        }
        if(_whirligigHandler->isConnected())
        {
            _whirligigHandler->dispose();
        }
        if (event.status == ConnectionStatus::Connected || event.status == ConnectionStatus::Connecting)
        {
            //ui.whirligigConnectButton->setEnabled(false);
        }
        else if(SettingsHandler::getDeoEnabled())
        {
            //ui.whirligigConnectButton->setEnabled(true);
        }
        setDeviceStatusStyle(event.status, event.deviceType);
    }
    emit xtpWebDeviceConnectionChange({event.deviceType, event.status, event.message});
}
void SettingsDialog::on_xtpWeb_error(QString error)
{
    emit xtpWebDeviceError(error);
}

VRDeviceHandler* SettingsDialog::getConnectedVRHandler() {
    return _connectedVRHandler;
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
    setUpMultiplierUi(checked);
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
    if(checked)
    {
        ui.whirligigCheckBox->setChecked(!checked);
        on_whirligigCheckBox_clicked(!checked);
        ui.xtpWebHandlerCheckbox->setChecked(!checked);
        on_xtpWebHandlerCheckbox_clicked(!checked);
    }
    SettingsHandler::setDeoEnabled(checked);
    ui.deoAddressTxt->setEnabled(checked);
    ui.deoPortTxt->setEnabled(checked);
    ui.deoConnectButton->setEnabled(checked);
    if (!checked)
        _deoHandler->dispose();
}

void SettingsDialog::on_whirligigCheckBox_clicked(bool checked)
{
    if(checked)
    {
        ui.deoCheckbox->setChecked(!checked);
        on_deoCheckbox_clicked(!checked);
        ui.xtpWebHandlerCheckbox->setChecked(!checked);
        on_xtpWebHandlerCheckbox_clicked(!checked);
    }
    SettingsHandler::setWhirligigEnabled(checked);
    ui.whirligigConnectButton->setEnabled(checked);
    if (!checked)
        _whirligigHandler->dispose();
}

void SettingsDialog::on_xtpWebHandlerCheckbox_clicked(bool checked)
{
    if(checked && !SettingsHandler::getEnableHttpServer()) {
        LogHandler::Dialog("XTP web is not enabled on the 'Web' tab. Set it up and return here afterwards.", XLogLevel::Information);
        ui.xtpWebHandlerCheckbox->setChecked(false);
        return;
    }
    if(checked)
    {
        ui.whirligigCheckBox->setChecked(!checked);
        on_whirligigCheckBox_clicked(!checked);
        ui.deoCheckbox->setChecked(!checked);
        on_deoCheckbox_clicked(!checked);
    }

    SettingsHandler::setXTPWebSyncEnabled(checked);
    if (!checked)
        _xtpWebHandler->dispose();
    else
        initXTPWebEvent();
}

void SettingsDialog::on_checkBox_clicked(bool checked)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "WARNING!", "Restart the app in debug mode?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        QApplication::quit();
        QProcess::startDetached(qApp->arguments()[0], QStringList("-debug"));
    }
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

void SettingsDialog::on_videoIncrement_valueChanged(int value)
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

void SettingsDialog::on_channelAddButton_clicked()
{
    bool ok;
    ChannelModel channel = AddChannelDialog::getNewChannel(this, &ok);
    if (ok)
    {
        SettingsHandler::addAxis(channel);
        channelTableViewModel->setMap();
        setUpTCodeAxis();
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
                if (channelData == nullptr || channelData->AxisName == TCodeChannelLookup::None())
                    continue;
                channelsToDelete << channelData->AxisName;
            }
            foreach(auto channel, channelsToDelete)
                SettingsHandler::deleteAxis(channel);
            channelTableViewModel->setMap();
            setUpTCodeAxis();
        }
    }
}

void SettingsDialog::on_axisDefaultButton_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "WARNING!", "Are you sure you want to reset the channel map?\nThis will reset ALL range and multiplier settings!",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        SettingsHandler::SetChannelMapDefaults();
        channelTableViewModel->setMap();
        setUpTCodeAxis();
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
             if(CheckPass(text) == PasswordResponse::CORRECT)
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

void SettingsDialog::on_thumbDirButton_clicked()
{
    SettingsHandler::setSelectedThumbsDir(this);
}
void SettingsDialog::on_thumbsDirDefaultButton_clicked()
{
    SettingsHandler::setSelectedThumbsDirDefault(this);
}

void SettingsDialog::on_disableVRScriptNotFoundCheckbox_stateChanged(int checkState)
{
    SettingsHandler::setDisableVRScriptSelect(checkState == Qt::CheckState::Checked);
}

void SettingsDialog::on_disableNoScriptFoundInLibrary_stateChanged(int checkState)
{
    SettingsHandler::setDisableNoScriptFound(checkState == Qt::CheckState::Checked);
}

void SettingsDialog::on_tCodeVSComboBox_currentIndexChanged(int index)
{
    SettingsHandler::setSelectedTCodeVersion(ui.tCodeVersionComboBox->currentData().value<TCodeVersion>());
}

void SettingsDialog::on_hideWelcomeDialog_clicked(bool checked)
{
    SettingsHandler::setHideWelcomeScreen(checked);
}

void SettingsDialog::on_launchWelcomeDialog_clicked()
{
    emit onOpenWelcomeDialog();
}

void SettingsDialog::on_videoRenderer_textChanged(const QString &value)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Warning!", "Changing the renderer can cause issues. Particularly in full screen.\nIf you have issues, REMEMBER your current renderer:\n("+XVideoRendererReverseMap.value(SettingsHandler::getSelectedVideoRenderer())+")\nSo you can change it back. The default renderer is OpenGLWidget.\nA restart WILL be required.\nChange your renderer?",
                                  QMessageBox::Yes|QMessageBox::No);
    XVideoRenderer renderer = XVideoRendererMap.value(value);
    if(reply == QMessageBox::Yes && _videoHandler->setVideoRenderer(renderer))
    {
        SettingsHandler::setSelectedVideoRenderer(renderer);
        SettingsHandler::requestRestart(this);
    }
    else
    {
        disconnect(ui.videoRendererComboBox, &QComboBox::currentTextChanged, this, &SettingsDialog::on_videoRenderer_textChanged);
        ui.videoRendererComboBox->setCurrentText(XVideoRendererReverseMap.value(SettingsHandler::getSelectedVideoRenderer()));
        connect(ui.videoRendererComboBox, &QComboBox::currentTextChanged, this, &SettingsDialog::on_videoRenderer_textChanged);
    }
}

void SettingsDialog::on_disableTCodeValidationCheckbox_clicked(bool checked)
{
    if(checked)
    {
        LogHandler::Dialog("Make sure to verify the version of TCode firmware installed on your device.", XLogLevel::Warning);
    }
    SettingsHandler::setDisableSerialTCodeValidation(checked);
    SettingsHandler::requestRestart(this);
}

void SettingsDialog::on_close_loading_dialog()
{
    LogHandler::LoadingClose();
}

void SettingsDialog::on_showLoneFunscriptsInLibraryCheckbox_clicked(bool checked)
{
    SettingsHandler::setHideStandAloneFunscriptsInLibrary(checked);
    ui.httpServerOptions->setVisible(SettingsHandler::getEnableHttpServer());
    _requiresRestart = true;
}

void SettingsDialog::on_skipStandAloneFunscriptsInMainLibraryPlaylist_clicked(bool checked)
{
    SettingsHandler::setSkipPlayingStandAloneFunscriptsInLibrary(checked);
}

void SettingsDialog::on_skipToMoneyShotPlaysFunscriptCheckbox_clicked(bool checked)
{
    SettingsHandler::setSkipToMoneyShotPlaysFunscript(checked);
    ui.skipToMoneyShotSkipsVideo->setEnabled(checked);
    ui.browseSkipToMoneyShotFunscriptButton->setEnabled(checked);
    ui.skipToMoneyShotStandAloneLoopCheckBox->setEnabled(checked);
}

void SettingsDialog::on_browseSkipToMoneyShotFunscriptButton_clicked(bool checked)
{
    QString selectedScript = QFileDialog::getOpenFileName(this, tr("Choose script"), SettingsHandler::getSelectedLibrary(), tr("Scripts (*.funscript *.zip)"));
    if (selectedScript != Q_NULLPTR)
    {
        SettingsHandler::setSkipToMoneyShotFunscript(selectedScript);
        ui.skipToMoneyShotFunscriptLineEdit->setText(selectedScript);
    }
}

void SettingsDialog::on_skipToMoneyShotSkipsVideo_clicked(bool checked)
{
    SettingsHandler::setSkipToMoneyShotSkipsVideo(checked);
}

void SettingsDialog::on_skipToMoneyShotStandAloneLoopCheckBox_clicked(bool checked)
{
    SettingsHandler::setSkipToMoneyShotStandAloneLoop(checked);
}

void SettingsDialog::on_enableHttpServerCheckbox_clicked(bool checked)
{
    SettingsHandler::setEnableHttpServer(checked);
    ui.httpServerOptions->setVisible(checked);
    _requiresRestart = true;
    if(!checked)
        SettingsHandler::setXTPWebSyncEnabled(false);
}

void SettingsDialog::on_browseHttpRootButton_clicked()
{
    QString selectedDirectory = QFileDialog::getExistingDirectory(this, tr("Choose HTTP root"), SettingsHandler::getSelectedLibrary());
    if (selectedDirectory != Q_NULLPTR)
    {
        SettingsHandler::setHttpServerRoot(selectedDirectory);
        ui.httpRootLineEdit->setText(selectedDirectory);
    }
}

void SettingsDialog::on_browseVRLibraryButton_clicked()
{
    QString selectedDirectory = QFileDialog::getExistingDirectory(this, tr("Choose VR library"), SettingsHandler::getSelectedLibrary());
    if (selectedDirectory != Q_NULLPTR)
    {
        SettingsHandler::setVRLibrary(selectedDirectory);
        ui.vrLibraryLineEdit->setText(selectedDirectory);
    }
}

void SettingsDialog::on_httpPort_valueChanged(int value)
{
    SettingsHandler::setHTTPPort(value);
    if(SettingsHandler::getEnableHttpServer())
        _requiresRestart = true;
}

void SettingsDialog::on_chunkSizeDouble_valueChanged(double value)
{
    SettingsHandler::setHTTPChunkSize(value * 1048576);
}

void SettingsDialog::on_httpRootLineEdit_textEdited(const QString &selectedDirectory)
{
    SettingsHandler::setHttpServerRoot(selectedDirectory);
    if(SettingsHandler::getEnableHttpServer())
        _requiresRestart = true;
}

void SettingsDialog::on_vrLibraryLineEdit_textEdited(const QString &selectedDirectory)
{
    SettingsHandler::setVRLibrary(selectedDirectory);
    if(SettingsHandler::getEnableHttpServer())
        _requiresRestart = true;
}

void SettingsDialog::on_finscriptModifierSpinBox_valueChanged(int value)
{
    FunscriptHandler::setModifier(value);
}
