#include "settingshandler.h"

const QString SettingsHandler::TCodeVersion = "TCode v0.2";
const QString SettingsHandler::XTPVersion = "0.23";
const float SettingsHandler::XTPVersionNum = 0.23f;

SettingsHandler::SettingsHandler()
{
}
SettingsHandler::~SettingsHandler()
{
    delete settings;
}
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
        SetMapDefaults();
        SetupDecoderPriority();
        QList<QVariant> decoderVarient;
        foreach(auto decoder, decoderPriority)
        {
            decoderVarient.append(QVariant::fromValue(decoder));
        }
        settings->setValue("decoderPriority", decoderVarient);
        defaultReset = false;
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
    serverPort = serverPort.isNull() ? "8000" : serverPort;
    deoAddress = settings->value("deoAddress").toString();
    deoAddress = deoAddress.isNull() ? "127.0.0.1" : deoAddress;
    deoPort = settings->value("deoPort").toString();
    deoPort = deoPort.isNull() ? "23554" : deoPort;
    deoEnabled = settings->value("deoEnabled").toBool();

    whirligigAddress = settings->value("whirligigAddress").toString();
    whirligigAddress = whirligigAddress.isNull() ? "127.0.0.1" : whirligigAddress;
    whirligigPort = settings->value("whirligigPort").toString();
    whirligigPort = whirligigPort.isNull() ? "2000" : whirligigPort;
    whirligigEnabled = settings->value("whirligigEnabled").toBool();

    yRollMultiplierChecked = settings->value("rollMultiplierChecked").toBool();
    yRollMultiplierValue = settings->value("rollMultiplierValue").toFloat() ?: 0.01f;
    xRollMultiplierChecked = settings->value("pitchMultiplierChecked").toBool();
    xRollMultiplierValue = settings->value("pitchMultiplierValue").toFloat() ?: 0.01f;
    zMultiplierChecked = settings->value("surgeMultiplierChecked").toBool();
    zMultiplierValue = settings->value("surgeMultiplierValue").toFloat() ?: 0.01f;
    yMultiplierChecked = settings->value("swayMultiplierChecked").toBool();
    yMultiplierValue = settings->value("swayMultiplierValue").toFloat() ?: 0.01f;
    twistMultiplierChecked = settings->value("twistMultiplierChecked").toBool();
    twistMultiplierValue = settings->value("twistMultiplierValue").toFloat() ?: 0.01f;
    vibMultiplierChecked = settings->value("vibMultiplierChecked").toBool();
    vibMultiplierValue = settings->value("vibMultiplierValue").toFloat() ?: 0.01f;
    suckMultiplierChecked = settings->value("suckMultiplierChecked").toBool();
    suckMultiplierValue = settings->value("suckMultiplierValue").toFloat() ?: 0.01f;

    libraryView = settings->value("libraryView").toInt();
    selectedLibrarySortMode = settings->value("selectedLibrarySortMode").toInt();
    thumbSize = settings->value("thumbSize").toInt();
    thumbSize = thumbSize == 0 ? 150 : thumbSize;
    thumbSizeList = settings->value("thumbSizeList").toInt();
    thumbSizeList = thumbSizeList == 0 ? 50 : thumbSizeList;
    videoIncrement = settings->value("videoIncrement").toInt();
    videoIncrement = videoIncrement == 0 ? 10 : videoIncrement;
    deoDnlaFunscriptLookup = settings->value("deoDnlaFunscriptLookup").toHash();

    _gamePadEnabled = settings->value("gamePadEnabled").toBool();
    _multiplierEnabled = settings->value("multiplierEnabled").toBool();
    _liveMultiplierEnabled = _multiplierEnabled;
    QVariantMap availableAxis = settings->value("availableAxis").toMap();
    foreach(auto axis, availableAxis.keys())
    {
        _availableAxis.insert(axis, availableAxis[axis].value<ChannelModel>());
    }
    _liveXRangeMax = _availableAxis[axisNames.Stroke].UserMax;
    _liveXRangeMin = _availableAxis[axisNames.Stroke].UserMin;
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
    _liveGamepadSpeed = _gamepadSpeed;
    _xRangeStep = settings->value("xRangeStep").toInt();
    _xRangeStep = _xRangeStep == 0 ? 50 : _xRangeStep;
    disableSpeechToText = settings->value("disableSpeechToText").toBool();
    QList<QVariant> decoderPriorityvarient = settings->value("decoderPriority").toList();
    decoderPriority.clear();
    foreach(auto varient, decoderPriorityvarient)
    {
        decoderPriority.append(varient.value<DecoderModel>());
    }
    auto splitterSizes = settings->value("mainWindowPos").toList();
    foreach (auto splitterPos, splitterSizes)
    {
        _mainWindowPos.append(splitterPos.value<int>());
    }
    if(currentVersion != 0 && currentVersion < 0.2f)
    {
        SetupGamepadButtonMap();
    }
    if(currentVersion != 0 && currentVersion < 0.23f)
    {
        locker.unlock();
        MigrateTo23();
    }
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
        settings->setValue("whirligigAddress", whirligigAddress);
        settings->setValue("whirligigPort", whirligigPort);
        settings->setValue("whirligigEnabled", whirligigEnabled);

        settings->setValue("rollMultiplierChecked", yRollMultiplierChecked);
        settings->setValue("rollMultiplierValue", yRollMultiplierValue);
        settings->setValue("pitchMultiplierChecked", xRollMultiplierChecked);
        settings->setValue("pitchMultiplierValue", xRollMultiplierValue);
        settings->setValue("surgeMultiplierChecked", zMultiplierChecked);
        settings->setValue("surgeMultiplierValue", zMultiplierValue);
        settings->setValue("swayMultiplierChecked", yMultiplierChecked);
        settings->setValue("swayMultiplierValue", yMultiplierValue);
        settings->setValue("twistMultiplierChecked", twistMultiplierChecked);
        settings->setValue("twistMultiplierValue", twistMultiplierValue);
        settings->setValue("vibMultiplierChecked", vibMultiplierChecked);
        settings->setValue("vibMultiplierValue", vibMultiplierValue);
        settings->setValue("suckMultiplierChecked", suckMultiplierChecked);
        settings->setValue("suckMultiplierValue", suckMultiplierValue);

        settings->setValue("libraryView", libraryView);
        settings->setValue("selectedLibrarySortMode", selectedLibrarySortMode);

        settings->setValue("thumbSize", thumbSize);
        settings->setValue("thumbSizeList", thumbSizeList);
        settings->setValue("videoIncrement", videoIncrement);

        settings->setValue("deoDnlaFunscriptLookup", deoDnlaFunscriptLookup);

        settings->setValue("gamePadEnabled", _gamePadEnabled);
        settings->setValue("multiplierEnabled", _multiplierEnabled);

        QList<QVariant> decoderVarient;
        foreach(auto decoder, decoderPriority)
        {
            decoderVarient.append(QVariant::fromValue(decoder));
        }
        settings->setValue("decoderPriority", decoderVarient);
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
        QList<QVariant> splitterPos;
        foreach(auto pos, _mainWindowPos)
        {
            splitterPos.append(pos);
        }
        settings->setValue("mainWindowPos", splitterPos);

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

void SettingsHandler::MigrateTo23()
{
    settings->setValue("version", XTPVersionNum);
    SetupAvailableAxis();
    SetupDecoderPriority();
    yRollMultiplierChecked = settings->value("yRollMultiplierChecked").toBool();
    yRollMultiplierValue = settings->value("yRollMultiplierValue").toFloat();
    xRollMultiplierChecked = settings->value("xRollMultiplierChecked").toBool();
    xRollMultiplierValue = settings->value("xRollMultiplierValue").toFloat();
    settings->setValue("rollMultiplierChecked", yRollMultiplierChecked);
    settings->setValue("rollMultiplierValue", yRollMultiplierValue);
    settings->setValue("pitchMultiplierChecked", xRollMultiplierChecked);
    settings->setValue("pitchMultiplierValue", xRollMultiplierValue);
    settings->setValue("surgeMultiplierChecked", zMultiplierChecked);
    settings->setValue("surgeMultiplierValue", zMultiplierValue);
    settings->setValue("swayMultiplierChecked", yMultiplierChecked);
    settings->setValue("swayMultiplierValue", yMultiplierValue);
    settings->setValue("twistMultiplierChecked", twistMultiplierChecked);
    settings->setValue("twistMultiplierValue", twistMultiplierValue);
    settings->setValue("vibMultiplierChecked", vibMultiplierChecked);
    settings->setValue("vibMultiplierValue", vibMultiplierValue);
    settings->setValue("suckMultiplierChecked", suckMultiplierChecked);
    settings->setValue("suckMultiplierValue", suckMultiplierValue);
    Save();
    Load();
    LogHandler::Dialog("Due to a standards update your range settings\nhave been set to default for a new data structure.", XLogLevel::Information);
}

QString SettingsHandler::getSelectedTheme()
{
    QMutexLocker locker(&mutex);
    return selectedTheme;
}
QString SettingsHandler::getSelectedLibrary()
{
    QMutexLocker locker(&mutex);
    return selectedLibrary;
}
QString SettingsHandler::getSelectedFunscriptLibrary()
{
     QMutexLocker locker(&mutex);
    return selectedFunscriptLibrary;
}
int SettingsHandler::getSelectedDevice()
{
    QMutexLocker locker(&mutex);
    return selectedDevice;
}
QString SettingsHandler::getSerialPort()
{
    QMutexLocker locker(&mutex);
    return serialPort;
}
QString SettingsHandler::getServerAddress()
{
    QMutexLocker locker(&mutex);
    return serverAddress;
}
QString SettingsHandler::getServerPort()
{
    QMutexLocker locker(&mutex);
    return serverPort;
}
int SettingsHandler::getPlayerVolume()
{
    QMutexLocker locker(&mutex);
    return playerVolume;
}
int SettingsHandler::getoffSet()
{
    QMutexLocker locker(&mutex);
    return offSet;
}

int SettingsHandler::getChannelUserMin(QString channel)
{
    QMutexLocker locker(&mutex);
    return _availableAxis[channel].UserMin;
}
int SettingsHandler::getChannelUserMax(QString channel)
{
    QMutexLocker locker(&mutex);
    return _availableAxis[channel].UserMax;
}
void SettingsHandler::setChannelUserMin(QString channel, int value)
{
    QMutexLocker locker(&mutex);
    _availableAxis[channel].UserMin = value;
    if(channel == axisNames.Stroke)
        _liveXRangeMin = value;
}
void SettingsHandler::setChannelUserMax(QString channel, int value)
{
    QMutexLocker locker(&mutex);
    _availableAxis[channel].UserMax = value;
    if(channel == axisNames.Stroke)
        _liveXRangeMax = value;
}

float SettingsHandler::getMultiplierValue(QString channel)
{
    if(channel == axisNames.Twist)
    {
        return twistMultiplierValue;
    }
    else if(channel == axisNames.Sway)
    {
        return yMultiplierValue;
    }
    else if(channel == axisNames.Surge)
    {
        return zMultiplierValue;
    }
    else if(channel == axisNames.Roll)
    {
        return yRollMultiplierValue;
    }
    else if(channel == axisNames.Pitch)
    {
        return xRollMultiplierValue;
    }
    else if(channel == axisNames.Vib)
    {
        return vibMultiplierValue;
    }
    else if(channel == axisNames.Suck)
    {
        return suckMultiplierValue;
    }
    return 0.0;
}

bool SettingsHandler::getMultiplierChecked(QString channel)
{
    if(channel == axisNames.Twist)
    {
        return twistMultiplierChecked;
    }
    else if(channel == axisNames.Sway)
    {
        return yMultiplierChecked;
    }
    else if(channel == axisNames.Surge)
    {
        return zMultiplierChecked;
    }
    else if(channel == axisNames.Roll)
    {
        return yRollMultiplierChecked;
    }
    else if(channel == axisNames.Pitch)
    {
        return xRollMultiplierChecked;
    }
    else if(channel == axisNames.Vib)
    {
        return vibMultiplierChecked;
    }
    else if(channel == axisNames.Suck)
    {
        return suckMultiplierChecked;
    }
    return false;
}

void SettingsHandler::setMultiplierValue(QString channel, float value)
{
    if(channel == axisNames.Twist)
    {
        twistMultiplierValue = value;
    }
    else if(channel == axisNames.Sway)
    {
        yMultiplierValue = value;
    }
    else if(channel == axisNames.Surge)
    {
        zMultiplierValue = value;
    }
    else if(channel == axisNames.Roll)
    {
        yRollMultiplierValue = value;
    }
    else if(channel == axisNames.Pitch)
    {
        xRollMultiplierValue = value;
    }
    else if(channel == axisNames.Vib)
    {
        vibMultiplierValue = value;
    }
    else if(channel == axisNames.Suck)
    {
        suckMultiplierValue = value;
    }
}

void SettingsHandler::setMultiplierChecked(QString channel, bool value)
{
    if(channel == axisNames.Twist)
    {
        twistMultiplierChecked = value;
    }
    else if(channel == axisNames.Sway)
    {
        yMultiplierChecked = value;
    }
    else if(channel == axisNames.Surge)
    {
        zMultiplierChecked = value;
    }
    else if(channel == axisNames.Roll)
    {
        yRollMultiplierChecked = value;
    }
    else if(channel == axisNames.Pitch)
    {
        xRollMultiplierChecked = value;
    }
    else if(channel == axisNames.Vib)
    {
        vibMultiplierChecked = value;
    }
    else if(channel == axisNames.Suck)
    {
        suckMultiplierChecked = value;
    }
}

int SettingsHandler::getLibraryView()
{
    QMutexLocker locker(&mutex);
    return libraryView;
}
int SettingsHandler::getThumbSize()
{
    QMutexLocker locker(&mutex);
    if (libraryView == LibraryView::List)
    {
        return thumbSizeList;
    }
    return thumbSize;
}
void SettingsHandler::setThumbSize(int value)
{
    QMutexLocker locker(&mutex);
    if (libraryView == LibraryView::List)
    {
        thumbSizeList = value;
    }
    else
    {
        thumbSize = value;
    }
}
QSize SettingsHandler::getMaxThumbnailSize()
{
    QMutexLocker locker(&mutex);
    return _maxThumbnailSize;
}
int SettingsHandler::getVideoIncrement()
{
    QMutexLocker locker(&mutex);
    return videoIncrement;
}

QString SettingsHandler::getDeoDnlaFunscript(QString key)
{
    QMutexLocker locker(&mutex);
    if (deoDnlaFunscriptLookup.contains(key))
    {
        return deoDnlaFunscriptLookup[key].toString();
    }
    return nullptr;
}

bool SettingsHandler::getGamepadEnabled()
{
    QMutexLocker locker(&mutex);
    return _gamePadEnabled;
}

bool SettingsHandler::getInverseTcXL0()
{
    QMutexLocker locker(&mutex);
    return _inverseTcXL0;
}
bool SettingsHandler::getInverseTcXRollR2()
{
    QMutexLocker locker(&mutex);
    return _inverseTcXRollR2;
}
bool SettingsHandler::getInverseTcYRollR1()
{
    QMutexLocker locker(&mutex);
    return _inverseTcYRollR1;
}

int SettingsHandler::getGamepadSpeed()
{
    QMutexLocker locker(&mutex);
    return _gamepadSpeed;
}

void SettingsHandler::setGamepadSpeed(int value)
{
    QMutexLocker locker(&mutex);
    _gamepadSpeed = value;
}

int SettingsHandler::getGamepadSpeedIncrement()
{
    QMutexLocker locker(&mutex);
    return _gamepadSpeedStep;
}

void SettingsHandler::setGamepadSpeedStep(int value)
{
    QMutexLocker locker(&mutex);
    _gamepadSpeedStep = value;
}

int SettingsHandler::getLiveGamepadSpeed()
{
    QMutexLocker locker(&mutex);
    return _liveGamepadSpeed;
}

void SettingsHandler::setLiveGamepadSpeed(int value)
{
    QMutexLocker locker(&mutex);
    _liveGamepadSpeed = value;
}
void SettingsHandler::setLiveGamepadConnected(bool value)
{
    _liveGamepadConnected = value;
}
bool SettingsHandler::getLiveGamepadConnected()
{
    return _liveGamepadConnected;
}
void SettingsHandler::setXRangeStep(int value)
{
    QMutexLocker locker(&mutex);
    _xRangeStep = value;
}

int SettingsHandler::getXRangeStep()
{
    QMutexLocker locker(&mutex);
    return _xRangeStep;
}

void SettingsHandler::setLiveXRangeMin(int value)
{
    QMutexLocker locker(&mutex);
    _liveXRangeMin = value;
}

int SettingsHandler::getLiveXRangeMin()
{
    QMutexLocker locker(&mutex);
    return _liveXRangeMin;
}

void SettingsHandler::setLiveXRangeMax(int value)
{
    QMutexLocker locker(&mutex);
    _liveXRangeMax = value;
}

int SettingsHandler::getLiveXRangeMax()
{
    QMutexLocker locker(&mutex);
    return _liveXRangeMax;
}
void SettingsHandler::resetLiveXRange()
{
    QMutexLocker locker(&mutex);
    AxisNames axisNames;
    _liveXRangeMax = _availableAxis.value(axisNames.Stroke).UserMax;
    _liveXRangeMin = _availableAxis.value(axisNames.Stroke).UserMin;
}

void SettingsHandler::setLiveMultiplierEnabled(bool value)
{
    QMutexLocker locker(&mutex);
    _liveMultiplierEnabled = value;
}

bool SettingsHandler::getMultiplierEnabled()
{
    QMutexLocker locker(&mutex);
    return (_liveMultiplierEnabled && _multiplierEnabled) || _liveMultiplierEnabled;
}

void SettingsHandler::setMultiplierEnabled(bool value)
{
    QMutexLocker locker(&mutex);
    _multiplierEnabled = value;
    _liveMultiplierEnabled = value;
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


void SettingsHandler::setSelectedLibrarySortMode(int value)
{
    selectedLibrarySortMode = value;
}
int SettingsHandler::getSelectedLibrarySortMode()
{
    return selectedLibrarySortMode;
}

ChannelModel SettingsHandler::getAxis(QString axis)
{
    QMutexLocker locker(&mutex);
    return _availableAxis[axis];
}
QMap<QString, QString>*  SettingsHandler::getGamePadMap()
{
    return &_gamepadButtonMap;
}
QMap<QString, ChannelModel>*  SettingsHandler::getAvailableAxis()
{
    return &_availableAxis;
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
QString SettingsHandler::getDeoAddress()
{
    QMutexLocker locker(&mutex);
    return deoAddress;
}
QString SettingsHandler::getDeoPort()
{
    QMutexLocker locker(&mutex);
    return deoPort;
}
bool SettingsHandler::getDeoEnabled()
{
    QMutexLocker locker(&mutex);
    return deoEnabled;
}

void SettingsHandler::setWhirligigAddress(QString value)
{
    QMutexLocker locker(&mutex);
    whirligigAddress = value;
}
void SettingsHandler::setWhirligigPort(QString value)
{
    QMutexLocker locker(&mutex);
    whirligigPort = value;
}
void SettingsHandler::setWhirligigEnabled(bool value)
{
    QMutexLocker locker(&mutex);
    whirligigEnabled = value;
}
QString SettingsHandler::getWhirligigAddress()
{
    QMutexLocker locker(&mutex);
    return whirligigAddress;
}
QString SettingsHandler::getWhirligigPort()
{
    QMutexLocker locker(&mutex);
    return whirligigPort;
}
bool SettingsHandler::getWhirligigEnabled()
{
    QMutexLocker locker(&mutex);
    return whirligigEnabled;
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

void SettingsHandler::setLibraryView(int value)
{
    QMutexLocker locker(&mutex);
    libraryView = value;
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

void SettingsHandler::addAxis(ChannelModel channel)
{
    QMutexLocker locker(&mutex);
    _availableAxis.insert(channel.AxisName, channel);
}

void SettingsHandler::setDecoderPriority(QList<DecoderModel> value)
{
    decoderPriority = value;
}
QList<DecoderModel> SettingsHandler::getDecoderPriority()
{
    return decoderPriority;
}

void SettingsHandler::deleteAxis(QString axis)
{
    QMutexLocker locker(&mutex);
    _availableAxis.remove(axis);
}

void SettingsHandler::setGamePadMapButton(QString gamePadButton, QString axis)
{
    QMutexLocker locker(&mutex);
    _gamepadButtonMap[gamePadButton] = axis;
}

void SettingsHandler::setInverseTcXL0(bool value)
{
    QMutexLocker locker(&mutex);
    _inverseTcXL0 = value;
}
void SettingsHandler::setInverseTcXRollR2(bool value)
{
    QMutexLocker locker(&mutex);
    _inverseTcXRollR2 = value;
}
void SettingsHandler::setInverseTcYRollR1(bool value)
{
    QMutexLocker locker(&mutex);
    _inverseTcYRollR1  = value;
}

QList<int> SettingsHandler::getMainWindowSplitterPos()
{
    QMutexLocker locker(&mutex);
    return _mainWindowPos;
}
void SettingsHandler::setMainWindowSplitterPos(QList<int> value)
{
    QMutexLocker locker(&mutex);
    _mainWindowPos = value;
}
void SettingsHandler::SetupAvailableAxis()
{
    _availableAxis = {
        {axisNames.None, { "None", axisNames.None, axisNames.None, 1, 500, 999, 1, 500, 999, AxisDimension::None, AxisType::None, "" } },
        {axisNames.Stroke, { "Stroke", axisNames.Stroke, axisNames.Stroke, 1, 500, 999, 1, 500, 999, AxisDimension::Heave, AxisType::Range, "" } },
        {axisNames.StrokeDown, { "Stroke Down", axisNames.StrokeDown, axisNames.Stroke, 1, 500, 999, 1, 500, 999, AxisDimension::Heave, AxisType::HalfRange, "" } },
        {axisNames.StrokeUp, { "Stroke Up", axisNames.StrokeUp, axisNames.Stroke, 1, 500, 999, 1, 500, 999, AxisDimension::Heave, AxisType::HalfRange, "" } },
        {axisNames.Sway, { "Sway", axisNames.Sway, axisNames.Sway, 1, 500, 999, 1, 500, 999, AxisDimension::Sway, AxisType::Range, "sway" } },
        {axisNames.SwayLeft, { "Sway Left", axisNames.SwayLeft, axisNames.Sway, 1, 500, 999, 1, 500, 999, AxisDimension::Sway, AxisType::HalfRange, "" } },
        {axisNames.SwayRight, { "Sway Right", axisNames.SwayRight, axisNames.Sway, 1, 500, 999, 1, 500, 999, AxisDimension::Sway, AxisType::HalfRange, "" } },
        {axisNames.Surge, { "Surge", axisNames.Surge, axisNames.Surge, 1, 500, 999, 1, 500, 999, AxisDimension::Surge, AxisType::Range, "surge" } },
        {axisNames.SurgeBack, { "Surge Back", axisNames.SurgeBack, axisNames.Surge, 1, 500, 999, 1, 500, 999, AxisDimension::Surge, AxisType::HalfRange, "" } },
        {axisNames.SurgeForward, { "Surge Forward", axisNames.SurgeForward, axisNames.Surge, 1, 500, 999, 1, 500, 999, AxisDimension::Surge, AxisType::HalfRange, "" } },
        {axisNames.Pitch, { "Pitch", axisNames.Pitch, axisNames.Pitch, 1, 500, 999, 1, 500, 999, AxisDimension::Pitch, AxisType::Range, "pitch" } },
        {axisNames.PitchForward, { "Pitch Forward", axisNames.PitchForward, axisNames.Pitch, 1, 500, 999, 1, 500, 999, AxisDimension::Pitch, AxisType::HalfRange, "" } },
        {axisNames.PitchBack, { "Pitch Back", axisNames.PitchBack, axisNames.Pitch, 1, 500, 999, 1, 500, 999, AxisDimension::Pitch, AxisType::HalfRange, "" } },
        {axisNames.Roll, { "Roll", axisNames.Roll, axisNames.Roll, 1, 500, 999, 1, 500, 999, AxisDimension::Roll, AxisType::Range, "roll" } },
        {axisNames.RollLeft, { "Roll Left", axisNames.RollLeft, axisNames.Roll, 1, 500, 999, 1, 500, 999, AxisDimension::Roll, AxisType::HalfRange, "" } },
        {axisNames.RollRight, { "Roll Right", axisNames.RollRight, axisNames.Roll, 1, 500, 999, 1, 500, 999, AxisDimension::Roll, AxisType::HalfRange, "" } },
        {axisNames.Twist, { "Twist", axisNames.Twist, axisNames.Twist, 1, 500, 999, 1, 500, 999, AxisDimension::Yaw, AxisType::Range, "twist" } },
        {axisNames.TwistClockwise, { "Twist (CW)", axisNames.TwistClockwise, axisNames.Twist, 1, 500, 999, 1, 500, 999, AxisDimension::Yaw, AxisType::HalfRange, "" } },
        {axisNames.TwistCounterClockwise, { "Twist (CCW)", axisNames.TwistCounterClockwise, axisNames.Twist, 1, 500, 999, 1, 500, 999, AxisDimension::Yaw, AxisType::HalfRange, "" } },
        {axisNames.Vib, { "Vib", axisNames.Vib, axisNames.Vib, 1, 500, 999, 1, 500, 999, AxisDimension::None, AxisType::None, "vib" } },
        {axisNames.Lube, { "Lube", axisNames.Lube, axisNames.Lube, 1, 500, 999, 1, 500, 999, AxisDimension::None, AxisType::None, "lube" } },
        {axisNames.Suck, { "Suck", axisNames.Suck, axisNames.Suck, 1, 500, 999, 1, 500, 999, AxisDimension::None, AxisType::None, "suck" } }
    };
}

//private
void SettingsHandler::SetupGamepadButtonMap()
{
    _gamepadButtonMap = {
        { "None", axisNames.None },
        { gamepadAxisNames.LeftXAxis, axisNames.Twist },
        { gamepadAxisNames.LeftYAxis,  axisNames.Stroke },
        { gamepadAxisNames.RightYAxis ,  axisNames.Pitch  },
        { gamepadAxisNames.RightXAxis, axisNames.Roll  },
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
        { gamepadAxisNames.DPadUp, mediaActions.IncreaseXRange },
        { gamepadAxisNames.DPadDown, mediaActions.DecreaseXRange },
        { gamepadAxisNames.DPadLeft, mediaActions.TCodeSpeedDown },
        { gamepadAxisNames.DPadRight, mediaActions.TCodeSpeedUp },
        { gamepadAxisNames.RightAxisButton, mediaActions.ToggleAxisMultiplier },
        { gamepadAxisNames.LeftAxisButton, axisNames.None },
        { gamepadAxisNames.Center, axisNames.None },
        { gamepadAxisNames.Guide, axisNames.None }
    };
}
void SettingsHandler::SetupDecoderPriority()
{
    decoderPriority =
    {
        { "CUDA", true },
        { "D3D11", true },
        { "DXVA", true },
        { "FFmpeg", true },
        { "VAAPI", true },
        { "VideoToolbox", true }
    };
}
QSettings* SettingsHandler::settings;
QMutex SettingsHandler::mutex;
QList<int> SettingsHandler::_mainWindowPos;
QSize SettingsHandler::_maxThumbnailSize = {200, 200};
GamepadAxisNames SettingsHandler::gamepadAxisNames;
AxisNames SettingsHandler::axisNames;
MediaActions SettingsHandler::mediaActions;
QHash<QString, QVariant> SettingsHandler::deoDnlaFunscriptLookup;
QString SettingsHandler::selectedTheme;
QString SettingsHandler::selectedLibrary;
int SettingsHandler::selectedDevice;
int SettingsHandler::selectedLibrarySortMode;
int SettingsHandler::playerVolume;
int SettingsHandler::offSet;
bool SettingsHandler::yRollMultiplierChecked;
float SettingsHandler::yRollMultiplierValue;
bool SettingsHandler::zMultiplierChecked;
float SettingsHandler::zMultiplierValue;
bool SettingsHandler::yMultiplierChecked;
float SettingsHandler::yMultiplierValue;
bool SettingsHandler::xRollMultiplierChecked;
float SettingsHandler::xRollMultiplierValue;
bool SettingsHandler::twistMultiplierChecked;
float SettingsHandler::twistMultiplierValue;
bool SettingsHandler::vibMultiplierChecked;
float SettingsHandler::vibMultiplierValue;
bool SettingsHandler::suckMultiplierChecked;
float SettingsHandler::suckMultiplierValue;

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
int SettingsHandler::_liveGamepadSpeed;
bool SettingsHandler::_liveGamepadConnected;

int SettingsHandler::_xRangeStep;
int SettingsHandler::_liveXRangeMax;
int SettingsHandler::_liveXRangeMin;
bool SettingsHandler::_liveMultiplierEnabled = false;
bool SettingsHandler::_multiplierEnabled;

QString SettingsHandler::selectedFunscriptLibrary;
QString SettingsHandler::selectedFile;
QString SettingsHandler::serialPort;
QString SettingsHandler::serverAddress;
QString SettingsHandler::serverPort;
QString SettingsHandler::deoAddress;
QString SettingsHandler::deoPort;
bool SettingsHandler::deoEnabled;
QString SettingsHandler::whirligigAddress;
QString SettingsHandler::whirligigPort;
bool SettingsHandler::whirligigEnabled;
bool SettingsHandler::defaultReset = false;
bool SettingsHandler::disableSpeechToText;

QList<DecoderModel> SettingsHandler::decoderPriority;
