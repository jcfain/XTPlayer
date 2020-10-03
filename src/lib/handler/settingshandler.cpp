#include "settingshandler.h"

SettingsHandler::SettingsHandler()
{
}
SettingsHandler::~SettingsHandler()
{
    delete settings;
}
bool resetRequired = false;
void SettingsHandler::Load()
{
    QMutexLocker locker(&mutex);
    QCoreApplication::setOrganizationName("cUrbSide prOd");
    QCoreApplication::setOrganizationDomain("https://www.patreon.com/Khrull");
    QCoreApplication::setApplicationName("XTPlayer");
    QFile settingsini(QApplication::applicationDirPath() + "/settings.ini");
    if(settingsini.exists())
    {
        settings = new QSettings("settings.ini", QSettings::Format::IniFormat);
    }
    else
    {
        settings = new QSettings("cUrbSide prOd", "XTPlayer");
    }
    float currentVersion = settings->value("version").toFloat();
    if (currentVersion == 0)
    {
        locker.unlock();
//        if((currentVersion < XTPVersionNum && resetRequired))
//            LogHandler::Dialog("Sorry, your settings have been set to default for a new data structure.", XLogLevel::Information);
        Default();
        defaultReset = false;
    }
    else if(currentVersion < XTPVersionNum && resetRequired)
    {
        SetMapDefaults();
    }
    locker.relock();
    selectedTheme = settings->value("selectedTheme").toString();
    selectedTheme = selectedTheme.isNull() ? QApplication::applicationDirPath() + "/themes/black-silver.css" : selectedTheme;
    selectedLibrary = settings->value("selectedLibrary").toString();
    selectedDevice = settings->value("selectedDevice").toInt();
    playerVolume = settings->value("playerVolume").toInt();
    offSet = settings->value("offSet").toInt();
    selectedFunscriptLibrary = settings->value("selectedFunscriptLibrary").toString();
    serialPort = settings->value("serialPort").toString();
    serverAddress = settings->value("serverAddress").toString();
    serverPort = settings->value("serverPort").toString();
    deoAddress = settings->value("deoAddress").toString();
    deoAddress = deoAddress.isNull() ? "127.0.0.1" : deoAddress;
    deoPort = settings->value("deoPort").toString();
    deoPort = deoPort.isNull() ? "23554" : deoPort;
    deoEnabled = settings->value("deoEnabled").toBool();
    yRollMultiplierChecked = settings->value("yRollMultiplierChecked").toBool();
    yRollMultiplierValue = settings->value("yRollMultiplierValue").toFloat();
    xRollMultiplierChecked = settings->value("xRollMultiplierChecked").toBool();
    xRollMultiplierValue = settings->value("xRollMultiplierValue").toFloat();
    twistMultiplierChecked = settings->value("twistMultiplierChecked").toBool();
    twistMultiplierValue = settings->value("twistMultiplierValue").toFloat();
    twistMultiplierChecked = settings->value("vibMultiplierChecked").toBool();
    twistMultiplierValue = settings->value("vibMultiplierValue").toFloat();

    libraryView = settings->value("libraryView").toInt();
    thumbSize = settings->value("thumbSize").toInt();
    thumbSize = thumbSize == 0 ? 150 : thumbSize;
    thumbSizeList = settings->value("thumbSizeList").toInt();
    thumbSizeList = thumbSizeList == 0 ? 50 : thumbSizeList;
    videoIncrement = settings->value("videoIncrement").toInt();
    videoIncrement = videoIncrement == 0 ? 10 : videoIncrement;
    deoDnlaFunscriptLookup = settings->value("deoDnlaFunscriptLookup").toHash();

    _gamePadEnabled = settings->value("gamePadEnabled").toBool();
//    qRegisterMetaTypeStreamOperators<ChannelModel>("ChannelModel");
//    qRegisterMetaType<ChannelModel>();
    QVariantMap availableAxis = settings->value("availableAxis").toMap();
    _availableAxis.clear();
    foreach(auto axis, availableAxis.keys())
    {
        _availableAxis.insert(axis, availableAxis[axis].value<ChannelModel>());
    }
    _liveXRangeMax = _availableAxis[axisNames.TcXUpDownL0].UserMax;
    _liveXRangeMin = _availableAxis[axisNames.TcXUpDownL0].UserMin;
    QVariantMap gamepadButtonMap = settings->value("gamepadButtonMap").toMap();
    foreach(auto button, gamepadButtonMap.keys())
    {
        _gamepadButtonMap.insert(button, gamepadButtonMap[button].toString());
    }
    _inverseTcXL0 = settings->value("inverseTcXL0").toBool();
    _inverseTcXRollR2 = settings->value("inverseTcXRollR2").toBool();
    _inverseTcYRollR1 = settings->value("inverseTcYRollR1").toBool();
    _gamepadSpeed = settings->value("gamepadSpeed").toInt();
    _gamepadSpeed = _gamepadSpeed == 0 ? 1000 : _gamepadSpeed;
    _gamepadSpeedStep = settings->value("gamepadSpeedStep").toInt();
    _gamepadSpeedStep = _gamepadSpeedStep == 0 ? 500 : _gamepadSpeedStep;
    _livegamepadSpeed = _gamepadSpeed;
    _xRangeStep = settings->value("xRangeStep").toInt();
    _xRangeStep = _xRangeStep == 0 ? 50 : _xRangeStep;
    disableSpeechToText = settings->value("disableSpeechToText").toBool();
//    SetupAvailableAxis();
//    SetupGamepadButtonMap();
}

void SettingsHandler::Save()
{
    QMutexLocker locker(&mutex);
    if (!defaultReset)
    {
        settings->setValue("version", XTPVersionNum);
        settings->setValue("selectedLibrary", selectedLibrary);
        settings->setValue("selectedTheme", selectedTheme);
        settings->setValue("selectedDevice", selectedDevice);
        settings->setValue("playerVolume", playerVolume);
        settings->setValue("offSet", offSet);
        settings->setValue("selectedFunscriptLibrary", selectedFunscriptLibrary);
        settings->setValue("serialPort", serialPort);
        settings->setValue("serverAddress", serverAddress);
        settings->setValue("serverPort", serverPort);
        settings->setValue("deoAddress", deoAddress);
        settings->setValue("deoPort", deoPort);
        settings->setValue("deoEnabled", deoEnabled);
        settings->setValue("yRollMultiplierChecked", yRollMultiplierChecked);
        settings->setValue("yRollMultiplierValue", yRollMultiplierValue);
        settings->setValue("xRollMultiplierChecked", xRollMultiplierChecked);
        settings->setValue("xRollMultiplierValue", xRollMultiplierValue);
        settings->setValue("twistMultiplierChecked", twistMultiplierChecked);
        settings->setValue("twistMultiplierValue", twistMultiplierValue);
        settings->setValue("vibMultiplierChecked", twistMultiplierChecked);
        settings->setValue("vibMultiplierValue", twistMultiplierValue);

        settings->setValue("libraryView", libraryView);
        settings->setValue("thumbSize", thumbSize);
        settings->setValue("thumbSizeList", thumbSizeList);
        settings->setValue("videoIncrement", videoIncrement);

        settings->setValue("deoDnlaFunscriptLookup", deoDnlaFunscriptLookup);

        settings->setValue("gamePadEnabled", _gamePadEnabled);
//        qRegisterMetaTypeStreamOperators<ChannelModel>("ChannelModel");
//        qRegisterMetaType<ChannelModel>();
        QVariantMap availableAxis;
        foreach(auto axis, _availableAxis.keys())
        {
            availableAxis.insert(axis, QVariant::fromValue(_availableAxis[axis]));
        }
        settings->setValue("availableAxis", availableAxis);
        QVariantMap gamepadMap;
        foreach(auto button, _gamepadButtonMap.keys())
        {
            gamepadMap.insert(button, QVariant::fromValue(_gamepadButtonMap[button]));
        }
        settings->setValue("gamepadButtonMap", gamepadMap);
        settings->setValue("inverseTcXL0", _inverseTcXL0);
        settings->setValue("inverseTcXRollR2", _inverseTcXRollR2);
        settings->setValue("inverseTcYRollR1", _inverseTcYRollR1);
        settings->setValue("gamepadSpeed", _gamepadSpeed);
        settings->setValue("gamepadSpeedStep", _gamepadSpeedStep);
        settings->setValue("xRangeStep", _xRangeStep);
        settings->setValue("disableSpeechToText", disableSpeechToText);
        settings->sync();
    }

}
void SettingsHandler::Clear()
{
    QMutexLocker locker(&mutex);
    defaultReset = true;
    settings->clear();
}
void SettingsHandler::Default()
{
    QMutexLocker locker(&mutex);
    defaultReset = true;
    settings->clear();
    SetMapDefaults();
    settings->setValue("version", XTPVersionNum);
//    settings->setValue("selectedTheme", QApplication::applicationDirPath() + "/themes/black-silver.css");
//    settings->setValue("selectedLibrary", QVariant::String);
//    settings->setValue("selectedDevice", DeviceType::Serial);
//    settings->setValue("playerVolume", 0);
//    settings->setValue("offSet", 0);
//    settings->setValue("speed", 1000);
//    settings->setValue("selectedFunscriptLibrary", QVariant::String);
//    settings->setValue("serialPort", QVariant::String);
//    settings->setValue("serverAddress", QVariant::String);
//    settings->setValue("serverPort", "0");
//    settings->setValue("deoAddress", "127.0.0.1");
//    settings->setValue("deoPort", "23554");
//    settings->setValue("deoEnabled", false);
//    settings->setValue("yRollMultiplierChecked", false);
//    settings->setValue("yRollMultiplierValue", 0);
//    settings->setValue("xRollMultiplierChecked", false);
//    settings->setValue("xRollMultiplierValue", 0);
//    settings->setValue("twistMultiplierChecked", false);
//    settings->setValue("twistMultiplierValue", 0);
//    settings->setValue("vibMultiplierChecked", false);
//    settings->setValue("vibMultiplierValue", 0);

//    settings->setValue("libraryView", 0);
//    settings->setValue("thumbSize", 150);
//    settings->setValue("thumbSizeList", 50);

//    settings->setValue("gamePadEnabled", false);
//    settings->setValue("inverseTcXL0", false);
//    settings->setValue("inverseTcXRollR2", false);
//    settings->setValue("inverseTcYRollR1", false);
//    SetMapDefaults();
}

void SettingsHandler::SetMapDefaults()
{
    SetupAvailableAxis();
    SetupGamepadButtonMap();
//    qRegisterMetaTypeStreamOperators<ChannelModel>("ChannelModel");
//    qRegisterMetaType<ChannelModel>();
    QVariantHash availableAxis;
    foreach(auto axis, _availableAxis.keys())
    {
        QVariant axisVariant;
        axisVariant.setValue(_availableAxis[axis]);
        availableAxis.insert(axis, axisVariant);
    }
    settings->setValue("availableAxis", availableAxis);
    QVariantHash gamepadMap;
    foreach(auto button, _gamepadButtonMap.keys())
    {
        QVariant buttonVariant;
        buttonVariant.setValue(_gamepadButtonMap[button]);
        gamepadMap.insert(button, buttonVariant);
    }
    settings->setValue("gamepadButtonMap", gamepadMap);
}

QString SettingsHandler::getSelectedTheme()
{
    return selectedTheme;
}
QString SettingsHandler::getSelectedLibrary()
{
    return selectedLibrary;
}
 QString SettingsHandler::getSelectedFunscriptLibrary()
{
    return selectedFunscriptLibrary;
}
int SettingsHandler::getSelectedDevice()
{
    return selectedDevice;
}
QString SettingsHandler::getSerialPort()
{
    return serialPort;
}
QString SettingsHandler::getServerAddress()
{
    return serverAddress;
}
QString SettingsHandler::getServerPort()
{
    return serverPort;
}
QString SettingsHandler::getDeoAddress()
{
    return deoAddress;
}
QString SettingsHandler::getDeoPort()
{
    return deoPort;
}
bool SettingsHandler::getDeoEnabled()
{
    return deoEnabled;
}
int SettingsHandler::getPlayerVolume()
{
    return playerVolume;
}
int SettingsHandler::getoffSet()
{
    return offSet;
}
int SettingsHandler::getXMin()
{
    return getAxis(axisNames.TcXUpDownL0).UserMin;
}
int SettingsHandler::getYRollMin()
{
    return getAxis(axisNames.TcYRollR1).UserMin;
}
int SettingsHandler::getXRollMin()
{
    return getAxis(axisNames.TcXRollR2).UserMin;
}
int SettingsHandler::getXMax()
{
    return getAxis(axisNames.TcXUpDownL0).UserMax;
}
int SettingsHandler::getYRollMax()
{
    return getAxis(axisNames.TcYRollR1).UserMax;
}
int SettingsHandler::getXRollMax()
{
    return getAxis(axisNames.TcXRollR2).UserMax;
}
int SettingsHandler::getTwistMax()
{
    return getAxis(axisNames.TcTwistR0).UserMax;
}
int SettingsHandler::getTwistMin()
{
    return getAxis(axisNames.TcTwistR0).UserMin;
}
float SettingsHandler::getMultiplierValue(QString channel)
{
    if(channel == "R0")
    {
        return getTwistMultiplierValue();
    }
    else if(channel == "R1")
    {
        return getYRollMultiplierValue();
    }
    else if(channel == "R2")
    {
        return getXRollMultiplierValue();
    }
    else if(channel == "V0")
    {
        return getVibMultiplierValue();
    }
    return 0.0;
}
bool SettingsHandler::getMultiplierChecked(QString channel)
{
    if(channel == "R0")
    {
        return getTwistMultiplierChecked();
    }
    else if(channel == "R1")
    {
        return getYRollMultiplierChecked();
    }
    else if(channel == "R2")
    {
        return getXRollMultiplierChecked();
    }
    else if(channel == "V0")
    {
        return getVibMultiplierChecked();
    }
    return false;
}
bool SettingsHandler::getYRollMultiplierChecked()
{
    return yRollMultiplierChecked;
}
float SettingsHandler::getYRollMultiplierValue()
{
    return yRollMultiplierValue;
}
bool SettingsHandler::getXRollMultiplierChecked()
{
    return xRollMultiplierChecked;
}
float SettingsHandler::getXRollMultiplierValue()
{
    return xRollMultiplierValue;
}
bool SettingsHandler::getTwistMultiplierChecked()
{
    return twistMultiplierChecked;
}
float SettingsHandler::getTwistMultiplierValue()
{
    return twistMultiplierValue;
}
bool SettingsHandler::getVibMultiplierChecked()
{
    return vibMultiplierChecked;
}
float SettingsHandler::getVibMultiplierValue()
{
    return vibMultiplierValue;
}

int SettingsHandler::getLibraryView()
{
    return libraryView;
}
int SettingsHandler::getThumbSize()
{
    return thumbSize;
}
int SettingsHandler::getThumbSizeList()
{
    return thumbSizeList;
}
int SettingsHandler::getVideoIncrement()
{
    return videoIncrement;
}
QString SettingsHandler::getDeoDnlaFunscript(QString key)
{
    if (deoDnlaFunscriptLookup.contains(key))
    {
        return deoDnlaFunscriptLookup[key].toString();
    }
    return nullptr;
}

bool SettingsHandler::getGamepadEnabled()
{
    return _gamePadEnabled;
}

bool SettingsHandler::getInverseTcXL0()
{
    return _inverseTcXL0;
}
bool SettingsHandler::getInverseTcXRollR2()
{
    return _inverseTcXRollR2;
}
bool SettingsHandler::getInverseTcYRollR1()
{
    return _inverseTcYRollR1;
}

int SettingsHandler::getGamepadSpeed()
{
    return _gamepadSpeed;
}

void SettingsHandler::setGamepadSpeed(int value)
{
    _gamepadSpeed = value;
}

int SettingsHandler::getGamepadSpeedIncrement()
{
    return _gamepadSpeedStep;
}

void SettingsHandler::setGamepadSpeedStep(int value)
{
    QMutexLocker locker(&mutex);
    _gamepadSpeedStep = value;
}

int SettingsHandler::getLiveGamepadSpeed()
{
    return _livegamepadSpeed;
}

void SettingsHandler::setLiveGamepadSpeed(int value)
{
    QMutexLocker locker(&mutex);
    _livegamepadSpeed = value;
}

void SettingsHandler::setXRangeStep(int value)
{
    QMutexLocker locker(&mutex);
    _xRangeStep = value;
}

int SettingsHandler::getXRangeStep()
{
    return _xRangeStep;
}

void SettingsHandler::setLiveXRangeMin(int value)
{
    QMutexLocker locker(&mutex);
    _liveXRangeMin = value;
}

int SettingsHandler::getLiveXRangeMin()
{
    return _liveXRangeMin;
}

void SettingsHandler::setLiveXRangeMax(int value)
{
    QMutexLocker locker(&mutex);
    _liveXRangeMax = value;
}

int SettingsHandler::getLiveXRangeMax()
{
    return _liveXRangeMax;
}
void SettingsHandler::resetLiveXRange()
{
    QMutexLocker locker(&mutex);
    AxisNames axisNames;
    _liveXRangeMax = _availableAxis.value(axisNames.TcXUpDownL0).UserMax;
    _liveXRangeMin = _availableAxis.value(axisNames.TcXUpDownL0).UserMin;
}
bool SettingsHandler::getDisableSpeechToText()
{
    return disableSpeechToText;
}
void SettingsHandler::setDisableSpeechToText(bool value)
{
    QMutexLocker locker(&mutex);
    disableSpeechToText = value;
}
ChannelModel SettingsHandler::getAxis(QString axis)
{
    QMutexLocker locker(&mutex);
    return _availableAxis[axis];
}
QMap<QString, QString>  SettingsHandler::getGamePadMap()
{
    return _gamepadButtonMap;
}
QMap<QString, ChannelModel>  SettingsHandler::getAvailableAxis()
{
    return _availableAxis;
}
QString SettingsHandler::getGamePadMapButton(QString gamepadAxis)
{
    if (_gamepadButtonMap.contains(gamepadAxis))
    {
        return _gamepadButtonMap[gamepadAxis];
    }
    return nullptr;
}
void SettingsHandler::setSelectedTheme(QString value)
{
    QMutexLocker locker(&mutex);
    selectedTheme = value;
}
void SettingsHandler::setSelectedLibrary(QString value)
{
    QMutexLocker locker(&mutex);
    selectedLibrary = value;
}
void SettingsHandler::setSelectedFunscriptLibrary(QString value)
{
    QMutexLocker locker(&mutex);
    selectedFunscriptLibrary = value;
}
void SettingsHandler::setSelectedDevice(int value)
{
    QMutexLocker locker(&mutex);
    selectedDevice = value;
}
void SettingsHandler::setSerialPort(QString value)
{
    QMutexLocker locker(&mutex);
    serialPort = value;
}
void SettingsHandler::setServerAddress(QString value)
{
    QMutexLocker locker(&mutex);
    serverAddress = value;
}
void SettingsHandler::setServerPort(QString value)
{
    QMutexLocker locker(&mutex);
    serverPort = value;
}
void SettingsHandler::setDeoAddress(QString value)
{
    QMutexLocker locker(&mutex);
    deoAddress = value;
}
void SettingsHandler::setDeoPort(QString value)
{
    QMutexLocker locker(&mutex);
    deoPort = value;
}
void SettingsHandler::setDeoEnabled(bool value)
{
    QMutexLocker locker(&mutex);
    deoEnabled = value;
}
void SettingsHandler::setPlayerVolume(int value)
{
    QMutexLocker locker(&mutex);
    playerVolume = value;
}
void SettingsHandler::setoffSet(int value)
{
    QMutexLocker locker(&mutex);
    offSet = value;
}
void SettingsHandler::setXMin(int value)
{
    QMutexLocker locker(&mutex);
    _liveXRangeMin = value;
    _availableAxis[axisNames.TcXUpDownL0].UserMin = value;
}
void SettingsHandler::setYRollMin(int value)
{
    QMutexLocker locker(&mutex);
    _availableAxis[axisNames.TcYRollR1].UserMin = value;
}
void SettingsHandler::setXRollMin(int value)
{
    QMutexLocker locker(&mutex);
    _availableAxis[axisNames.TcXRollR2].UserMin = value;
}
void SettingsHandler::setXMax(int value)
{
    QMutexLocker locker(&mutex);
    _liveXRangeMax = value;
    _availableAxis[axisNames.TcXUpDownL0].UserMax = value;
}
void SettingsHandler::setYRollMax(int value)
{
    QMutexLocker locker(&mutex);
    _availableAxis[axisNames.TcYRollR1].UserMax = value;
}
void SettingsHandler::setXRollMax(int value)
{
    QMutexLocker locker(&mutex);
    _availableAxis[axisNames.TcXRollR2].UserMax = value;
}
void SettingsHandler::setTwistMax(int value)
{
    QMutexLocker locker(&mutex);
    _availableAxis[axisNames.TcTwistR0].UserMax = value;
}
void SettingsHandler::setTwistMin(int value)
{
    QMutexLocker locker(&mutex);
    _availableAxis[axisNames.TcTwistR0].UserMin = value;
}

void SettingsHandler::setYRollMultiplierChecked(bool value)
{
    QMutexLocker locker(&mutex);
    yRollMultiplierChecked = value;
}
void SettingsHandler::setYRollMultiplierValue(float value)
{
    QMutexLocker locker(&mutex);
    yRollMultiplierValue = value;
}
void SettingsHandler::setXRollMultiplierChecked(bool value)
{
    QMutexLocker locker(&mutex);
    xRollMultiplierChecked = value;
}
void SettingsHandler::setXRollMultiplierValue(float value)
{
    QMutexLocker locker(&mutex);
    xRollMultiplierValue = value;
}
void SettingsHandler::setTwistMultiplierChecked(bool value)
{
    QMutexLocker locker(&mutex);
    twistMultiplierChecked = value;
}
void SettingsHandler::setTwistMultiplierValue(float value)
{
    QMutexLocker locker(&mutex);
    twistMultiplierValue = value;
}
void SettingsHandler::setVibMultiplierChecked(bool value)
{
    QMutexLocker locker(&mutex);
    twistMultiplierChecked = value;
}
void SettingsHandler::setVibMultiplierValue(float value)
{
    QMutexLocker locker(&mutex);
    twistMultiplierValue = value;
}

void SettingsHandler::setLibraryView(int value)
{
    QMutexLocker locker(&mutex);
    libraryView = value;
}
void SettingsHandler::setThumbSize(int value)
{
    QMutexLocker locker(&mutex);
    thumbSize = value;
}
void SettingsHandler::setThumbSizeList(int value)
{
    QMutexLocker locker(&mutex);
    thumbSizeList = value;
}
void SettingsHandler::setVideoIncrement(int value)
{
    videoIncrement = value;
}
void SettingsHandler::setDeoDnlaFunscript(QString key, QString value)
{
    QMutexLocker locker(&mutex);
    deoDnlaFunscriptLookup[key] = value;
}

void SettingsHandler::setGamepadEnabled(bool value)
{
    QMutexLocker locker(&mutex);
    _gamePadEnabled = value;
}

void SettingsHandler::setAxis(QString axis, ChannelModel channel)
{
    QMutexLocker locker(&mutex);
    _availableAxis[axis] = channel;
}

void SettingsHandler::setGamePadMapButton(QString gamePadButton, QString axis)
{
    QMutexLocker locker(&mutex);
    _gamepadButtonMap[gamePadButton] = axis;
}

void SettingsHandler::setInverseTcXL0(bool value)
{
    _inverseTcXL0 = value;
}
void SettingsHandler::setInverseTcXRollR2(bool value)
{
    _inverseTcXRollR2 = value;
}
void SettingsHandler::setInverseTcYRollR1(bool value)
{
    _inverseTcYRollR1  = value;
}

//private
void SettingsHandler::SetupAvailableAxis()
{
    _availableAxis = {
        {axisNames.None, { "None", axisNames.None, axisNames.None, 1, 500, 999, 1, 500, 999 } },
        {axisNames.TcXUpDownL0, { "X (Up/down L0)", axisNames.TcXUpDownL0, "L0", 1, 500, 999, 1, 500, 999 } },
        {axisNames.TcXDownL0, { "X (Down)", axisNames.TcXDownL0, "L0", 1, 500, 999, 1, 500, 999 } },
        {axisNames.TcXUpL0, { "X (Up)", axisNames.TcXUpL0, "L0", 1, 500, 999, 1, 500, 999 } },
        {axisNames.TcXRollR2, { "X (Roll R2)", axisNames.TcXRollR2, "R2", 1, 500, 999, 1, 500, 999 } },
        {axisNames.TcXRollForwardR2, { "X (Roll Forward)", axisNames.TcXRollForwardR2, "R2", 1, 500, 999, 1, 500, 999 } },
        {axisNames.TcXRollBackR2, { "X (Roll Back)", axisNames.TcXRollBackR2, "R2", 1, 500, 999, 1, 500, 999 } },
        {axisNames.TcYRollR1, { "Y (Roll R1)", axisNames.TcYRollR1, "R1", 1, 500, 999, 1, 500, 999 } },
        {axisNames.TcYRollLeftR1, { "Y (Roll Left)", axisNames.TcYRollLeftR1, "R1", 1, 500, 999, 1, 500, 999 } },
        {axisNames.TcYRollRightR1, { "Y (Roll Right)", axisNames.TcYRollRightR1, "R1", 1, 500, 999, 1, 500, 999 } },
        {axisNames.TcTwistR0, { "Twist R0", axisNames.TcTwistR0, "R0", 1, 500, 999, 1, 500, 999 } },
        {axisNames.TcTwistCWR0, { "Twist (CW)", axisNames.TcTwistCWR0, "R0", 1, 500, 999, 1, 500, 999 } },
        {axisNames.TcTwistCCWR0, { "Twist (CCW)", axisNames.TcTwistCCWR0, "R0", 1, 500, 999, 1, 500, 999 } },
        {axisNames.TcVibV0, { "Vib V0", axisNames.TcVibV0, "V0", 1, 500, 999, 1, 500, 999 } },
        {axisNames.TcPumpV2, { "Pump V2", axisNames.TcPumpV2, "V2", 1, 500, 999, 1, 500, 999 } }
    };
}

void SettingsHandler::SetupGamepadButtonMap()
{
    _gamepadButtonMap = {
        { "None", axisNames.None },
        { gamepadAxisNames.LeftXAxis, axisNames.TcTwistR0 },
        { gamepadAxisNames.LeftYAxis,  axisNames.TcXUpDownL0 },
        { gamepadAxisNames.RightYAxis ,  axisNames.TcXRollR2  },
        { gamepadAxisNames.RightXAxis, axisNames.TcYRollR1  },
        { gamepadAxisNames.RightTrigger, mediaActions.IncreaseXRange },
        { gamepadAxisNames.LeftTrigger, mediaActions.DecreaseXRange },
        { gamepadAxisNames.RightBumper, mediaActions.FastForward },
        { gamepadAxisNames.LeftBumper, mediaActions.Rewind },
        { gamepadAxisNames.Select, mediaActions.FullScreen },
        { gamepadAxisNames.Start, mediaActions.TogglePause },
        { gamepadAxisNames.X, axisNames.None },
        { gamepadAxisNames.Y, mediaActions.Loop },
        { gamepadAxisNames.B, mediaActions.Stop },
        { gamepadAxisNames.A, mediaActions.Mute },
        { gamepadAxisNames.DPadUp, mediaActions.VolumeUp },
        { gamepadAxisNames.DPadDown, mediaActions.VolumeDown },
        { gamepadAxisNames.DPadLeft, mediaActions.Back },
        { gamepadAxisNames.DPadRight, mediaActions.Next },
        { gamepadAxisNames.RightAxisButton, axisNames.None },
        { gamepadAxisNames.LeftAxisButton, axisNames.None },
        { gamepadAxisNames.Center, axisNames.None },
        { gamepadAxisNames.Guide, axisNames.None }
    };
}
const QString SettingsHandler::TCodeVersion = "TCode v0.2";
const QString SettingsHandler::XTPVersion = "0.16b2";
const float SettingsHandler::XTPVersionNum = 0.16f;

QSettings* SettingsHandler::settings;
QMutex SettingsHandler::mutex;
GamepadAxisNames SettingsHandler::gamepadAxisNames;
AxisNames SettingsHandler::axisNames;
MediaActions SettingsHandler::mediaActions;
QHash<QString, QVariant> SettingsHandler::deoDnlaFunscriptLookup;
QString SettingsHandler::selectedTheme;
QString SettingsHandler::selectedLibrary;
int SettingsHandler::selectedDevice;
int SettingsHandler::playerVolume;
int SettingsHandler::offSet;
//int SettingsHandler::xMin;
//int SettingsHandler::yRollMin;
//int SettingsHandler::xRollMin;
//int SettingsHandler::xMax;
//int SettingsHandler::yRollMax;
//int SettingsHandler::xRollMax;
//int SettingsHandler::twistMin;
//int SettingsHandler::twistMax;
bool SettingsHandler::yRollMultiplierChecked;
float SettingsHandler::yRollMultiplierValue;
bool SettingsHandler::xRollMultiplierChecked;
float SettingsHandler::xRollMultiplierValue;
bool SettingsHandler::twistMultiplierChecked;
float SettingsHandler::twistMultiplierValue;
bool SettingsHandler::vibMultiplierChecked;
float SettingsHandler::vibMultiplierValue;

int SettingsHandler::libraryView = LibraryView::Thumb;
int SettingsHandler::thumbSize = 175;
int SettingsHandler::thumbSizeList = 50;
int SettingsHandler::videoIncrement = 10;

bool SettingsHandler::_gamePadEnabled;
QMap<QString, QString> SettingsHandler::_gamepadButtonMap;
QMap<QString, ChannelModel> SettingsHandler::_availableAxis;
bool SettingsHandler::_inverseTcXL0;
bool SettingsHandler::_inverseTcXRollR2;
bool SettingsHandler::_inverseTcYRollR1;
int SettingsHandler::_gamepadSpeed;
int SettingsHandler::_gamepadSpeedStep;
int SettingsHandler::_livegamepadSpeed;

int SettingsHandler::_xRangeStep;
int SettingsHandler::_liveXRangeMax;
int SettingsHandler::_liveXRangeMin;

QString SettingsHandler::selectedFunscriptLibrary;
QString SettingsHandler::selectedFile;
QString SettingsHandler::serialPort;
QString SettingsHandler::serverAddress;
QString SettingsHandler::serverPort;
QString SettingsHandler::deoAddress;
QString SettingsHandler::deoPort;
bool SettingsHandler::deoEnabled;
bool SettingsHandler::defaultReset = false;
bool SettingsHandler::disableSpeechToText;
