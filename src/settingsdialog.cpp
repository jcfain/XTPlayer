#include "settingsdialog.h"

#include "lib/handler/settingshandler.h"
#include "lib/handler/serialhandler.h"
#include "lib/handler/funscripthandler.h"
#include "lib/tool/qsettings_json.h"

#include "addchanneldialog.h"
#include "channeltablecomboboxdelegate.h"
#include "xtpsettings.h"
#include "gettextdialog.h"
#include "tagManager.h"

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent), _inputMapWidget(0)
{
    ui.setupUi(this);
    setModal(false);
    connect(ui.buttonBox, &QDialogButtonBox::clicked, this, &SettingsDialog::on_dialogButtonboxClicked);
    connect(this, &SettingsDialog::loadingDialogClose, this, &SettingsDialog::on_close_loading_dialog);
}

SettingsDialog::~SettingsDialog()
{
}

// TODO: Combine the following two functions...
void SettingsDialog::on_settingsChange(bool dirty)
{
    saveAllBtn->setEnabled(dirty);
    closeBtn->setEnabled(dirty);
    saveBtn->setEnabled(dirty);
}

void SettingsDialog::set_requires_restart(bool enabled) {
    _requiresRestart = enabled;
    if(_requiresRestart) {
        ui.restartRequiredLabel->show();
    }
}

void SettingsDialog::dispose()
{
    _connectionHandler->dispose();
}

void SettingsDialog::init(MediaLibraryHandler* medialLibraryHandler, VideoHandler* videoHandler, ConnectionHandler* connectionHandler)
{
    m_medialLibraryHandler = medialLibraryHandler;
    _videoHandler = videoHandler;
    _connectionHandler = connectionHandler;

    ui.useWebSocketsCheckbox->setHidden(true);//Fast sends buffer in QWebSocket and sends late
    ui.dubugButton->hide();// Doesnt restart in debug mode.
    ui.useMediaDirectoryCheckbox->hide();//Not fully developed.

// #ifdef _WIN32
    ui.bleConnectButton->setVisible(false);
    ui.bleOutputRdo->setVisible(false);
    ui.bleStatuslbl->setVisible(false);
// #endif
    setupUi();
}

void SettingsDialog::initInputDevice()
{
//    DeviceName deviceName = SettingsHandler::getSelectedInputDevice();
//    setDeviceStatusStyle(ConnectionStatus::Disconnected, deviceName);
//    if(deviceName == DeviceName::Deo)
//    {
//        initDeoEvent();
//    }
//    else if(deviceName == DeviceName::Whirligig)
//    {
//        initWhirligigEvent();
//    }
//    else if(SettingsHandler::getEnableHttpServer() && deviceName == DeviceName::XTPWeb)
//    {
//        initXTPWebEvent();
//    }
//    if(SettingsHandler::getGamepadEnabled()) {
//        _connectionHandler->initInputDevice(DeviceName::Gamepad);
//    }
}

void SettingsDialog::initLive()
{
//    if(_videoHandler->isPlaying())
//        _hasVideoPlayed = true;
//    ui.videoRendererComboBox->setEnabled(!_hasVideoPlayed);
    ui.disableNoScriptFoundInLibrary->setChecked(SettingsHandler::getDisableNoScriptFound());
    if(!SettingsHandler::GetHashedPass().isEmpty())
        ui.passwordButton->setText("Change password");
    if(!SettingsHandler::hashedWebPass().isEmpty())
        ui.webPasswordButton->setText("Change password");
    ui.hideWelcomeDialog->setChecked(SettingsHandler::getHideWelcomeScreen());
//    auto availableAxis = SettingsHandler::getAvailableAxis();
//    foreach(auto channel, availableAxis->keys())
//    {
//        ChannelModel axis = SettingsHandler::getAxis(channel);
//        QCheckBox* invertedCheckbox = ui.FunscriptSettingsGrid->findChild<QCheckBox*>(axis.FriendlyName);
//        if(invertedCheckbox != nullptr)
//            invertedCheckbox->setChecked(SettingsHandler::getChannelInverseChecked(channel));
//    }
}

void SettingsDialog::initializeVoice(QTextToSpeech *tts)
{
    m_tts = tts;

    auto availableVoices = tts->availableVoices();
    foreach (QVoice voice, availableVoices) {
        ui.voiceCombobox->addItem(voice.name(), QVariant(availableVoices.indexOf(voice)));
    }
    QString voiceName = XTPSettings::voiceName();
    auto voice = std::find_if(availableVoices.begin(), availableVoices.end(), [voiceName](const QVoice &x) {
        return x.name() == voiceName ||  x.gender() == QVoice::Female;
    });
    if(voice != availableVoices.end()) {
        if(voiceName.isEmpty() || voice->name() != voiceName) {
            XTPSettings::setVoiceName(voice->name());
        }
        tts->setVoice(*voice);
        ui.voiceCombobox->setCurrentIndex(availableVoices.indexOf(*voice));
    }
    connect(ui.voiceCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if(m_tts) {
            auto availableVoices = m_tts->availableVoices();
            m_tts->setVoice(availableVoices[index]);
            XTPSettings::setVoiceName(availableVoices[index].name());
            m_tts->say("Testing, one two three");
        }
    });

    double volume = XTPSettings::voiceVolume();
    tts->setVolume(volume);
    ui.voiceVolumeSlider->setValue(volume * 100);
    on_voiceVolumeSlider_sliderMoved(volume * 100);
    double pitch = XTPSettings::voicePitch();
    tts->setPitch(pitch);
    ui.voicePitchSlider->setValue(pitch * 100);
    on_voicePitchSlider_sliderMoved(pitch * 100);
    double rate = XTPSettings::voiceRate();
    tts->setRate(rate);
    ui.voiceRateSlider->setValue(rate * 100);
    on_voiceRateSlider_sliderMoved(rate * 100);
}

void SettingsDialog::reject()
{
    if(_requiresRestart)
    {
        set_requires_restart(false);
        askRestart(this);
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
            save();
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
                save();
                emit loadingDialogClose();
            });
        }
    }
    if(_requiresRestart && (ui.buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole || ui.buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole))
    {
        set_requires_restart(false);
        askRestart(this->parentWidget());
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
    setDeviceStatusStyle(ConnectionStatus::Disconnected, DeviceName::HereSphere);
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
    else if (SettingsHandler::getSelectedOutputDevice() == DeviceName::BLE)
    {
        ui.bleOutputRdo->setChecked(true);
    }
    if (!_interfaceInitialized)
    {
        ui.restartRequiredLabel->hide();
        ui.restartRequiredLabel->setStyleSheet("* { color : red; }");
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
        foreach(auto version, TCodeChannelLookup::SupportedTCodeVersions.keys())
        {
            QVariant variant;
            variant.setValue(version);
            ui.tCodeVersionComboBox->addItem(TCodeChannelLookup::SupportedTCodeVersions.value(version), variant);
        }
        ui.tCodeVersionComboBox->setCurrentText(TCodeChannelLookup::getSelectedTCodeVersionName());
        connect(ui.tCodeVersionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::on_tCodeVSComboBox_currentIndexChanged);

        connect(TCodeChannelLookup::instance(), &TCodeChannelLookup::channelProfileChanged, this, &SettingsDialog::setUpTCodeChannelUI);
        connect(TCodeChannelLookup::instance(), &TCodeChannelLookup::allProfilesDeleted, this, &SettingsDialog::setUpTCodeChannelProfiles);

        channelTableViewModel = new ChannelTableViewModel(this);
        connect(TCodeChannelLookup::instance(), &TCodeChannelLookup::channelProfileChanged, this, [this]() {
            channelTableViewModel->setMap();
            set_channelProfilesComboBox_value(TCodeChannelLookup::getSelectedChannelProfile());
        });

        ui.channelTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui.channelTableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        ChannelTableComboboxDelegate* channelTypeCombobox = new ChannelTableComboboxDelegate(ui.channelTableView);
        QMap<QString, int> axisTypes;
        foreach(auto key, ChannelTypes.keys())
            axisTypes.insert(key, (int)ChannelTypes.value(key));
        channelTypeCombobox->setData(axisTypes);
        ChannelTableComboboxDelegate* channelDimensionCombobox = new ChannelTableComboboxDelegate(ui.channelTableView);
        QMap<QString, int> axisDimensions;
        foreach(auto key, ChannelDimensions.keys())
            axisDimensions.insert(key, (int)ChannelDimensions.value(key));
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


        setUpTCodeChannelProfiles();
        setUpTCodeChannelUI();

        ui.useWebSocketsCheckbox->setChecked(SettingsHandler::getSelectedNetworkDevice() == NetworkProtocol::WEBSOCKET);

        enableOrDisableDeviceConnectionUI(SettingsHandler::getSelectedOutputDevice());
        bool deoEnabled = SettingsHandler::getSelectedInputDevice() == DeviceName::HereSphere;
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
        ui.videoIncrementSpinBox->setValue(SettingsHandler::getVideoIncrement());
        ui.voiceGroupBox->setChecked(!SettingsHandler::getDisableSpeechToText());
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
        ui.webServerWarningLabel->setVisible(SettingsHandler::getEnableHttpServer());
        ui.webAddressInstructionsLabel->setVisible(SettingsHandler::getEnableHttpServer());
        ui.webAddressLinkLabel->setVisible(SettingsHandler::getEnableHttpServer());
        ui.httpRootLineEdit->setText(SettingsHandler::getHttpServerRoot());
        ui.vrLibraryLineEdit->setText(SettingsHandler::getLastSelectedVRLibrary());
        ui.chunkSizeDoubleSpinBox->setValue(SettingsHandler::getHTTPChunkSizeMB());
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

        ui.showVRInLibraryViewCheckbox->setChecked(SettingsHandler::getShowVRInLibraryView());

        ui.webAddressLinkLabel->setProperty("cssClass", "linkLabel");
        ui.webAddressInstructionsLabel->setProperty("cssClass", "linkLabel");

        ui.disableTimeLinePreviewChk->setChecked(XTPSettings::getDisableTimeLinePreview());
        ui.hideMediaWithoutFunscriptsCheckbox->setChecked(XTPSettings::getHideMediaWithoutFunscripts());

        ui.disableFunscriptHeatmapheckBox->setChecked(XTPSettings::getHeatmapDisabled());
        ui.disableHeartbeatChk->setChecked(SettingsHandler::getDisableHeartBeat());
        connect(ui.disableHeartbeatChk, &QCheckBox::clicked, this, &SettingsDialog::onDisableHeartbeatChkClicked);

        int percentage = SettingsHandler::getViewedThreshold()*100;
        ui.viewedPercentageSpinBox->setValue(percentage);
        ui.viewedPercentageSpinBox->setSuffix("%");

        updateIPAddress();

        ui.rememberWindowSettingsChk->setChecked(XTPSettings::getRememberWindowsSettings());

        ui.schedulerGroupbox->setChecked(SettingsHandler::scheduleLibraryLoadEnabled());
        ui.scheduleLibraryLoadAtTime->setTime(SettingsHandler::scheduleLibraryLoadTime());
        ui.fullMetadataProcessChk->setChecked(SettingsHandler::scheduleLibraryLoadFullProcess());
        ui.syncSettingsChk->setChecked(SettingsHandler::scheduleSettingsSync());

        ui.playbackRateSpinBox->setValue(SettingsHandler::getPlaybackRateStep());
        connect(ui.playbackRateSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SettingsDialog::on_playbackRateSpinBoxValueChanged);

        connect(ui.schedulerGroupbox, &QGroupBox::clicked, this, &SettingsDialog::schedulerEnabledChk_clicked);
        connect(ui.fullMetadataProcessChk, &QCheckBox::clicked, this, &SettingsDialog::fullMetadataProcessChk_clicked);
        connect(ui.syncSettingsChk, &QCheckBox::clicked, this, &SettingsDialog::fullMetadataProcessChk_clicked);

        connect(ui.randomMotionGroupbox, &QGroupBox::clicked, this, &SettingsDialog::on_enableMultiplierCheckbox_clicked);

        ui.processMetadataOnStartChk->setChecked(SettingsHandler::processMetadataOnStart());
        connect(ui.processMetadataOnStartChk, &QCheckBox::clicked, this, &SettingsDialog::processMetadataOnStart_clicked);

        connect(SettingsHandler::instance(), &SettingsHandler::restartRequired, this, &SettingsDialog::set_requires_restart);
        connect(SettingsHandler::instance(), &SettingsHandler::settingsChanged, this, &SettingsDialog::on_settingsChange);
    }
}

void SettingsDialog::updateIPAddress() {
    if(SettingsHandler::getEnableHttpServer()) {
        const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
//        for (const QNetworkInterface &interface: QNetworkInterface::allInterfaces()) {
//            if(interface.type() == QNetworkInterface::Ethernet) {
//                for (const QNetworkAddressEntry &networkAddress: interface.addressEntries()) {
//                    auto address = networkAddress.ip();
//                    if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
//                         ui.webAddressLinkLabel->setText("<a href='"+address.toString() + ":"+SettingsHandler::getHTTPPort()+"'</a>");
//                }
//             }
//        }
        ui.webAddressLinkLabel->clear();
        int port = SettingsHandler::getHTTPPort();
        auto portText = port == 80 ? "" : ":" + QString::number(port);
        ui.webAddressInstructionsLabel->setText("Choose from the list (click to test or right click and copy link)<br>to enter into your devices browser. You can also test:<br><a href='http://localhost" + portText + "/'><span>http://localhost" + portText + "/</span></a> ONLY on the local machine.");
        int found = 0;
        auto allAddresses = QNetworkInterface::allAddresses();
        for (const QHostAddress &address: allAddresses) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost) {
                found++;
                 ui.webAddressLinkLabel->setText(ui.webAddressLinkLabel->text() + "<br><a href='http://" + address.toString() + portText + "/'><span>http://" + address.toString() + portText + "/</span></a><br>");
            }
        }
        if(found == 0) {
            ui.webAddressLinkLabel->setText("No addresses found.");
        }
        ui.webAddressLinkLabel->show();
        ui.webAddressLinkLabel->show();
        ui.webAddressInstructionsLabel->show();
    } else {
        ui.webAddressLinkLabel->hide();
        ui.webAddressLinkLabel->hide();
         ui.webAddressInstructionsLabel->hide();
    }
}

void SettingsDialog::setupGamepadMap()
{
    if(_inputMapWidget)
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
    ui.gamePadMapGridLayout->addWidget(_inputMapWidget, 0, 0, 1, 11);
    QLabel* instructionsLabel = new QLabel(_inputMapWidget);
    instructionsLabel->setText("Click a cell in either Gamepad or Key column for an action to assign an input.");
    ui.gamePadMapGridLayout->addWidget(instructionsLabel, 1, 0, 1, 6, Qt::AlignLeft);
    instructionsLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    QLabel* speedLabel = new QLabel(_inputMapWidget);
    speedLabel->setText("Default speed");
    speedLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    QSpinBox* speedInput = new QSpinBox(_inputMapWidget);
    speedInput->setMinimum(1);
    speedInput->setMaximum(std::numeric_limits<int>::max());
    speedInput->setMinimumWidth(75);
    speedInput->setSuffix("ms");
    speedInput->setSingleStep(100);
    speedInput->setValue(SettingsHandler::getGamepadSpeed());
    speedInput->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    connect(speedInput, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::on_speedInput_valueChanged);
    ui.gamePadMapGridLayout->addWidget(speedLabel, 1, 7, Qt::AlignRight);
    ui.gamePadMapGridLayout->addWidget(speedInput, 1, 8, Qt::AlignLeft);
    QLabel* speedIncrementLabel = new QLabel(_inputMapWidget);
    speedIncrementLabel->setText("Speed change step");
    speedIncrementLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    QSpinBox* speedIncrmentInput = new QSpinBox(_inputMapWidget);
    speedIncrmentInput->setMinimum(1);
    speedIncrmentInput->setMaximum(std::numeric_limits<int>::max());
    speedIncrmentInput->setMinimumWidth(75);
    speedIncrmentInput->setSingleStep(100);
    speedIncrmentInput->setValue(SettingsHandler::getGamepadSpeedIncrement());
    speedIncrmentInput->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    connect(speedIncrmentInput, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::on_speedIncrementInput_valueChanged);
    ui.gamePadMapGridLayout->addWidget(speedIncrementLabel, 1, 9, Qt::AlignRight);
    ui.gamePadMapGridLayout->addWidget(speedIncrmentInput, 1, 10, Qt::AlignLeft);
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

void SettingsDialog::setUpTCodeChannelProfiles() {
    disconnect(ui.channelProfilesComboBox, &QComboBox::currentTextChanged, this, &SettingsDialog::on_channelProfilesComboBox_textChange);
    ui.channelProfilesComboBox->clear();
    auto profiles = TCodeChannelLookup::getChannelProfiles();
    foreach(auto profileName, profiles)
    {
        QVariant variant;
        variant.setValue(TCodeChannelLookup::getChannels(profileName));
        ui.channelProfilesComboBox->addItem(profileName, variant);
    }
    ui.channelProfilesComboBox->setCurrentText(TCodeChannelLookup::getSelectedChannelProfile());
    connect(ui.channelProfilesComboBox, &QComboBox::currentTextChanged, this, &SettingsDialog::on_channelProfilesComboBox_textChange);
}

void SettingsDialog::setUpTCodeChannelUI()
{
    if(ui.rangeLimitGroupbox->layout())
    {
        QGridLayout* rangeLimitLayout = (QGridLayout*)ui.rangeLimitGroupbox->layout();
        QLayoutItem *child;
        while ((child = rangeLimitLayout->takeAt(0)) != 0)
        {
            //setParent is NULL, preventing the interface from disappearing after deletion.
            if(child->widget())
            {
                child->widget()->setParent(NULL);
                QWidget* widget = child->widget();
                delete widget;
            }
            delete child;
        }
        delete rangeLimitLayout;
    }
    if(ui.randomMotionGroupbox->layout())
    {
        QGridLayout* randomMotionlayout = (QGridLayout*)ui.randomMotionGroupbox->layout();
        QLayoutItem *child;
        while ((child = randomMotionlayout->takeAt(0)) != 0)
        {
            if(child->widget())
            {
                child->widget()->setParent(NULL);
                QWidget* widget = child->widget();
                delete widget;
            }
            delete child;
        }
        delete randomMotionlayout;
    }
    if(ui.inversionGroupBox->layout())
    {
        QGridLayout* inversionlayout = (QGridLayout*)ui.inversionGroupBox->layout();
        QLayoutItem *child;
        while ((child = inversionlayout->takeAt(0)) != 0)
        {
            if(child->widget())
            {
                child->widget()->setParent(NULL);
                QWidget* widget = child->widget();
                delete widget;
            }
            delete child;
        }
        delete inversionlayout;
    }

    QGridLayout* rangeGrid = new QGridLayout();
    rangeGrid->setSizeConstraint(QLayout::SetMinimumSize);
    // rangeGrid->setHorizontalSpacing(2);
    // rangeGrid->setVerticalSpacing(0);
    rangeGrid->setSpacing(5);
    QGridLayout* randomGrid = new QGridLayout();
    randomGrid->setSizeConstraint(QLayout::SetMinimumSize);
    QGridLayout* inversionGrid = new QGridLayout();
    inversionGrid->setSizeConstraint(QLayout::SetMinimumSize);
    ui.rangeLimitGroupbox->setLayout(rangeGrid);
    ui.randomMotionGroupbox->setLayout(randomGrid);
    ui.inversionGroupBox->setLayout(inversionGrid);
    QFont font( "Sans Serif", 8);
    int sliderGridRow = 0;
    int randomMotionGridRow = 0;
    int inversionGridRow = 0;
    auto tcodeChannels = TCodeChannelLookup::GetSelectedVersionMap();
    ui.randomMotionGroupbox->setChecked(SettingsHandler::getMultiplierEnabled());
    //QCheckBox* enableCheckbox = new QCheckBox("Enable", this);
    //enableCheckbox->setChecked(SettingsHandler::getMultiplierEnabled());
    //connect(enableCheckbox, &QCheckBox::clicked, this, &SettingsDialog::on_enableMultiplierCheckbox_clicked);
    //randomGrid->addWidget(enableCheckbox, multiplierGridRow, 0, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
    //multiplierGridRow++;

    rangeLabels.clear();
    rangeMinLabels.clear();
    rangeMaxLabels.clear();
    rangeSliders.clear();
    _multiplierWidgets.clear();
    foreach(auto channel, tcodeChannels.keys())
    {
        QString channelName = TCodeChannelLookup::ToString(channel);
        auto axis = TCodeChannelLookup::getChannel(channelName);
        if(axis == nullptr || axis->Type == ChannelType::None || axis->Type == ChannelType::HalfOscillate)
            continue;

        int userMin = axis->UserMin;
        int userMid = axis->UserMid;
        int userMax = axis->UserMax;
        QLabel* rangeMinLabel = new QLabel(QString::number(userMin), ui.rangeLimitGroupbox);
        rangeMinLabel->setObjectName(axis->AxisName+"RangeMinLabel");
        rangeMinLabel->setFont(font);
        rangeMinLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        rangeGrid->addWidget(rangeMinLabel, sliderGridRow, 0);
        rangeMinLabels.insert(channelName, rangeMinLabel);

        QLabel* rangeLabel = new QLabel(axis->FriendlyName + " mid: " + QString::number(userMid), ui.rangeLimitGroupbox);
        rangeLabel->setObjectName(axis->AxisName+"RangeLabel");
        rangeLabel->setFont(font);
        rangeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        rangeGrid->addWidget(rangeLabel, sliderGridRow, 1, 1, 2, Qt::AlignHCenter | Qt::AlignVCenter);
        rangeLabels.insert(channelName, rangeLabel);

        QLabel* rangeMaxLabel = new QLabel(QString::number(userMax), ui.rangeLimitGroupbox);
        rangeMaxLabel->setObjectName(axis->AxisName+"RangeMaxLabel");
        rangeMaxLabel->setFont(font);
        rangeMaxLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        rangeGrid->addWidget(rangeMaxLabel, sliderGridRow, 3);
        rangeMaxLabels.insert(channelName, rangeMaxLabel);

        RangeSlider* axisRangeSlider = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, ui.rangeLimitGroupbox);
        axisRangeSlider->setObjectName(axis->AxisName+"RangeSlider");
        axisRangeSlider->SetRange(axis->Min, axis->Max);
        axisRangeSlider->setLowerValue(userMin);
        axisRangeSlider->setUpperValue(userMax);
        axisRangeSlider->SetMinimumRange(1);
        axisRangeSlider->setName(channelName);// Required
        sliderGridRow++;
        rangeGrid->addWidget(axisRangeSlider, sliderGridRow,0,1,4);
        rangeSliders.insert(channelName, axisRangeSlider);
        sliderGridRow++;

        QProgressBar* funscriptProgressbar = new QProgressBar(ui.rangeLimitGroupbox);
        funscriptProgressbar->setObjectName(axis->AxisName+"FunscriptStatus");
        funscriptProgressbar->setMinimum(0);
        funscriptProgressbar->setMaximum(100);
        funscriptProgressbar->setMaximumHeight(5);
        rangeGrid->addWidget(funscriptProgressbar, sliderGridRow,0,1,4);
        axisProgressbars.insert(channelName, funscriptProgressbar);
        sliderGridRow++;

        connect(this, &SettingsDialog::onAxisValueChange, this, &SettingsDialog::on_axis_valueChange);
        connect(this, &SettingsDialog::onAxisValueReset, this, &SettingsDialog::on_axis_valueReset);
        connect(axisRangeSlider, QOverload<QString, int>::of(&RangeSlider::lowerValueChanged), this, &SettingsDialog::onRange_valueChanged);
        connect(axisRangeSlider, QOverload<QString, int>::of(&RangeSlider::upperValueChanged), this, &SettingsDialog::onRange_valueChanged);
        // mouse release work around for gamepad recalculation reseting on every valueChange event.
        connect(axisRangeSlider, QOverload<QString>::of(&RangeSlider::mouseRelease), this, &SettingsDialog::onRange_mouseRelease);

        QCheckBox* multiplierCheckbox = new QCheckBox(ui.randomMotionGroupbox);
        multiplierCheckbox->setText(axis->FriendlyName);
        multiplierCheckbox->setChecked(SettingsHandler::getMultiplierChecked(channelName));
        connect(multiplierCheckbox, &QCheckBox::clicked, this,
                 [this, channelName](bool checked)
                   {
                     SettingsHandler::setMultiplierChecked(channelName, checked);
                     if(!checked)
                         emit TCodeHomeClicked();
                   });
        QCheckBox* damperCheckbox = new QCheckBox(ui.randomMotionGroupbox);
        damperCheckbox->setText("Speed");
        damperCheckbox->setChecked(SettingsHandler::getDamperChecked(channelName));
        QDoubleSpinBox* damperInput = new QDoubleSpinBox(ui.randomMotionGroupbox);
        damperInput->setToolTip("Multiply the speed by the value.\n4000 * 0.5 = 2000");
        damperInput->setDecimals(1);
        damperInput->setSingleStep(0.1f);
        damperInput->setMinimum(0.1f);
        damperInput->setMaximum(std::numeric_limits<int>::max());
        damperInput->setValue(SettingsHandler::getDamperValue(channelName));
        connect(damperInput, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                 [channelName](float value)
                   {
                     SettingsHandler::setDamperValue(channelName, value);
                   });
        connect(damperCheckbox, &QCheckBox::clicked, this,
                 [channelName](bool checked)
                   {
                     SettingsHandler::setDamperChecked(channelName, checked);
                   });

        QCheckBox* linkCheckbox = new QCheckBox(ui.randomMotionGroupbox);
        auto relatedChannel = TCodeChannelLookup::getChannel(axis->RelatedChannel);
        linkCheckbox->setToolTip("This will link the channel to the related script.\nThis will remove the random calculation and just link\nthe current MFS " + relatedChannel->FriendlyName + " funscript value.\nIf there is no " + relatedChannel->FriendlyName + " funscript then it will default to random motion.");
        linkCheckbox->setText("Link to script: ");
        linkCheckbox->setChecked(SettingsHandler::getLinkToRelatedAxisChecked(channelName));
        connect(linkCheckbox, &QCheckBox::clicked, this,
                 [channelName](bool checked)
                   {
                     SettingsHandler::setLinkToRelatedAxisChecked(channelName, checked);
                   });

        QComboBox* linkToAxisCombobox = new QComboBox(ui.randomMotionGroupbox);
        foreach(auto axis, tcodeChannels.keys())
        {
            auto channel =  TCodeChannelLookup::getChannel(TCodeChannelLookup::ToString(axis));
            if(channel == nullptr || channel->AxisName == channelName || channel->Type == ChannelType::HalfOscillate)
                 continue;
            QVariant variant;
            variant.setValue(*channel);
            linkToAxisCombobox->addItem(channel->FriendlyName, variant);
        }
        linkToAxisCombobox->setCurrentText(relatedChannel->FriendlyName);
        connect(linkToAxisCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                 [channelName, linkToAxisCombobox, linkCheckbox](int value)
                   {
                        auto relatedChannel = linkToAxisCombobox->currentData().value<ChannelModel33>();
                        linkCheckbox->setToolTip("This will link the channel to the related axis.\nThis will remove the random calculation and just link\nthe current MFS (Multi-funscript) " + relatedChannel.FriendlyName + " funscript value.\nIf there is no " + relatedChannel.FriendlyName + " funscript then it will default to random motion.");
                        SettingsHandler::setLinkToRelatedAxis(channelName, relatedChannel.AxisName);
                   });


         randomGrid->addWidget(multiplierCheckbox, randomMotionGridRow, 0, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
         randomGrid->addWidget(linkCheckbox, randomMotionGridRow, 1, 1, 1, Qt::AlignRight | Qt::AlignVCenter);
         randomGrid->addWidget(linkToAxisCombobox, randomMotionGridRow, 2, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);
         randomGrid->addWidget(damperCheckbox, randomMotionGridRow, 3, 1, 1, Qt::AlignRight | Qt::AlignVCenter);
         randomGrid->addWidget(damperInput, randomMotionGridRow, 4, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

        _multiplierWidgets.append(multiplierCheckbox);
        _multiplierWidgets.append(linkCheckbox);
        _multiplierWidgets.append(linkToAxisCombobox);
        _multiplierWidgets.append(damperCheckbox);
        _multiplierWidgets.append(damperInput);

         randomMotionGridRow++;

        QCheckBox* invertedCheckbox = new QCheckBox(ui.inversionGroupBox);
        invertedCheckbox->setText(axis->FriendlyName);
        invertedCheckbox->setChecked(SettingsHandler::getChannelFunscriptInverseChecked(channelName));
        connect(invertedCheckbox, &QCheckBox::clicked, this,
             [channelName](bool checked)
               {
                 SettingsHandler::setChannelFunscriptInverseChecked(channelName, checked);
               });
        inversionGrid->addWidget(invertedCheckbox, inversionGridRow, 0, 1, 1, Qt::AlignLeft | Qt::AlignVCenter);

        inversionGridRow++;
    }

    //setUpMultiplierUi(SettingsHandler::getMultiplierEnabled());

    QPushButton* zeroOutButton = new QPushButton(ui.rangeLimitGroupbox);
    zeroOutButton->setText("Send device home");
    connect(zeroOutButton, & QPushButton::clicked, this, &SettingsDialog::on_tCodeHome_clicked);
    rangeGrid->addWidget(zeroOutButton, sliderGridRow + 1, 0);

    QLabel* xRangeStepLabel = new QLabel(ui.rangeLimitGroupbox);
    xRangeStepLabel->setText("Stroke range change step");
    xRangeStepLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QSpinBox* xRangeStepInput = new QSpinBox(ui.rangeLimitGroupbox);
    xRangeStepInput->setToolTip("The amount to modify the stroke range when using keyboard/gamepad.");
    xRangeStepInput->setMinimum(1);
    xRangeStepInput->setMaximum(INT_MAX);
    xRangeStepInput->setMinimumWidth(75);
    xRangeStepInput->setSingleStep(50);
    xRangeStepInput->setValue(SettingsHandler::getGamepadSpeedIncrement());
    xRangeStepInput->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    connect(xRangeStepInput, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsDialog::xRangeStepInput_valueChanged);
    rangeGrid->addWidget(xRangeStepLabel, sliderGridRow + 1, 2);
    rangeGrid->addWidget(xRangeStepInput, sliderGridRow + 1, 3);

    if(ui.otherMotionGridLayout->isEmpty())
    {
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
        auto max = TCodeChannelLookup::getTCodeMaxValue();
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
        ui.otherMotionGridLayout->addWidget(lubePulseCheckbox, 0, 0);
        ui.otherMotionGridLayout->addWidget(lubePulseAmountLabel, 1, 0);
        lubePulseCheckbox->raise();
        lubePulseCheckbox->setStyleSheet("* {background: transparent}");

        QLabel* customTCodeLabel = new QLabel(this);
        customTCodeLabel->setText("Custom TCode");
        QListWidget* customTCodeListWidget = new QListWidget(this);
        customTCodeListWidget->setObjectName(tr("customTCodeCommandList"));
        customTCodeListWidget->setMinimumSize(100, 150);
        customTCodeListWidget->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
        customTCodeListWidget->addItems(SettingsHandler::getCustomTCodeCommands());
        connect(customTCodeListWidget, & QListWidget::doubleClicked, this, [this, customTCodeListWidget](const QModelIndex index) {
            if(customTCodeListWidget->selectedItems().length()) {
                bool ok;
                QString selected = customTCodeListWidget->selectedItems().first()->text();
                auto newValue = GetTextDialog::show(this, "Custom TCode", selected, &ok);
                if(ok) {
                    MediaActions actions;
                    if(actions.Values.contains(newValue)) {
                        DialogHandler::MessageBox(this, "Reserved value: "+newValue, XLogLevel::Critical);
                    } else if(customTCodeListWidget->findItems(newValue, Qt::MatchExactly).isEmpty()) {
                        SettingsHandler::editCustomTCodeCommand(selected, newValue);
                        customTCodeListWidget->clear();
                        customTCodeListWidget->addItems(SettingsHandler::getCustomTCodeCommands());
                        set_requires_restart(true);
                    } else {
                        DialogHandler::MessageBox(this, "Duplicate value: "+newValue, XLogLevel::Critical);
                    }
                }
            }
        });

        QPushButton* customTCodeAddbutton = new QPushButton(this);
        customTCodeAddbutton->setText("Add");
        connect(customTCodeAddbutton, &QPushButton::clicked, this, [this, customTCodeListWidget]() {
            bool ok;
            auto value = GetTextDialog::show(this, "Custom TCode", nullptr, &ok);
            if(ok) {
                MediaActions actions;
                if(actions.Values.contains(value)) {
                    DialogHandler::MessageBox(this, "Reserved value: "+value, XLogLevel::Critical);
                } else if(customTCodeListWidget->findItems(value, Qt::MatchExactly).isEmpty()) {
                    SettingsHandler::addCustomTCodeCommand(value);
                    customTCodeListWidget->clear();
                    customTCodeListWidget->addItems(SettingsHandler::getCustomTCodeCommands());
                    MediaActions::AddOtherAction(value, "TCode command: " + value, ActionType::TCODE);
                    set_requires_restart(true);
                } else {
                    DialogHandler::MessageBox(this, "Duplicate value: "+value, XLogLevel::Critical);
                }
            }
        });

        QPushButton* customTCodeRemovebutton = new QPushButton(this);
        customTCodeRemovebutton->setEnabled(false);
        customTCodeRemovebutton->setText("Remove");
        connect(customTCodeRemovebutton, &QPushButton::clicked, this, [this, customTCodeListWidget, customTCodeRemovebutton]() {
            auto amount = customTCodeListWidget->selectedItems().length();
            if(amount) {
                auto ok = DialogHandler::Dialog(this, "Remove selected " + QString::number(amount) + " item(s)?");
                if(ok) {
                    for(auto selected: customTCodeListWidget->selectedItems()) {
                        SettingsHandler::removeCustomTCodeCommand(selected->text());
                    }
                    customTCodeListWidget->clear();
                    customTCodeListWidget->addItems(SettingsHandler::getCustomTCodeCommands());
                    customTCodeRemovebutton->setEnabled(false);
                    set_requires_restart(true);
                }
            }
        });

        connect(customTCodeListWidget, & QListWidget::itemClicked, this, [customTCodeListWidget, customTCodeRemovebutton](QListWidgetItem* item) {
         customTCodeRemovebutton->setEnabled(customTCodeListWidget->selectedItems().length());
        });

        ui.otherMotionGridLayout->addWidget(libePulseAmountInput, 1, 1);
        ui.otherMotionGridLayout->addWidget(lubePulseFrequencyLabel, 2, 0);
        ui.otherMotionGridLayout->addWidget(libePulseFrequencyInput, 2, 1);
        ui.otherMotionGridLayout->addWidget(customTCodeLabel, 3, 0);
        ui.otherMotionGridLayout->addWidget(customTCodeListWidget, 4, 0, 1, 2);
        ui.otherMotionGridLayout->addWidget(customTCodeAddbutton, 5, 0);
        ui.otherMotionGridLayout->addWidget(customTCodeRemovebutton, 5, 1);
    }

    setupGamepadMap();
}

void SettingsDialog::setAxisProgressBar(QString axis, int value, int time, ChannelTimeType timeType)
{
    emit onAxisValueChange(axis, value, time, timeType);
}

void SettingsDialog::on_axis_valueChange(QString axis, int value, int time, ChannelTimeType timeType)
{
    if (ui.settingsTabWidget->currentWidget() == ui.motionTab)
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

void SettingsDialog::on_xtpWeb_initInputDevice(DeviceName deviceName, bool checked)
{
    if(deviceName == DeviceName::HereSphere) {
        ui.deoAddressTxt->setText(SettingsHandler::getDeoAddress());
        ui.deoPortTxt->setText(SettingsHandler::getDeoPort());
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
}

void SettingsDialog::on_xtpWeb_initOutputDevice(DeviceName deviceName, bool checked)
{
    if(deviceName == DeviceName::Serial) {
        ui.SerialOutputCmb->setCurrentText(SettingsHandler::getSerialPort());
        ui.serialOutputRdo->setChecked(checked);
        on_serialOutputRdo_clicked();
        on_serialConnectButton_clicked();
    } else if(deviceName == DeviceName::Network) {
        ui.networkAddressTxt->setText(SettingsHandler::getServerAddress());
        ui.networkPortTxt->setText(SettingsHandler::getServerPort());
        ui.networkOutputRdo->setChecked(checked);
        on_networkOutputRdo_clicked();
        ui.networkConnectButton->setEnabled(false);
    } else if(deviceName == DeviceName::BLE) {
        ui.bleOutputRdo->setChecked(checked);
        on_bleConnectButton_clicked();
        ui.bleConnectButton->setEnabled(false);
    } else if(deviceName == DeviceName::None) {
        ui.serialOutputRdo->setChecked(false);
        ui.networkOutputRdo->setChecked(false);
        ui.bleOutputRdo->setChecked(false);
    }
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
    if(deviceName == DeviceName::Serial || deviceName == DeviceName::Network || deviceName == DeviceName::BLE )
    {
        ui.serialStatuslbl->clear();
        ui.networkStatuslbl->clear();
        ui.bleStatuslbl->clear();
    }
    else if(deviceName == DeviceName::HereSphere ||
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
    else if (deviceName == DeviceName::BLE)
    {
        ui.bleStatuslbl->setText(statusUnicode + " " + message);
        ui.bleStatuslbl->setFont(font);
        ui.bleStatuslbl->setStyleSheet("color: " + statusColor);
    }
    else if (deviceName == DeviceName::HereSphere)
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
    SettingsHandler::setSelectedOutputDevice(DeviceName::Serial);
}

void SettingsDialog::on_networkOutputRdo_clicked()
{
    enableOrDisableDeviceConnectionUI(DeviceName::Network);
    SettingsHandler::setSelectedOutputDevice(DeviceName::Network);
}

void SettingsDialog::on_bleOutputRdo_clicked()
{
    DialogHandler::Dialog(this, "BLE is experimental and I could not get it to work.\nIf you do let me know. Maybe it will help me trouble shoot.\nIn otherwords, dont expect much.");
    enableOrDisableDeviceConnectionUI(DeviceName::BLE);
    SettingsHandler::setSelectedOutputDevice(DeviceName::BLE);
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
        ui.useWebSocketsCheckbox->setEnabled(true);
        ui.bleConnectButton->setEnabled(false);
    }
    else if(deviceName == DeviceName::Serial)
    {
        ui.SerialOutputCmb->setEnabled(true);
        ui.networkAddressTxt->setEnabled(false);
        ui.networkPortTxt->setEnabled(false);
        ui.networkConnectButton->setEnabled(false);
        ui.serialConnectButton->setEnabled(true);
        ui.serialRefreshBtn->setEnabled(true);
        ui.useWebSocketsCheckbox->setEnabled(false);
        ui.bleConnectButton->setEnabled(false);
    }
    else if(deviceName == DeviceName::BLE)
    {
        ui.bleConnectButton->setEnabled(true);
        ui.SerialOutputCmb->setEnabled(false);
        ui.networkAddressTxt->setEnabled(false);
        ui.networkPortTxt->setEnabled(false);
        ui.networkConnectButton->setEnabled(false);
        ui.serialConnectButton->setEnabled(false);
        ui.serialRefreshBtn->setEnabled(false);
        ui.useWebSocketsCheckbox->setEnabled(false);
    }
}
void SettingsDialog::on_serialRefreshBtn_clicked()
{
    loadSerialPorts();
}

void SettingsDialog::onRange_valueChanged(QString name, int value)
{
    RangeSlider* slider = rangeSliders.value(name);
    auto channel = TCodeChannelLookup::getChannel(name);
    auto mainLabel = rangeLabels.value(name);
    int max = slider->GetUpperValue();
    int min = slider->GetLowerValue();
    rangeMinLabels.value(name)->setText(QString::number(min));
    rangeMaxLabels.value(name)->setText(QString::number(max));
    mainLabel->setText(channel->FriendlyName + " mid: " + QString::number(XMath::middle(min, max)));
    OutputDeviceHandler* outputDevice = _connectionHandler->getSelectedOutputDevice();
    InputDeviceHandler* inputDevice = _connectionHandler->getSelectedInputDevice();
    if ((!_videoHandler->isPlaying() || _videoHandler->isPaused())
        && (!inputDevice || !inputDevice->isPlaying()) && (outputDevice && outputDevice->isConnected()))
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

    if(event.deviceName == DeviceName::Whirligig || event.deviceName == DeviceName::HereSphere)
    {
        ui.whirligigConnectButton->setEnabled(event.deviceName == DeviceName::Whirligig && connectingOrConnected);
        ui.deoConnectButton->setEnabled(event.deviceName == DeviceName::HereSphere && connectingOrConnected);
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
    else if (event.deviceName == DeviceName::BLE)
    {
        ui.bleConnectButton->setEnabled(event.status == ConnectionStatus::Error || event.status == ConnectionStatus::Disconnected);
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
    if(SettingsHandler::getSelectedInputDevice() == DeviceName::HereSphere)
        ui.deoConnectButton->setEnabled(true);
    SettingsHandler::setDeoAddress(ui.deoAddressTxt->text());
}

void SettingsDialog::on_useWebSocketsCheckbox_clicked(bool checked)
{
    if(SettingsHandler::getSelectedOutputDevice() == DeviceName::Network)
        ui.networkConnectButton->setEnabled(true);
    _connectionHandler->disposeOutputDevice(DeviceName::Network);
    SettingsHandler::setSelectedNetworkDevice(checked ? NetworkProtocol::WEBSOCKET : NetworkProtocol::UDP);
}

void SettingsDialog::on_deoPortTxt_editingFinished()
{
    if(SettingsHandler::getSelectedInputDevice() == DeviceName::HereSphere)
        ui.deoConnectButton->setEnabled(true);
    SettingsHandler::setDeoPort(ui.deoPortTxt->text());
}

void SettingsDialog::on_enableMultiplierCheckbox_clicked(bool checked)
{
    SettingsHandler::setMultiplierEnabled(checked);
    //setUpMultiplierUi(checked);
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
    ui.serialConnectButton->setEnabled(false);
    _connectionHandler->initOutputDevice(DeviceName::Serial);
}

void SettingsDialog::on_networkConnectButton_clicked()
{
    if(SettingsHandler::getServerAddress() != "" && SettingsHandler::getServerPort() != "" &&
     SettingsHandler::getServerAddress() != "0" && SettingsHandler::getServerPort() != "0")
    {
        ui.networkConnectButton->setEnabled(false);
        _connectionHandler->initOutputDevice(DeviceName::Network);
    }
    else
    {
        DialogHandler::MessageBox(this, "Invalid network address!", XLogLevel::Critical);
    }
}

void SettingsDialog::on_bleConnectButton_clicked()
{
    ui.networkConnectButton->setEnabled(false);
    _connectionHandler->initOutputDevice(DeviceName::BLE);
}

void SettingsDialog::on_deoConnectButton_clicked()
{
    if(SettingsHandler::getDeoAddress() != "" && SettingsHandler::getDeoPort() != "" &&
     SettingsHandler::getDeoAddress() != "0" && SettingsHandler::getDeoPort() != "0")
    {
        ui.deoConnectButton->setEnabled(false);
        _connectionHandler->initInputDevice(DeviceName::HereSphere);
    }
    else
    {
        DialogHandler::MessageBox(this, "Invalid heresphere address!", XLogLevel::Warning);
    }
}

void SettingsDialog::on_whirligigConnectButton_clicked()
{
    if(SettingsHandler::getWhirligigAddress() != "" && SettingsHandler::getWhirligigPort() != "" &&
     SettingsHandler::getWhirligigAddress() != "0" && SettingsHandler::getWhirligigPort() != "0")
    {
        ui.whirligigConnectButton->setEnabled(false);
        _connectionHandler->initInputDevice(DeviceName::Whirligig);
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
        _connectionHandler->initInputDevice(DeviceName::XTPWeb);
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
            SettingsHandler::Restart();
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
    if (checked)
        _connectionHandler->initInputDevice(DeviceName::Gamepad);
    else
        _connectionHandler->disposeInputDevice(DeviceName::Gamepad);
}

void SettingsDialog::on_videoIncrement_valueChanged(int value)
{
    SettingsHandler::setVideoIncrement(value);
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
        //channelTableViewModel->setMap();
        //setUpTCodeAxis();
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
            QStringList cannotBedeleted;
            foreach(auto row, selectedRows)
            {
                const auto model = row.model();
                const auto channelKey = ((ChannelTableViewModel*)model)->getRowKey(row.row());
                if (channelKey.isEmpty())
                    continue;
                if(TCodeChannelLookup::isDefaultChannel(channelKey)) {
                    cannotBedeleted << channelKey;
                    continue;
                }

                channelsToDelete << channelKey;
            }
            foreach(auto channel, channelsToDelete)
                SettingsHandler::deleteAxis(channel);

            if(!cannotBedeleted.empty())
                DialogHandler::MessageBox(this, "The following channels are default and cannot be deleted: "+cannotBedeleted.join(", "), XLogLevel::Critical);
            //channelTableViewModel->setMap();
            //setUpTCodeAxis();
        }
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
    bool ok;
    QString hashedPass = DialogHandler::passwordSetWizard(this, SettingsHandler::GetHashedPass(), &ok);
    if(ok) {
        SettingsHandler::SetHashedPass(hashedPass);
        if(!hashedPass.isEmpty()) {
            ui.passwordButton->setText("Change password");
        } else {
            ui.passwordButton->setText("Set password");
        }
    }
}



void SettingsDialog::on_webPasswordButton_clicked()
{
    bool ok;
    QString hashedPass = DialogHandler::passwordSetWizard(this, SettingsHandler::hashedWebPass(), &ok);
    if(ok) {
        SettingsHandler::setHashedWebPass(hashedPass);
        if(!hashedPass.isEmpty()) {
            ui.webPasswordButton->setText("Change password");
        } else {
            ui.webPasswordButton->setText("Set password");
        }
    }
}

void SettingsDialog::save(QSettings *settingsToSaveTo)
{
    XTPSettings::save(settingsToSaveTo);
    SettingsHandler::Save(settingsToSaveTo);
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
       save();
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
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Warning!", "This will reset ALL CHANNEL PROFILES to default.\nContinue?",
                                  QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::Yes)
    {
        SettingsHandler::changeSelectedTCodeVersion(ui.tCodeVersionComboBox->currentData().value<TCodeVersion>());
        askHowToResetChannelProfileDefaults();
    }
    else
    {
        disconnect(ui.tCodeVersionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::on_tCodeVSComboBox_currentIndexChanged);
        ui.tCodeVersionComboBox->setCurrentText(TCodeChannelLookup::getSelectedTCodeVersionName());
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
    askRestart(this);
}

void SettingsDialog::on_close_loading_dialog()
{
    DialogHandler::LoadingClose();
}

void SettingsDialog::on_showLoneFunscriptsInLibraryCheckbox_clicked(bool checked)
{
    SettingsHandler::setHideStandAloneFunscriptsInLibrary(checked);
    emit updateLibrary();
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
    QString selectedScript = QFileDialog::getOpenFileName(this, tr("Choose script"), SettingsHandler::getLastSelectedLibrary(), tr("Scripts (*.funscript *.zip)"));
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
    ui.webServerWarningLabel->setVisible(checked);
    ui.webAddressInstructionsLabel->setVisible(checked);
    ui.webAddressLinkLabel->setVisible(checked);
    set_requires_restart(true);
    if(!checked && SettingsHandler::getSelectedInputDevice() == DeviceName::XTPWeb)
        on_xtpWeb_initInputDevice(DeviceName::None, false);
}

void SettingsDialog::on_browseHttpRootButton_clicked()
{
    QString selectedDirectory = QFileDialog::getExistingDirectory(this, tr("Choose HTTP root"), QApplication::applicationDirPath());
    if (selectedDirectory != Q_NULLPTR)
    {
        SettingsHandler::setHttpServerRoot(selectedDirectory);
        ui.httpRootLineEdit->setText(selectedDirectory);
        if(SettingsHandler::getEnableHttpServer())
            set_requires_restart(true);
    }
}

void SettingsDialog::on_browseVRLibraryButton_clicked()
{
    QString selectedDirectory = QFileDialog::getExistingDirectory(this, tr("Choose VR library"), SettingsHandler::getLastSelectedLibrary());
    on_vrLibraryLineEdit_textEdited(selectedDirectory);
}

void SettingsDialog::on_vrLibraryLineEdit_textEdited(const QString &selectedDirectory)
{
    if (!selectedDirectory.isEmpty() && QFile::exists(selectedDirectory))
    {
        QStringList messages;
        if(SettingsHandler::addSelectedVRLibrary(selectedDirectory, messages))
            ui.vrLibraryLineEdit->setText(selectedDirectory);
        else
            DialogHandler::MessageBox(this, messages.join("\n"), XLogLevel::Warning);
    } else if(!selectedDirectory.isEmpty()) {
        DialogHandler::MessageBox(this, "Invalid directory!", XLogLevel::Warning);
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
    SettingsHandler::setHTTPChunkSizeMB(value);
}

void SettingsDialog::on_httpRootLineEdit_textEdited(const QString &selectedDirectory)
{
    SettingsHandler::setHttpServerRoot(selectedDirectory);
    if(SettingsHandler::getEnableHttpServer())
        set_requires_restart(true);
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
        set_requires_restart(true);
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
        set_requires_restart(true);
}

void SettingsDialog::on_httpThumbQualitySpinBox_editingFinished()
{
    SettingsHandler::setHttpThumbQuality(ui.httpThumbQualitySpinBox->value());
}

void SettingsDialog::Export(QWidget* parent)
{
    QString ext;
    QString selectedFile = QFileDialog::getSaveFileName(parent, QApplication::applicationDirPath() + "/Save settings", "settings_export-"+XTPSettings::XTPVersion, "JSON Files (*.json);;INI Files (*.ini)", &ext);
    if(!selectedFile.isEmpty())
    {
        bool isJSON = ext == "JSON Files (*.json)";
        selectedFile += isJSON ? selectedFile.endsWith(".json") ? "" : ".json" : selectedFile.endsWith(".ini") ? "" : ".ini";
        if(!XTPSettings::exportToFile(selectedFile, isJSON ? JSONSettingsFormatter::JsonFormat : QSettings::Format::IniFormat))
            return;
        emit messageSend("Settings saved to "+ selectedFile, XLogLevel::Information);
    }
}

void SettingsDialog::Import(QWidget* parent)
{
    QString ext;
    QString selectedFile = QFileDialog::getOpenFileName(parent, "Choose settings", QApplication::applicationDirPath(), "JSON Files (*.json);;INI Files (*.ini)", &ext);
    if(!selectedFile.isEmpty())
    {
        QSettings::Format format = ext == "JSON Files (*.json)" ? JSONSettingsFormatter::JsonFormat : QSettings::Format::IniFormat;
        if(!XTPSettings::importFromFile(selectedFile, format))
            return;
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
    SettingsHandler::Quit(restart);
}

void SettingsDialog::restart()
{
    SettingsHandler::Restart();
}

void SettingsDialog::on_openDeoPDFButton_clicked()
{
    QString filePath = QApplication::applicationDirPath() + "/XTP_and_VR_guide.pdf";
    if(QFile::exists(filePath)) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    } else {
        DialogHandler::MessageBox(this, "Error opening: "+ filePath, XLogLevel::Critical);
    }
}

void SettingsDialog::on_showVRInLibraryViewCheckbox_clicked(bool checked)
{
    SettingsHandler::setShowVRInLibraryView(checked);
    emit updateLibrary();
}

void SettingsDialog::on_webAddressCopyButton_clicked()
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    //QString originalText = clipboard->text();
    clipboard->setText(ui.webAddressLinkLabel->text());
}


void SettingsDialog::on_rememberWindowSettingsChk_clicked(bool checked)
{
    XTPSettings::setRememberWindowsSettings(checked);
}


void SettingsDialog::on_dubugButton_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "WARNING!", "Restart the app in debug mode?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        QApplication::quit();
        QProcess::startDetached("XTPlayerDebug.bat", QStringList("-debug"));
    }
}

void SettingsDialog::on_disableTimeLinePreviewChk_clicked(bool checked)
{
    XTPSettings::setDisableTimeLinePreview(checked);
}

void SettingsDialog::on_channelProfilesComboBox_textChange(const QString &profile) {
    if(TCodeChannelLookup::getSelectedChannelProfile() != profile)
        TCodeChannelLookup::setSelectedChannelProfile(profile);
}

void SettingsDialog::set_channelProfilesComboBox_value(const QString &profile) {
    if(ui.channelProfilesComboBox->currentText() != profile)
        ui.channelProfilesComboBox->setCurrentText(profile);
}

void SettingsDialog::on_addChannelProfileButton_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Copy!", "Copy from current profile?",
                                  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
    if(reply != QMessageBox::Cancel) {
        bool ok;
        QString text = QInputDialog::getText(this, tr("New profile"),
                                             tr("Profile name:"), QLineEdit::Normal,
                                             "", &ok);
        if (ok && !text.isEmpty())
        {
            auto duplicate = TCodeChannelLookup::hasProfile(text);
            if(duplicate) {
                DialogHandler::MessageBox(this, "There is already a profile named: "+ text, XLogLevel::Critical);
            } else {
                if (reply == QMessageBox::Yes) {
                    TCodeChannelLookup::copyChannelsProfile(text);
                }
                else if(reply == QMessageBox::No) {
                    TCodeChannelLookup::addChannelsProfile(text);
                }
                auto profiles = TCodeChannelLookup::getChannelProfiles();
                QVariant variant;
                variant.setValue(TCodeChannelLookup::getChannels(text));
                ui.channelProfilesComboBox->addItem(text, variant);
                ui.channelProfilesComboBox->setCurrentText(text);
            }
        }
    }
}


void SettingsDialog::on_deleteProfileButton_clicked()
{
    auto lastProfile = TCodeChannelLookup::getChannelProfiles().count() == 1;
    if(lastProfile) {
        DialogHandler::MessageBox(this, "Must have at least 1 profile!", XLogLevel::Critical);
        return;
    }
    QMessageBox::StandardButton reply;
    auto selectedProfile = ui.channelProfilesComboBox->currentData().value<QMap<QString, ChannelModel33>>();
    reply = QMessageBox::question(this, "Delete!", "Delete current profile?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        auto selectedProfileName = ui.channelProfilesComboBox->currentText();
        TCodeChannelLookup::deleteChannelsProfile(selectedProfileName);
        ui.channelProfilesComboBox->removeItem(ui.channelProfilesComboBox->currentIndex());
        auto newProfile = TCodeChannelLookup::getSelectedChannelProfile();
        ui.channelProfilesComboBox->setCurrentText(newProfile);
    }
}

void SettingsDialog::on_defultSelectedProfile_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "WARNING!", "Are you sure you want to reset the channel map for the selected profile?\nThis will reset ALL range and multiplier settings!",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        TCodeChannelLookup::setProfileDefaults();
    }
}


void SettingsDialog::on_allProfilesDefaultButton_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "WARNING!", "Are you sure you want to DEFAULT ALL PROFILES?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        askHowToResetChannelProfileDefaults();
    }
}


void SettingsDialog::on_hideMediaWithoutFunscriptsCheckbox_clicked(bool checked)
{
    XTPSettings::setHideMediaWithoutFunscripts(checked);
    emit updateLibrary();
}


void SettingsDialog::on_cleanupThumbsPushButton_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "WARNING!", "Are you sure you want to clean up generated thumbs?\nThis will search the chosen thumbs directory for files that end with jpg\nthat are either duplicated or not existant in the current library.\nWARNING: if your current libraries doesnt not have some of these thumbs they will be deleted.\nThis will not delete files that are in your media directory.\n\nThis process could take a long time.\nContinue?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        ui.cleanUpThumbsStatus->setText("Thumb cleanup: Running");
        emit cleanUpThumbsDirectory();
    }
}


void SettingsDialog::onCleanUpThumbsDirectoryComplete() {
    ui.cleanUpThumbsStatus->setText("Thumb cleanup: Complete!");
}

void SettingsDialog::onCleanUpThumbsDirectoryStopped() {
    ui.cleanUpThumbsStatus->setText("Thumb cleanup: Stopped!");
}

void SettingsDialog::askHowToResetChannelProfileDefaults() {
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Keep current profiles?", "Do you want to keep the current profile names?",
                                  QMessageBox::Yes|QMessageBox::No);
    TCodeChannelLookup::setAllProfileDefaults(reply == QMessageBox::Yes);
}

void SettingsDialog::on_disableFunscriptHeatmapheckBox_clicked(bool checked)
{
    XTPSettings::setHeatmapDisabled(checked);
    emit disableHeatmapToggled(checked);
}

void SettingsDialog::on_tagSetupButton_clicked()
{
    TagManager tagManager(this);
    tagManager.exec();
}


void SettingsDialog::on_smartTagButton_clicked()
{
    TagManager tagManager(this, true);
    tagManager.exec();
}

void SettingsDialog::on_defaultUserTagsButton_clicked()
{
    QMessageBox::StandardButton reply =
        QMessageBox::question(this, "Reset all user tags?", "This will delete all added user tags. Note: this will not effect smart tags.",
            QMessageBox::Yes|QMessageBox::No);

    if(reply == QMessageBox::Yes)
        SettingsHandler::SetUserTagDefaults();
}


void SettingsDialog::on_defaultSmartTagsButton_clicked()
{
    QMessageBox::StandardButton reply =
        QMessageBox::question(this, "Reset all smart tags?", "This will delete all added smart tags. Note: this will not effect user tags.",
                              QMessageBox::Yes|QMessageBox::No);

    if(reply == QMessageBox::Yes)
        SettingsHandler::SetSmartTagDefaults();
}

void SettingsDialog::onViewedPercentageSpinBoxValueChanged(int arg1)
{
    float percentage = arg1/(float)100;
    SettingsHandler::setViewedThreshold(percentage);
}


void SettingsDialog::onDisableHeartbeatChkClicked(bool checked)
{
    SettingsHandler::setDisableHeartBeat(checked);
    set_requires_restart(true);
}

void SettingsDialog::schedulerEnabledChk_clicked(bool checked)
{
    SettingsHandler::setScheduleLibraryLoadEnabled(checked);
}

void SettingsDialog::onScheduleLibraryLoadAtTimeUserTimeChanged(const QTime &time)
{
    SettingsHandler::setScheduleLibraryLoadTime(time);
}

void SettingsDialog::fullMetadataProcessChk_clicked(bool checked)
{
    SettingsHandler::setScheduleLibraryLoadFullProcess(checked);
}

void SettingsDialog::processMetadataOnStart_clicked(bool checked)
{
    SettingsHandler::setProcessMetadataOnStart(checked);
}

void SettingsDialog::settingsSyncChkClicked(bool checked)
{
    SettingsHandler::setScheduleSettingsSync(checked);
}

void SettingsDialog::on_voiceGroupBox_clicked(bool checked)
{
    SettingsHandler::setDisableSpeechToText(!checked);
}

void SettingsDialog::on_voiceVolumeSlider_sliderReleased()
{
    if(m_tts) {
        int sliderValue = ui.voiceVolumeSlider->value();
        double volume = sliderValue ? sliderValue / 100.0f : 0.0f;
        m_tts->setVolume(volume);
        XTPSettings::setVoiceVolume(volume);
        m_tts->say("Testing, one two three");
    }
}

void SettingsDialog::on_voicePitchSlider_sliderReleased()
{
    if(m_tts) {
        int sliderValue = ui.voicePitchSlider->value();
        double pitch = sliderValue ? sliderValue / 100.0f : 0.0f;
        m_tts->setPitch(pitch);
        XTPSettings::setVoicePitch(pitch);
        m_tts->say("Testing, one two three");
    }
}

void SettingsDialog::on_voiceRateSlider_sliderReleased()
{
    if(m_tts) {
        int sliderValue = ui.voiceRateSlider->value();
        double rate = sliderValue ? sliderValue / 100.0f : 0.0f;
        m_tts->setRate(rate);
        XTPSettings::setVoiceRate(rate);
        m_tts->say("Testing, one two three");
    }
}

void SettingsDialog::on_voiceVolumeSlider_sliderMoved(int position)
{
    double volume = position ? position / 100.0f : 0.0f;
    ui.voiceVolumeLbl->setText("Volume "+ QString::number(volume));
}

void SettingsDialog::on_voicePitchSlider_sliderMoved(int position)
{
    double pitch = position ? position / 100.0f : 0.0f;
    ui.voicePitchLbl->setText("Pitch "+ QString::number(pitch));
}

void SettingsDialog::on_voiceRateSlider_sliderMoved(int position)
{
    double rate = position ? position / 100.0f : 0.0f;
    ui.voiceRateLbl->setText("Rate "+ QString::number(rate));
}

void SettingsDialog::onUseDTRAndRTSChkClicked(bool checked)
{
    if(!checked)
    {
        SettingsHandler::setUseDTRAndRTS(checked);
        return;
    }
    QMessageBox::StandardButton reply =
        QMessageBox::question(this, "Warning!", "DTR and RTS can speed up the serial connection but causes a board reboot.\nBe sure to cut power to servos before connecting.",
                              QMessageBox::Yes|QMessageBox::No);

    if(reply == QMessageBox::Yes)
        SettingsHandler::setUseDTRAndRTS(checked);

}

void SettingsDialog::on_defaultWebDirBtn_clicked()
{
    SettingsHandler::setHttpServerRootDefault();
    ui.httpRootLineEdit->setText(SettingsHandler::getHttpServerRoot());
}


void SettingsDialog::on_defaultVRLibraryBtn_clicked()
{
    SettingsHandler::removeAllVRLibraries();
    ui.vrLibraryLineEdit->setText(SettingsHandler::getLastSelectedVRLibrary());
}


void SettingsDialog::on_playbackRateSpinBoxValueChanged(double arg1)
{
    SettingsHandler::setPlaybackRateStep(arg1);
    set_requires_restart(true);
}

