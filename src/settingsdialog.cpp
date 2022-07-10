#include "settingsdialog.h"
//http://192.168.0.145/toggleContinousTwist
SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
    setModal(false);
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
    _connectionHandler->dispose();
}

void SettingsDialog::init(VideoHandler* videoHandler, SyncHandler* syncHandler, TCodeHandler* tcodeHandler,  ConnectionHandler* connectionHandler)
{
    _videoHandler = videoHandler;
    _syncHandler = syncHandler;
    _connectionHandler = connectionHandler;
    _tcodeHandler = tcodeHandler;
    connect(_connectionHandler, &ConnectionHandler::outputConnectionChange, this, &SettingsDialog::on_output_device_connectionChanged);
    connect(_connectionHandler, &ConnectionHandler::inputConnectionChange, this, &SettingsDialog::on_input_device_connectionChanged);
    connect(_connectionHandler, &ConnectionHandler::gamepadConnectionChange, this, &SettingsDialog::on_gamepad_connectionChanged);
    connect(_syncHandler, &SyncHandler::channelPositionChange, this, &SettingsDialog::setAxisProgressBar, Qt::QueuedConnection);
    connect(_syncHandler, &SyncHandler::funscriptEnded, this, &SettingsDialog::resetAxisProgressBars, Qt::QueuedConnection);

    setupUi();
    if(SettingsHandler::getGamepadEnabled())
    {
        setDeviceStatusStyle(ConnectionStatus::Disconnected, DeviceName::Gamepad);
    }
    if(SettingsHandler::getSelectedOutputDevice() == DeviceName::Serial)
    {
        initSerialEvent();
    }
    else if (SettingsHandler::getSelectedOutputDevice() == DeviceName::Network)
    {
        initNetworkEvent();
    }
    initInputDevice();
}

void SettingsDialog::initInputDevice()
{
    DeviceName deviceName = SettingsHandler::getSelectedInputDevice();
    setDeviceStatusStyle(ConnectionStatus::Disconnected, deviceName);
    if(deviceName == DeviceName::Deo)
    {
        initDeoEvent();
    }
    else if(deviceName == DeviceName::Whirligig)
    {
        initWhirligigEvent();
    }
    else if(SettingsHandler::getEnableHttpServer() && deviceName == DeviceName::XTPWeb)
    {
        initXTPWebEvent();
    }
    if(SettingsHandler::getGamepadEnabled()) {
        _connectionHandler->initInputDevice(DeviceName::Gamepad);
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
        askRestart(this, "Some changes made requires a restart.\nWould you like to restart now?");
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
        DialogHandler::Loading(this, "Saving settings...");
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
            DialogHandler::Loading(this, "Saving settings...");
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
        askRestart(this, "Some changes made requires a restart.\nWould you like to restart now?");
    }
    if (ui.buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
    {
        QDialog::reject();
    }
}

void SettingsDialog::setupUi()
{

    loadSerialPorts();
    setDeviceStatusStyle(ConnectionStatus::Disconnected, DeviceName::Serial);
    setDeviceStatusStyle(ConnectionStatus::Disconnected, DeviceName::Network);
    setDeviceStatusStyle(ConnectionStatus::Disconnected, DeviceName::Deo);
    setDeviceStatusStyle(ConnectionStatus::Disconnected, DeviceName::Whirligig);
    setDeviceStatusStyle(ConnectionStatus::Disconnected, DeviceName::Gamepad);
    if(SettingsHandler::getSelectedOutputDevice() == DeviceName::Serial)
    {
        ui.serialOutputRdo->setChecked(true);
    }
    else if (SettingsHandler::getSelectedOutputDevice() == DeviceName::Network)
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

        ui.disableTCodeValidationCheckbox->setChecked(SettingsHandler::getDisableTCodeValidation());

        ui.RangeSettingsGrid->setSpacing(5);

        setUpTCodeAxis();

        if(SettingsHandler::getSelectedOutputDevice() == DeviceName::Serial)
        {
            ui.serialOutputRdo->setChecked(true);
        }
        else if (SettingsHandler::getSelectedOutputDevice() == DeviceName::Network)
        {
            ui.networkOutputRdo->setChecked(true);
        }
        enableOrDisableDeviceConnectionUI(SettingsHandler::getSelectedOutputDevice());
        bool deoEnabled = SettingsHandler::getSelectedInputDevice() == DeviceName::Deo;
        bool whiriligigEnabled = SettingsHandler::getSelectedInputDevice() == DeviceName::Whirligig;
        bool xtpWebEnabled = SettingsHandler::getSelectedInputDevice() == DeviceName::XTPWeb;
        if(deoEnabled)
        {
            ui.deoCheckbox->setChecked(deoEnabled);
            on_deoCheckbox_clicked(deoEnabled);
        }
        else if(whiriligigEnabled)
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
        ui.webSocketPortSpinBox->setValue(SettingsHandler::getWebSocketPort());
        ui.httpThumbQualitySpinBox->setValue(SettingsHandler::getHttpThumbQuality());
        connect(ui.chunkSizeDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SettingsDialog::on_chunkSizeDouble_valueChanged);
        connect(ui.httpPortSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::on_httpPort_valueChanged);
        connect(ui.webSocketPortSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::on_webSocketPort_valueChanged);
        connect(ui.httpThumbQualitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::on_httpThumbQualitySpinBox_editingFinished);



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
//        QLayoutItem *child;
//        while ((child = ui.gamePadMapGridLayout->takeAt(0)) != 0)
//        {
//            //setParent is NULL, preventing the interface from disappearing after deletion.
//            if(child->widget())
//            {
//                child->widget()->setParent(NULL);
//            }

//            delete child;
//        }
        delete _inputMapWidget;
    }
    _inputMapWidget = new InputMapWidget(_connectionHandler, this);
    ui.gamePadMapGridLayout->addWidget(_inputMapWidget);
//    auto gamepadMap = SettingsHandler::getGamePadMap();
//    auto availableAxis = SettingsHandler::getAvailableAxis();
//    auto tcodeChannels = TCodeChannelLookup::GetSelectedVersionMap();
//    MediaActions actions;
//    int rowIterator = 0;
//    int columnIterator = 0;
//    int maxRows = 4;//6 total
//    foreach(auto button, gamepadMap->keys())
//    {
//        if (button == "None")
//            continue;
//        QLabel* mapLabel = new QLabel(this);
//        mapLabel->setText(button);
//        QComboBox* mapComboBox = new QComboBox(this);
//        mapComboBox->setObjectName(button);
//        foreach(auto axis, tcodeChannels.keys())
//        {
//            auto channel = availableAxis->value(TCodeChannelLookup::ToString(axis));
//            QVariant variant;
//            variant.setValue(channel);
//            mapComboBox->addItem(channel.FriendlyName, variant);
//        }
//        foreach(auto action, actions.Values.keys())
//        {
//            ChannelModel channel;
//            channel.AxisName = action;
//            channel.FriendlyName = actions.Values.value(action);
//            QVariant variant;
//            variant.setValue(channel);
//            mapComboBox->addItem(channel.FriendlyName, variant);
//        }
//        auto gameMapList = gamepadMap->value(button);
//        auto gameMap = gameMapList.empty() ? TCodeChannelLookup::None() : gameMapList.first();

//        if (availableAxis->contains(gameMap))
//            mapComboBox->setCurrentText(availableAxis->value(gameMap).FriendlyName);
//        else
//            mapComboBox->setCurrentText(actions.Values.value(gameMap));

//        ui.gamePadMapGridLayout->addWidget(mapLabel, rowIterator, columnIterator, Qt::AlignRight);
//        ui.gamePadMapGridLayout->addWidget(mapComboBox, rowIterator, columnIterator + 1, Qt::AlignLeft);

//        connect(mapComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
//                [this, mapComboBox, gamepadMap, button](int index)
//                  {
//                        ChannelModel selectedChannel = mapComboBox->currentData().value<ChannelModel>();
//                        SettingsHandler::setGamePadMapButton(button, selectedChannel.AxisName);
//                  });
//        if (rowIterator <= maxRows)
//            rowIterator++;
//        else
//        {
//            rowIterator = 0;
//            columnIterator += 2;
//        }
//    }
//    QGridLayout* inverseGrid = new QGridLayout(this);
//    QFrame* inverseFrame = new QFrame(this);
//    inverseFrame->setLayout(inverseGrid);
//    ui.gamePadMapGridLayout->addWidget(inverseFrame, maxRows + 2, 0, 1, columnIterator + 2);

//    QLabel* speedLabel = new QLabel(this);
//    speedLabel->setText("Speed");
//    speedLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
//    QSpinBox* speedInput = new QSpinBox(this);
//    speedInput->setMinimum(1);
//    speedInput->setMaximum(std::numeric_limits<int>::max());
//    speedInput->setMinimumWidth(75);
//    speedInput->setSuffix("ms");
//    speedInput->setSingleStep(100);
//    speedInput->setValue(SettingsHandler::getGamepadSpeed());
//    speedInput->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
//    connect(speedInput, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::on_speedInput_valueChanged);
//    inverseGrid->addWidget(speedLabel, 0, 0, Qt::AlignCenter);
//    inverseGrid->addWidget(speedInput, 1, 0, Qt::AlignCenter);

//    QPushButton* resetGamepadMap = new QPushButton("Reset gamepad map", this);
//    connect(resetGamepadMap, &QPushButton::clicked, this,
//            [this, gamepadMap, availableAxis]()
//              {
//                    QMessageBox::StandardButton reply;
//                    reply = QMessageBox::question(this, "WARNING!", "Are you sure you want to reset the gamepad map?",
//                                                  QMessageBox::Yes|QMessageBox::No);
//                    if (reply == QMessageBox::Yes)
//                    {
//                        MediaActions actions;
//                        SettingsHandler::SetGamepadMapDefaults();
//                        foreach(auto button, gamepadMap->keys())
//                        {
//                            if (button == "None")
//                                continue;
//                            auto mapComboBox = ui.gamePadMapGroupbox->findChild<QComboBox*>(button);

//                            auto gameMapList = gamepadMap->value(button);
//                            auto gameMap = gameMapList.empty() ? TCodeChannelLookup::None() : gameMapList.first();

//                            if (availableAxis->contains(gameMap))
//                                mapComboBox->setCurrentText(availableAxis->value(gameMap).FriendlyName);
//                            else
//                                mapComboBox->setCurrentText(actions.Values.value(gameMap));
//                        }
//                    }
//              });
//    inverseGrid->addWidget(resetGamepadMap, 1, 1, Qt::AlignCenter);

//    QLabel* speedIncrementLabel = new QLabel(this);
//    speedIncrementLabel->setText("Speed change step");
//    speedIncrementLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
//    QSpinBox* speedIncrmentInput = new QSpinBox(this);
//    speedIncrmentInput->setMinimum(1);
//    speedIncrmentInput->setMaximum(std::numeric_limits<int>::max());
//    speedIncrmentInput->setMinimumWidth(75);
//    speedIncrmentInput->setSingleStep(100);
//    speedIncrmentInput->setValue(SettingsHandler::getGamepadSpeedIncrement());
//    speedIncrmentInput->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
//    connect(speedIncrmentInput, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::on_speedIncrementInput_valueChanged);
//    inverseGrid->addWidget(speedIncrementLabel, 0, 2, Qt::AlignCenter);
//    inverseGrid->addWidget(speedIncrmentInput, 1, 2, Qt::AlignCenter);

//    QCheckBox* inverseX = new QCheckBox(this);
//    inverseX->setText("Inverse Stroke");
//    inverseX->setChecked(SettingsHandler::getInverseTcXL0());
//    connect(inverseX, &QCheckBox::toggled, this, &SettingsDialog::on_inverseTcXL0_valueChanged);
//    inverseGrid->addWidget(inverseX, 3, 0, Qt::AlignCenter);
//    QCheckBox* inverseYRoll = new QCheckBox(this);
//    inverseYRoll->setText("Inverse Roll");
//    inverseYRoll->setChecked(SettingsHandler::getInverseTcYRollR1());
//    connect(inverseYRoll, &QCheckBox::toggled, this, &SettingsDialog::on_inverseTcYRollR1_valueChanged);
//    inverseGrid->addWidget(inverseYRoll, 3, 1, Qt::AlignCenter);
//    QCheckBox* inverseXRoll = new QCheckBox(this);
//    inverseXRoll->setText("Inverse Pitch");
//    inverseXRoll->setChecked(SettingsHandler::getInverseTcXRollR2());
//    connect(inverseXRoll, &QCheckBox::toggled, this, &SettingsDialog::on_inverseTcXRollR2_valueChanged);
//    inverseGrid->addWidget(inverseXRoll, 3, 2, Qt::AlignCenter);
}

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
         ChannelModel33 axis = SettingsHandler::getAxis(channelName);
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
         ui.RangeSettingsGrid->addWidget(rangeLabel, sliderGridRow, 1, 1, 2, Qt::AlignHCenter | Qt::AlignVCenter);
         rangeLabels.insert(channelName, rangeLabel);

         QLabel* rangeMaxLabel = new QLabel(QString::number(userMax));
         rangeMaxLabel->setObjectName(axis.AxisName+"RangeMaxLabel");
         rangeMaxLabel->setFont(font);
         rangeMaxLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
         ui.RangeSettingsGrid->addWidget(rangeMaxLabel, sliderGridRow, 3);
         rangeMaxLabels.insert(channelName, rangeMaxLabel);

         RangeSlider* axisRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, this);
         axisRangeSlider->setObjectName(axis.AxisName+"RangeSlider");
         axisRangeSlider->SetRange(axis.Min, axis.Max);
         axisRangeSlider->setLowerValue(userMin);
         axisRangeSlider->setUpperValue(userMax);
         axisRangeSlider->SetMinimumRange(1);
         axisRangeSlider->setName(channelName);// Required
         sliderGridRow++;
         ui.RangeSettingsGrid->addWidget(axisRangeSlider, sliderGridRow,0,1,4);
         rangeSliders.insert(channelName, axisRangeSlider);
         sliderGridRow++;

         QProgressBar* funscriptProgressbar = new QProgressBar(this);
         funscriptProgressbar->setObjectName(axis.AxisName+"FunscriptStatus");
         funscriptProgressbar->setMinimum(0);
         funscriptProgressbar->setMaximum(100);
         funscriptProgressbar->setMaximumHeight(5);
         ui.RangeSettingsGrid->addWidget(funscriptProgressbar, sliderGridRow,0,1,4);
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
                         if(!checked)
                            _connectionHandler->sendTCode(_tcodeHandler->getChannelHome(channelName));
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
                 if(channel.AxisName == channelName || channel.Type == AxisType::HalfRange)
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
         invertedCheckbox->setChecked(SettingsHandler::getChannelFunscriptInverseChecked(channelName));
         connect(invertedCheckbox, &QCheckBox::clicked, this,
                 [this, channelName](bool checked)
                   {
                     SettingsHandler::setChannelFunscriptInverseChecked(channelName, checked);
                   });
         ui.FunscriptSettingsGrid->addWidget(invertedCheckbox, funscriptSettingsGridRow, 0, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

         funscriptSettingsGridRow++;
     }

     setUpMultiplierUi(SettingsHandler::getMultiplierEnabled());

     QLabel* lubePulseAmountLabel = new QLabel(this);
     lubePulseAmountLabel->setText("Pulse lube amount");
     lubePulseAmountLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
     QLabel* lubePulseFrequencyLabel = new QLabel(this);
     lubePulseFrequencyLabel->setText("Pulse lube frequency");
     lubePulseFrequencyLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
     QCheckBox* lubePulseCheckbox = new QCheckBox("Pulse lube enabled", this);
     lubePulseCheckbox->setToolTip("Enable a tcode signal to be sent to the selected channel every n ms");
     connect(lubePulseCheckbox, &QCheckBox::clicked, this, &SettingsDialog::lubePulseEnabled_valueChanged);
     lubePulseCheckbox->setChecked(SettingsHandler::getLubePulseEnabled());
     QSpinBox* libePulseAmountInput = new QSpinBox(this);
     auto max = SettingsHandler::getAxis(TCodeChannelLookup::Stroke()).Max;
     libePulseAmountInput->setToolTip("TCode value to be sent to the selected channel between 0-"+QString::number(max));
     libePulseAmountInput->setMinimum(0);
     libePulseAmountInput->setMaximum(max);
     libePulseAmountInput->setMinimumWidth(75);
     libePulseAmountInput->setSingleStep(100);
     libePulseAmountInput->setValue(SettingsHandler::getLubePulseAmount());
     libePulseAmountInput->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
     connect(libePulseAmountInput, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::lubeAmount_valueChanged);
     QSpinBox* libePulseFrequencyInput = new QSpinBox(this);
     libePulseFrequencyInput->setToolTip("Time between pulse sent values in milliseconds");
     libePulseFrequencyInput->setMinimum(0);
     libePulseFrequencyInput->setMaximum(INT_MAX);
     libePulseFrequencyInput->setMinimumWidth(75);
     libePulseFrequencyInput->setSingleStep(500);
     libePulseFrequencyInput->setSuffix("ms");
     libePulseFrequencyInput->setValue(SettingsHandler::getLubePulseFrequency());
     libePulseFrequencyInput->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
     connect(libePulseFrequencyInput, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::lubeFrequency_valueChanged);
     ui.RangeSettingsGrid->addWidget(lubePulseCheckbox, sliderGridRow + 1, 0);
     ui.RangeSettingsGrid->addWidget(lubePulseAmountLabel, sliderGridRow + 1, 0);
     lubePulseCheckbox->raise();
     lubePulseCheckbox->setStyleSheet("* {background: transparent}");
     ui.RangeSettingsGrid->addWidget(libePulseAmountInput, sliderGridRow + 1, 1);
     ui.RangeSettingsGrid->addWidget(lubePulseFrequencyLabel, sliderGridRow + 1, 2);
     ui.RangeSettingsGrid->addWidget(libePulseFrequencyInput, sliderGridRow + 1, 3);

     QPushButton* zeroOutButton = new QPushButton(this);
     zeroOutButton->setText("All axis home");
     connect(zeroOutButton, & QPushButton::clicked, this, &SettingsDialog::on_tCodeHome_clicked);
     ui.RangeSettingsGrid->addWidget(zeroOutButton, sliderGridRow + 4, 0);

     QLabel* xRangeStepLabel = new QLabel(this);
     xRangeStepLabel->setText("Stroke range change step");
     xRangeStepLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
     QSpinBox* xRangeStepInput = new QSpinBox(this);
     xRangeStepInput->setToolTip("The amount to modify the stroke range when using keyboard/gamepad.");
     xRangeStepInput->setMinimum(1);
     xRangeStepInput->setMaximum(INT_MAX);
     xRangeStepInput->setMinimumWidth(75);
     xRangeStepInput->setSingleStep(50);
     xRangeStepInput->setValue(SettingsHandler::getGamepadSpeedIncrement());
     xRangeStepInput->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
     connect(xRangeStepInput, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::xRangeStepInput_valueChanged);
     ui.RangeSettingsGrid->addWidget(xRangeStepLabel, sliderGridRow + 3, 3);
     ui.RangeSettingsGrid->addWidget(xRangeStepInput, sliderGridRow + 4, 3);

     setupGamepadMap();

}

void SettingsDialog::setUpMultiplierUi(bool enabled)
{
    foreach(auto widget, _multiplierWidgets)
        widget->setHidden(!enabled);
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

void SettingsDialog::lubePulseEnabled_valueChanged(bool value)
{
    SettingsHandler::setLubePulseEnabled(value);
}
void SettingsDialog::lubeFrequency_valueChanged(int value)
{
    SettingsHandler::setLubePulseFrequency(value);
}
void SettingsDialog::lubeAmount_valueChanged(int value)
{
    SettingsHandler::setLubePulseAmount(value);
}

//UdpHandler* SettingsDialog::getNetworkHandler()
//{
//    return _udpHandler;
//}

//SerialHandler* SettingsDialog::getSerialHandler()
//{
//    return _serialHandler;
//}

//DeoHandler* SettingsDialog::getDeoHandler()
//{
//    return _deoHandler;
//}
//WhirligigHandler* SettingsDialog::getWhirligigHandler()
//{
//    return _whirligigHandler;
//}
//XTPWebHandler* SettingsDialog::getXTPWebHandler()
//{
//    return _xtpWebHandler;
//}

void SettingsDialog::initSerialEvent()
{
    ui.serialConnectButton->setEnabled(false);
    setDeviceStatusStyle(ConnectionStatus::Connecting, DeviceName::Serial);
    _connectionHandler->initOutputDevice(DeviceName::Serial);
}

void SettingsDialog::initNetworkEvent()
{
    if(!SettingsHandler::getServerAddress().isEmpty() && !SettingsHandler::getServerPort().isEmpty() &&
        SettingsHandler::getServerAddress() != "0" && SettingsHandler::getServerPort() != "0")
    {
        setDeviceStatusStyle(ConnectionStatus::Connecting, DeviceName::Network);
        ui.networkConnectButton->setEnabled(false);
        _connectionHandler->initOutputDevice(DeviceName::Network);
    }
    else
    {
        DialogHandler::MessageBox(this, "Invalid Network address.", XLogLevel::Critical);
    }
}

void SettingsDialog::on_xtpWeb_initSyncDevice(DeviceName deviceName, bool checked)
{
    if(deviceName == DeviceName::Deo) {
        ui.deoCheckbox->setChecked(checked);
        on_deoCheckbox_clicked(checked);
        on_deoConnectButton_clicked();
    } else if(deviceName == DeviceName::Whirligig) {
        ui.whirligigCheckBox->setChecked(checked);
        on_whirligigCheckBox_clicked(checked);
        on_whirligigConnectButton_clicked();
    } else if(deviceName == DeviceName::XTPWeb) {
        ui.xtpWebHandlerCheckbox->setChecked(checked);
        on_xtpWebHandlerCheckbox_clicked(checked);
    } else if(deviceName == DeviceName::Gamepad) {
        ui.gamePadCheckbox->setChecked(checked);
        on_gamePadCheckbox_clicked(checked);
    } else if(deviceName == DeviceName::None) {
        ui.xtpWebHandlerCheckbox->setChecked(false);
        on_xtpWebHandlerCheckbox_clicked(false);
        ui.whirligigCheckBox->setChecked(false);
        on_whirligigCheckBox_clicked(false);
        ui.deoCheckbox->setChecked(false);
        on_deoCheckbox_clicked(false);
    }
    _connectionHandler->initInputDevice(deviceName);
}

void SettingsDialog::initDeoEvent()
{
    setDeviceStatusStyle(ConnectionStatus::Connecting, DeviceName::Deo);
    if(!SettingsHandler::getDeoAddress().isEmpty() && !SettingsHandler::getDeoPort().isEmpty() &&
        SettingsHandler::getDeoAddress() != "0" && SettingsHandler::getDeoPort() != "0")
    {
        ui.deoConnectButton->setEnabled(false);
        _connectionHandler->initInputDevice(DeviceName::Deo);
    }
}

void SettingsDialog::initWhirligigEvent()
{
    setDeviceStatusStyle(ConnectionStatus::Connecting, DeviceName::Whirligig);
    if(!SettingsHandler::getWhirligigAddress().isEmpty() && !SettingsHandler::getWhirligigPort().isEmpty() &&
        SettingsHandler::getWhirligigAddress() != "0" && SettingsHandler::getWhirligigPort() != "0")
    {
        ui.whirligigConnectButton->setEnabled(false);
        _connectionHandler->initInputDevice(DeviceName::Whirligig);
    }
}

void SettingsDialog::initXTPWebEvent()
{
    setDeviceStatusStyle(ConnectionStatus::Connecting, DeviceName::XTPWeb);
    _connectionHandler->initInputDevice(DeviceName::XTPWeb);
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

void SettingsDialog::setDeviceStatusStyle(ConnectionStatus status, DeviceName deviceName, QString message)
{
    QString statusUnicode = "\u2717";
    QString statusColor = "red";
    QFont font( "Sans Serif", 12);
    if(deviceName == DeviceName::Serial || deviceName == DeviceName::Network)
    {
        ui.serialStatuslbl->clear();
        ui.networkStatuslbl->clear();
    }
    else if(deviceName == DeviceName::Deo ||
            deviceName == DeviceName::Whirligig ||
            deviceName == DeviceName::XTPWeb)
    {
        ui.deoStatuslbl->clear();
        ui.whirligigStatuslbl->clear();
        ui.xtpWebStatuslbl->clear();
    }
    else if(deviceName == DeviceName::Gamepad)
        ui.gamepadStatusLbl->clear();
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
    if (deviceName == DeviceName::Serial)
    {
        ui.serialStatuslbl->setText(statusUnicode + " " + message);
        ui.serialStatuslbl->setFont(font);
        ui.serialStatuslbl->setStyleSheet("color: " + statusColor);
    }
    else if (deviceName == DeviceName::Network)
    {
        ui.networkStatuslbl->setText(statusUnicode + " " + message);
        ui.networkStatuslbl->setFont(font);
        ui.networkStatuslbl->setStyleSheet("color: " + statusColor);
    }
    else if (deviceName == DeviceName::Deo)
    {
        ui.deoStatuslbl->setText(statusUnicode + " " + message);
        ui.deoStatuslbl->setFont(font);
        ui.deoStatuslbl->setStyleSheet("color: " + statusColor);
    }
    else if (deviceName == DeviceName::Whirligig)
    {
        ui.whirligigStatuslbl->setText(statusUnicode + " " + message);
        ui.whirligigStatuslbl->setFont(font);
        ui.whirligigStatuslbl->setStyleSheet("color: " + statusColor);
    }
    else if (deviceName == DeviceName::XTPWeb)
    {
        ui.xtpWebStatuslbl->setText(statusUnicode + " " + message);
        ui.xtpWebStatuslbl->setFont(font);
        ui.xtpWebStatuslbl->setStyleSheet("color: " + statusColor);
    }
    else if (deviceName == DeviceName::Gamepad)
    {
        ui.gamepadStatusLbl->setText(statusUnicode + " " + message);
        ui.gamepadStatusLbl->setFont(font);
        ui.gamepadStatusLbl->setStyleSheet("color: " + statusColor);
    }
}

void SettingsDialog::on_serialOutputRdo_clicked()
{
    enableOrDisableDeviceConnectionUI(DeviceName::Serial);
    SettingsHandler::setSelectedDevice(DeviceName::Serial);
}

void SettingsDialog::on_networkOutputRdo_clicked()
{
    enableOrDisableDeviceConnectionUI(DeviceName::Network);
    SettingsHandler::setSelectedDevice(DeviceName::Network);
}

void SettingsDialog::enableOrDisableDeviceConnectionUI(DeviceName deviceName)
{
    if(deviceName == DeviceName::Network)
    {
        ui.SerialOutputCmb->setEnabled(false);
        ui.networkAddressTxt->setEnabled(true);
        ui.networkPortTxt->setEnabled(true);
        ui.serialConnectButton->setEnabled(false);
        ui.networkConnectButton->setEnabled(true);
        ui.serialRefreshBtn->setEnabled(false);
    }
    else if(deviceName == DeviceName::Serial)
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
    OutputDeviceHandler* outputDevice = _connectionHandler->getSelectedOutputDevice();
    InputDeviceHandler* inputDevice = _connectionHandler->getSelectedInputDevice();
    if ((!_videoHandler->isPlaying() || _videoHandler->isPaused() || SettingsHandler::getLiveActionPaused())
        && (!inputDevice || !inputDevice->isPlaying()) && (outputDevice && outputDevice->isRunning()))
    {
        _connectionHandler->sendTCode(name + QString::number(value).rightJustified(SettingsHandler::getTCodePadding(), '0')+ "S1000");
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

void SettingsDialog::on_input_device_connectionChanged(ConnectionChangedSignal event)
{
    bool connectingOrConnected = event.status != ConnectionStatus::Connected && event.status != ConnectionStatus::Connecting;

    if(event.deviceName == DeviceName::Whirligig || event.deviceName == DeviceName::Deo)
    {
        ui.whirligigConnectButton->setEnabled(event.deviceName == DeviceName::Whirligig && connectingOrConnected);
        ui.deoConnectButton->setEnabled(event.deviceName == DeviceName::Deo && connectingOrConnected);
    }
    setDeviceStatusStyle(event.status, event.deviceName, event.message);
}

void SettingsDialog::on_output_device_connectionChanged(ConnectionChangedSignal event)
{
    if (event.deviceName == DeviceName::Serial)
    {
        ui.serialConnectButton->setEnabled(event.status == ConnectionStatus::Error || event.status == ConnectionStatus::Disconnected);
    }
    else if (event.deviceName == DeviceName::Network)
    {
        ui.networkConnectButton->setEnabled(event.status == ConnectionStatus::Error || event.status == ConnectionStatus::Disconnected);
    }
    setDeviceStatusStyle(event.status, event.deviceName);
}

void SettingsDialog::on_gamepad_connectionChanged(ConnectionChangedSignal event)
{
    setDeviceStatusStyle(event.status, event.deviceName);
    SettingsHandler::setLiveGamepadConnected(event.status == ConnectionStatus::Connected);
}

void SettingsDialog::on_SerialOutputCmb_currentIndexChanged(int index)
{
    if(SettingsHandler::getSelectedOutputDevice() == DeviceName::Serial)
        ui.serialConnectButton->setEnabled(true);
    SerialComboboxItem serialInfo = ui.SerialOutputCmb->currentData(Qt::UserRole).value<SerialComboboxItem>();
    selectedSerialPort = serialInfo;
}

void SettingsDialog::on_networkAddressTxt_editingFinished()
{
    if(SettingsHandler::getSelectedOutputDevice() == DeviceName::Network)
        ui.networkConnectButton->setEnabled(true);
    SettingsHandler::setServerAddress(ui.networkAddressTxt->text());
}

void SettingsDialog::on_networkPortTxt_editingFinished()
{
    if(SettingsHandler::getSelectedOutputDevice() == DeviceName::Network)
        ui.networkConnectButton->setEnabled(true);
    SettingsHandler::setServerPort(ui.networkPortTxt->text());
}

void SettingsDialog::on_deoAddressTxt_editingFinished()
{
    if(SettingsHandler::getSelectedInputDevice() == DeviceName::Deo)
        ui.deoConnectButton->setEnabled(true);
    SettingsHandler::setDeoAddress(ui.deoAddressTxt->text());
}

void SettingsDialog::on_deoPortTxt_editingFinished()
{
    if(SettingsHandler::getSelectedInputDevice() == DeviceName::Deo)
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
        DialogHandler::MessageBox(this, "No portname specified", XLogLevel::Critical);
        return;
    }
    else if(ui.SerialOutputCmb->count() == 0)
    {
        DialogHandler::MessageBox(this, "No ports on machine", XLogLevel::Critical);
        return;
    }
    else if(!boolinq::from(serialPorts).any([portName](const SerialComboboxItem &x) { return x.portName == portName; }))
    {
        DialogHandler::MessageBox(this, "Port: "+ portName + " not found", XLogLevel::Critical);
        return;
    }
    SettingsHandler::setSerialPort(portName);
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
        DialogHandler::MessageBox(this, "Invalid network address!", XLogLevel::Critical);
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
        DialogHandler::MessageBox(this, "Invalid deo vr address!", XLogLevel::Warning);
    }
}

void SettingsDialog::on_whirligigConnectButton_clicked()
{
    if(SettingsHandler::getWhirligigAddress() != "" && SettingsHandler::getWhirligigPort() != "" &&
     SettingsHandler::getWhirligigAddress() != "0" && SettingsHandler::getWhirligigPort() != "0")
    {
        initWhirligigEvent();
    }
    else
    {
        DialogHandler::MessageBox(this, "Invalid whirligig address!", XLogLevel::Warning);
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
    ui.deoAddressTxt->setEnabled(checked);
    ui.deoPortTxt->setEnabled(checked);
    ui.deoConnectButton->setEnabled(checked);
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
    ui.whirligigConnectButton->setEnabled(checked);
}

void SettingsDialog::on_xtpWebHandlerCheckbox_clicked(bool checked)
{
    if(checked && !SettingsHandler::getEnableHttpServer()) {
        DialogHandler::MessageBox(this, "XTP web is not enabled on the 'Web' tab. Set it up and return here afterwards.", XLogLevel::Information);
        ui.xtpWebHandlerCheckbox->setChecked(false);
        return;
    }
    if(checked)
    {
        ui.whirligigCheckBox->setChecked(!checked);
        on_whirligigCheckBox_clicked(!checked);
        ui.deoCheckbox->setChecked(!checked);
        on_deoCheckbox_clicked(!checked);
        initXTPWebEvent();
    }
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
        _connectionHandler->initInputDevice(DeviceName::Gamepad);
    else
        _connectionHandler->disposeInputDevice(DeviceName::Gamepad);
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

void SettingsDialog::on_channelAddButton_clicked()
{
    bool ok;
    ChannelModel33 channel = AddChannelDialog::getNewChannel(this, &ok);
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
                if (channelData == nullptr)
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

void SettingsDialog::on_tCodeHome_clicked()
{
    emit TCodeHomeClicked();
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
             DialogHandler::MessageBox(this, "Password set.", XLogLevel::Information);
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
                     DialogHandler::MessageBox(this, "Password changed!", XLogLevel::Information);
                 }
             }
             else
             {
                 DialogHandler::MessageBox(this, "Password incorrect!", XLogLevel::Warning);
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
    Export(this);
}

void SettingsDialog::on_importButton_clicked()
{
    Import(this);
}

void SettingsDialog::on_thumbDirButton_clicked()
{
    auto selectedThumbsDir = SettingsHandler::getSelectedThumbsDir();
   auto customThumbDirExists  = !selectedThumbsDir.isEmpty() && QFileInfo::exists(selectedThumbsDir);
   QString selectedDir = QFileDialog::getExistingDirectory(this, QFileDialog::tr("Choose thumbnail storage directory"), customThumbDirExists ? selectedThumbsDir : QApplication::applicationDirPath() + "/thumbs/", QFileDialog::ReadOnly);
   if (selectedDir != Q_NULLPTR)
   {
       SettingsHandler::setSelectedThumbsDir(selectedDir);
       SettingsHandler::Save();
       requestRestart(this);
   }
}
void SettingsDialog::on_thumbsDirDefaultButton_clicked()
{
    SettingsHandler::setSelectedThumbsDirDefault();
    ui.useMediaDirectoryCheckbox->setChecked(false);
    on_useMediaDirectoryCheckbox_clicked(false);
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
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Warning!", "This will reset your ranges and any custom channel settings to default.\nContinue?",
                                  QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::Yes)
    {
        SettingsHandler::setSelectedTCodeVersion(ui.tCodeVersionComboBox->currentData().value<TCodeVersion>());
    }
    else
    {
        disconnect(ui.tCodeVersionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::on_tCodeVSComboBox_currentIndexChanged);
        ui.tCodeVersionComboBox->setCurrentText(SettingsHandler::getSelectedTCodeVersion());
        connect(ui.tCodeVersionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::on_tCodeVSComboBox_currentIndexChanged);
    }
}

void SettingsDialog::on_hideWelcomeDialog_clicked(bool checked)
{
    SettingsHandler::setHideWelcomeScreen(checked);
}

void SettingsDialog::on_launchWelcomeDialog_clicked()
{
    emit onOpenWelcomeDialog();
}

void SettingsDialog::on_disableTCodeValidationCheckbox_clicked(bool checked)
{
    if(checked)
    {
        DialogHandler::MessageBox(this, "Make sure to verify the version of TCode firmware installed on your device.", XLogLevel::Warning);
    }
    SettingsHandler::setDisableTCodeValidation(checked);
    requestRestart(this);
}

void SettingsDialog::on_close_loading_dialog()
{
    DialogHandler::LoadingClose();
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
    _requiresRestart = !checked;
    if(!checked && SettingsHandler::getSelectedInputDevice() == DeviceName::XTPWeb)
        SettingsHandler::setSelectedInputDevice(DeviceName::None);
}

void SettingsDialog::on_browseHttpRootButton_clicked()
{
    QString selectedDirectory = QFileDialog::getExistingDirectory(this, tr("Choose HTTP root"), SettingsHandler::getSelectedLibrary());
    if (selectedDirectory != Q_NULLPTR)
    {
        SettingsHandler::setHttpServerRoot(selectedDirectory);
        ui.httpRootLineEdit->setText(selectedDirectory);
        if(SettingsHandler::getEnableHttpServer())
            _requiresRestart = true;
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
}

void SettingsDialog::on_webSocketPort_valueChanged(int value)
{
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
}

void SettingsDialog::on_finscriptModifierSpinBox_valueChanged(int value)
{
    FunscriptHandler::setModifier(value);
}

void SettingsDialog::on_useMediaDirectoryCheckbox_clicked(bool checked)
{
    ui.thumbDirButton->setDisabled(checked);
    SettingsHandler::setUseMediaDirForThumbs(checked);
}

void SettingsDialog::on_httpPortSpinBox_editingFinished()
{
    int value = ui.httpPortSpinBox->value();
    if(SettingsHandler::getWebSocketPort() == value) {
        DialogHandler::MessageBox(this, "Http port cannot be the same as the Wwb socket port.", XLogLevel::Critical);
        ui.httpPortSpinBox->setValue(SettingsHandler::getHTTPPort());
        return;
    }
    SettingsHandler::setHTTPPort(value);
    if(SettingsHandler::getEnableHttpServer())
        _requiresRestart = true;
}

void SettingsDialog::on_webSocketPortSpinBox_editingFinished()
{
    int value = ui.webSocketPortSpinBox->value();
    if(SettingsHandler::getHTTPPort() == value) {
        DialogHandler::MessageBox(this, "Web socket port cannot be the same as the http port.", XLogLevel::Critical);
        ui.webSocketPortSpinBox->setValue(SettingsHandler::getWebSocketPort());
        return;
    }
    SettingsHandler::setWebSocketPort(value);
    if(SettingsHandler::getEnableHttpServer())
        _requiresRestart = true;
}

void SettingsDialog::on_httpThumbQualitySpinBox_editingFinished()
{
    SettingsHandler::setHttpThumbQuality(ui.httpThumbQualitySpinBox->value());
}

void SettingsDialog::Export(QWidget* parent)
{
    QString selectedFile = QFileDialog::getSaveFileName(parent, QApplication::applicationDirPath() + "/Save settings ini", "settings_export.ini", "INI Files (*.ini)");
    if(!selectedFile.isEmpty())
    {
        QSettings* settingsExport = new QSettings(selectedFile, QSettings::Format::IniFormat);
        SettingsHandler::Save(settingsExport);
        delete settingsExport;
        emit messageSend("Settings saved to "+ selectedFile, XLogLevel::Information);
    }
}

void SettingsDialog::Import(QWidget* parent)
{
    QString selectedFile = QFileDialog::getOpenFileName(parent, "Choose settings ini", QApplication::applicationDirPath(), "INI Files (*.ini)");
    if(!selectedFile.isEmpty())
    {
        QSettings* settingsImport = new QSettings(selectedFile, QSettings::Format::IniFormat);
        SettingsHandler::Load(settingsImport);
        SettingsHandler::Save();
        SettingsHandler::setSaveOnExit(false);
        delete settingsImport;
        requestRestart(parent);
    }
}

void SettingsDialog::requestRestart(QWidget* parent)
{
    int value = QMessageBox::question(parent, "Restart Application", "Changes will take effect on application restart.",
                                  "Exit XTP", "Restart now", 0, 1);
    quit(value);
}
void SettingsDialog::askRestart(QWidget* parent, QString message)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(parent, "Restart?", message,
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
        quit(true);
}
void SettingsDialog::quit(bool restart)
{
    QApplication::quit();
    if(restart)
        QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

void SettingsDialog::restart()
{
    QApplication::quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}
