#include "settingshandler.h"

const QString SettingsHandler::TCodeVersion = "TCode v0.2";
const QString SettingsHandler::XTPVersion = "0.2581";
const float SettingsHandler::XTPVersionNum = 0.258f;

SettingsHandler::SettingsHandler()
{
}
SettingsHandler::~SettingsHandler()
{
    delete settings;
}
void SettingsHandler::Load(QSettings* settingsToLoadFrom)
{
    QMutexLocker locker(&mutex);
    QCoreApplication::setOrganizationName("cUrbSide prOd");
    QCoreApplication::setOrganizationDomain("https://www.patreon.com/Khrull");
    QCoreApplication::setApplicationName("XTPlayer");

    _appdataLocation = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if(_appdataLocation.isEmpty())
        _appdataLocation = QApplication::applicationDirPath();
    QDir dir(_appdataLocation);
    if (!dir.exists())
        dir.mkpath(_appdataLocation);
    if(settingsToLoadFrom == nullptr)
    {
        QFile settingsini(QApplication::applicationDirPath() + "/settings.ini");
        if(settingsini.exists())
        {
            settings = new QSettings("settings.ini", QSettings::Format::IniFormat);
        }
        else
        {
            settings = new QSettings("cUrbSide prOd", "XTPlayer");
        }
        settingsToLoadFrom = settings;
    }

    float currentVersion = settingsToLoadFrom->value("version").toFloat();
    if (currentVersion == 0)
    {
        locker.unlock();
        SetMapDefaults();
        SetupDecoderPriority();
        QList<QVariant> decoderVarient;
        foreach(auto decoder, decoderPriority)
        {
            decoderVarient.append(QVariant::fromValue(decoder));
        }
        settingsToLoadFrom->setValue("decoderPriority", decoderVarient);
    }
    locker.relock();
    selectedTheme = settingsToLoadFrom->value("selectedTheme").toString();
    selectedTheme = selectedTheme.isNull() ? QApplication::applicationDirPath() + "/themes/black-silver.css" : selectedTheme;
    selectedLibrary = settingsToLoadFrom->value("selectedLibrary").toString();
    _selectedThumbsDir = settingsToLoadFrom->value("selectedThumbsDir").toString();
    selectedDevice = settingsToLoadFrom->value("selectedDevice").toInt();
    playerVolume = settingsToLoadFrom->value("playerVolume").toInt();
    offSet = settingsToLoadFrom->value("offSet").toInt();
    selectedFunscriptLibrary = settingsToLoadFrom->value("selectedFunscriptLibrary").toString();
    serialPort = settingsToLoadFrom->value("serialPort").toString();
    serverAddress = settingsToLoadFrom->value("serverAddress").toString();
    serverAddress = serverAddress.isNull() ? "tcode.local" : serverAddress;
    serverPort = settingsToLoadFrom->value("serverPort").toString();
    serverPort = serverPort.isNull() ? "8000" : serverPort;
    deoAddress = settingsToLoadFrom->value("deoAddress").toString();
    deoAddress = deoAddress.isNull() ? "127.0.0.1" : deoAddress;
    deoPort = settingsToLoadFrom->value("deoPort").toString();
    deoPort = deoPort.isNull() ? "23554" : deoPort;
    deoEnabled = settingsToLoadFrom->value("deoEnabled").toBool();

    whirligigAddress = settingsToLoadFrom->value("whirligigAddress").toString();
    whirligigAddress = whirligigAddress.isNull() ? "127.0.0.1" : whirligigAddress;
    whirligigPort = settingsToLoadFrom->value("whirligigPort").toString();
    whirligigPort = whirligigPort.isNull() ? "2000" : whirligigPort;
    whirligigEnabled = settingsToLoadFrom->value("whirligigEnabled").toBool();

    libraryView = settingsToLoadFrom->value("libraryView").toInt();
    _librarySortMode = settingsToLoadFrom->value("selectedLibrarySortMode").toInt();
    thumbSize = settingsToLoadFrom->value("thumbSize").toInt();
    thumbSize = thumbSize == 0 ? 150 : thumbSize;
    thumbSizeList = settingsToLoadFrom->value("thumbSizeList").toInt();
    thumbSizeList = thumbSizeList == 0 ? 50 : thumbSizeList;
    videoIncrement = settingsToLoadFrom->value("videoIncrement").toInt();
    videoIncrement = videoIncrement == 0 ? 10 : videoIncrement;
    deoDnlaFunscriptLookup = settingsToLoadFrom->value("deoDnlaFunscriptLookup").toHash();

    _gamePadEnabled = settingsToLoadFrom->value("gamePadEnabled").toBool();
    _multiplierEnabled = settingsToLoadFrom->value("multiplierEnabled").toBool();
    _liveMultiplierEnabled = _multiplierEnabled;
    QVariantMap availableAxis = settingsToLoadFrom->value("availableAxis").toMap();
    foreach(auto axis, availableAxis.keys())
    {
        _availableAxis.insert(axis, availableAxis[axis].value<ChannelModel>());
        _funscriptLoaded.insert(axis, false);
    }
    _liveXRangeMax = _availableAxis[channelNames.Stroke].UserMax;
    _liveXRangeMin = _availableAxis[channelNames.Stroke].UserMin;
    QVariantMap gamepadButtonMap = settingsToLoadFrom->value("gamepadButtonMap").toMap();
    foreach(auto button, gamepadButtonMap.keys())
    {
        _gamepadButtonMap.insert(button, gamepadButtonMap[button].toString());
    }
    _inverseStroke = settingsToLoadFrom->value("inverseTcXL0").toBool();
    _inversePitch = settingsToLoadFrom->value("inverseTcXRollR2").toBool();
    _inverseRoll = settingsToLoadFrom->value("inverseTcYRollR1").toBool();
    _gamepadSpeed = settingsToLoadFrom->value("gamepadSpeed").toInt();
    _gamepadSpeed = _gamepadSpeed == 0 ? 1000 : _gamepadSpeed;
    _gamepadSpeedStep = settingsToLoadFrom->value("gamepadSpeedStep").toInt();
    _gamepadSpeedStep = _gamepadSpeedStep == 0 ? 500 : _gamepadSpeedStep;
    _liveGamepadSpeed = _gamepadSpeed;
    _xRangeStep = settingsToLoadFrom->value("xRangeStep").toInt();
    _xRangeStep = _xRangeStep == 0 ? 50 : _xRangeStep;
    disableSpeechToText = settingsToLoadFrom->value("disableSpeechToText").toBool();
    QList<QVariant> decoderPriorityvarient = settingsToLoadFrom->value("decoderPriority").toList();
    decoderPriority.clear();
    foreach(auto varient, decoderPriorityvarient)
    {
        decoderPriority.append(varient.value<DecoderModel>());
    }
    auto splitterSizes = settingsToLoadFrom->value("mainWindowPos").toList();
    foreach (auto splitterPos, splitterSizes)
    {
        _mainWindowPos.append(splitterPos.value<int>());
    }
    _libraryExclusions = settingsToLoadFrom->value("libraryExclusions").value<QList<QString>>();

    QVariantMap playlists = settingsToLoadFrom->value("playlists").toMap();
    foreach(auto playlist, playlists.keys())
    {
        _playlists.insert(playlist, playlists[playlist].value<QList<LibraryListItem>>());
    }

    _hashedPass = settingsToLoadFrom->value("userData").toString();
    if(currentVersion != 0 && currentVersion < 0.258f)
    {
        locker.unlock();
        MigrateLibraryMetaDataTo258();
    }
    else
    {
        QVariantHash libraryListItemMetaDatas = settingsToLoadFrom->value("libraryListItemMetaDatas").toHash();
        foreach(auto key, libraryListItemMetaDatas.keys())
        {
            _libraryListItemMetaDatas.insert(key, libraryListItemMetaDatas[key].value<LibraryListItemMetaData258>());
            foreach(auto bookmark, libraryListItemMetaDatas[key].value<LibraryListItemMetaData>().bookmarks)
                _libraryListItemMetaDatas[key].bookmarks.append(bookmark);
            foreach(auto funscript, libraryListItemMetaDatas[key].value<LibraryListItemMetaData>().funscripts)
                _libraryListItemMetaDatas[key].funscripts.append(funscript);
        }
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
    if(currentVersion != 0 && currentVersion < 0.25f)
    {
        locker.unlock();
        MigrateTo25();
    }
    if(currentVersion != 0 && currentVersion < 0.252f)
    {
        locker.unlock();
        MigrateTo252();
    }
}

void SettingsHandler::Save(QSettings* settingsToSaveTo)
{
    QMutexLocker locker(&mutex);
    if (!defaultReset)
    {
        if(settingsToSaveTo == nullptr) {
            settingsToSaveTo = settings;
        }
        settingsToSaveTo->setValue("version", XTPVersionNum);
        settingsToSaveTo->setValue("selectedLibrary", selectedLibrary);
        settingsToSaveTo->setValue("selectedTheme", selectedTheme);
        settingsToSaveTo->setValue("selectedThumbsDir", _selectedThumbsDir);
        settingsToSaveTo->setValue("selectedDevice", selectedDevice);
        settingsToSaveTo->setValue("playerVolume", playerVolume);
        settingsToSaveTo->setValue("offSet", offSet);
        settingsToSaveTo->setValue("selectedFunscriptLibrary", selectedFunscriptLibrary);
        settingsToSaveTo->setValue("serialPort", serialPort);
        settingsToSaveTo->setValue("serverAddress", serverAddress);
        settingsToSaveTo->setValue("serverPort", serverPort);
        settingsToSaveTo->setValue("deoAddress", deoAddress);
        settingsToSaveTo->setValue("deoPort", deoPort);
        settingsToSaveTo->setValue("deoEnabled", deoEnabled);
        settingsToSaveTo->setValue("whirligigAddress", whirligigAddress);
        settingsToSaveTo->setValue("whirligigPort", whirligigPort);
        settingsToSaveTo->setValue("whirligigEnabled", whirligigEnabled);

        settingsToSaveTo->setValue("libraryView", libraryView);
        settingsToSaveTo->setValue("selectedLibrarySortMode", _librarySortMode);

        settingsToSaveTo->setValue("thumbSize", thumbSize);
        settingsToSaveTo->setValue("thumbSizeList", thumbSizeList);
        settingsToSaveTo->setValue("videoIncrement", videoIncrement);

        settingsToSaveTo->setValue("deoDnlaFunscriptLookup", deoDnlaFunscriptLookup);

        settingsToSaveTo->setValue("gamePadEnabled", _gamePadEnabled);
        settingsToSaveTo->setValue("multiplierEnabled", _multiplierEnabled);

        QList<QVariant> decoderVarient;
        foreach(auto decoder, decoderPriority)
        {
            decoderVarient.append(QVariant::fromValue(decoder));
        }
        settingsToSaveTo->setValue("decoderPriority", decoderVarient);
        QVariantMap availableAxis;
        foreach(auto axis, _availableAxis.keys())
        {
            availableAxis.insert(axis, QVariant::fromValue(_availableAxis[axis]));
        }
        settingsToSaveTo->setValue("availableAxis", availableAxis);
        QVariantMap gamepadMap;
        foreach(auto button, _gamepadButtonMap.keys())
        {
            gamepadMap.insert(button, QVariant::fromValue(_gamepadButtonMap[button]));
        }
        settingsToSaveTo->setValue("gamepadButtonMap", gamepadMap);
        settingsToSaveTo->setValue("inverseTcXL0", _inverseStroke);
        settingsToSaveTo->setValue("inverseTcXRollR2", _inversePitch);
        settingsToSaveTo->setValue("inverseTcYRollR1", _inverseRoll);
        settingsToSaveTo->setValue("gamepadSpeed", _gamepadSpeed);
        settingsToSaveTo->setValue("gamepadSpeedStep", _gamepadSpeedStep);
        settingsToSaveTo->setValue("xRangeStep", _xRangeStep);
        settingsToSaveTo->setValue("disableSpeechToText", disableSpeechToText);
        QList<QVariant> splitterPos;
        foreach(auto pos, _mainWindowPos)
        {
            splitterPos.append(pos);
        }
        settingsToSaveTo->setValue("mainWindowPos", splitterPos);

        settingsToSaveTo->setValue("libraryExclusions", QVariant::fromValue(_libraryExclusions));

        QVariantMap playlists;
        foreach(auto playlist, _playlists.keys())
        {
            playlists.insert(playlist, QVariant::fromValue(_playlists[playlist]));
        }
        settingsToSaveTo->setValue("playlists", playlists);
        settingsToSaveTo->setValue("userData", _hashedPass);

        QVariantHash libraryListItemMetaDatas;
        foreach(auto libraryListItemMetaData, _libraryListItemMetaDatas.keys())
        {
            libraryListItemMetaDatas.insert(libraryListItemMetaData, QVariant::fromValue(_libraryListItemMetaDatas[libraryListItemMetaData]));
            foreach(auto bookmark, _libraryListItemMetaDatas[libraryListItemMetaData].bookmarks)
                libraryListItemMetaDatas[libraryListItemMetaData].value<LibraryListItemMetaData258>().bookmarks.append(bookmark);
            foreach(auto funscript, _libraryListItemMetaDatas[libraryListItemMetaData].funscripts)
                libraryListItemMetaDatas[libraryListItemMetaData].value<LibraryListItemMetaData258>().funscripts.append(funscript);
        }
        settingsToSaveTo->setValue("libraryListItemMetaDatas", libraryListItemMetaDatas);



        settingsToSaveTo->sync();
    }

}

void SettingsHandler::Export(QWidget* parent)
{
    QString selectedFile = QFileDialog::getSaveFileName(parent, QApplication::applicationDirPath() + "/Save settings ini", "settings_export.ini", "INI Files (*.ini)");
    if(!selectedFile.isEmpty())
    {
        QSettings* settingsExport = new QSettings(selectedFile, QSettings::Format::IniFormat);
        Save(settingsExport);
        delete settingsExport;
        LogHandler::Dialog("Settings saved to "+ selectedFile, XLogLevel::Information);
    }
}

void SettingsHandler::Import(QWidget* parent)
{
    QString selectedFile = QFileDialog::getOpenFileName(parent, "Choose settings ini", QApplication::applicationDirPath(), "INI Files (*.ini)");
    if(!selectedFile.isEmpty())
    {
        QSettings* settingsImport = new QSettings(selectedFile, QSettings::Format::IniFormat);
        Load(settingsImport);
        delete settingsImport;
        requestRestart(parent);
    }
}

void SettingsHandler::requestRestart(QWidget* parent)
{
    QMessageBox::question(parent, "Restart Application", "Changes will take effect on application restart.",
                                  "Restart now", 0);
    QApplication::quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

void SettingsHandler::Clear()
{
    QMutexLocker locker(&mutex);
    defaultReset = true;
    settings->clear();
}
void SettingsHandler::PersistSelectSettings()
{
    QVariantMap playlists;
    foreach(auto playlist, _playlists.keys())
    {
        playlists.insert(playlist, QVariant::fromValue(_playlists[playlist]));
    }
    settings->setValue("playlists", playlists);

    if(deoDnlaFunscriptLookup.count() > 0)
        settings->setValue("deoDnlaFunscriptLookup", deoDnlaFunscriptLookup);

    settings->sync();
}
void SettingsHandler::Default()
{
    Clear();
}

void SettingsHandler::SetMapDefaults()
{
    SetupAvailableAxis();
    SetupGamepadButtonMap();
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
    settings->setValue("version", 0.23f);
    SetupAvailableAxis();
    SetupDecoderPriority();
    Save();
    Load();
    LogHandler::Dialog("Due to a standards update your RANGE settings\nhave been set to default for a new data structure.", XLogLevel::Information);
}

void SettingsHandler::MigrateTo25()
{
    settings->setValue("version", 0.25f);
    Save();
    Load();
}

void SettingsHandler::MigrateTo252()
{
    settings->setValue("version", 0.252f);
    SetupAvailableAxis();
    Save();
    Load();
    LogHandler::Dialog("Due to a standards update your CHANNELS\nhave been set to default for a new data structure.\nPlease reset your Multiplier/Range settings before using.", XLogLevel::Information);
}
void SettingsHandler::MigrateLibraryMetaDataTo258()
{
    settings->setValue("version", 0.258f);
    QVariantHash libraryListItemMetaDatas = settings->value("libraryListItemMetaDatas").toHash();
    foreach(auto key, libraryListItemMetaDatas.keys())
    {
        auto libraryListItemMetaData = libraryListItemMetaDatas[key].value<LibraryListItemMetaData258>();
        QFile file(libraryListItemMetaData.libraryItemPath);
        if(file.exists())
        {
            _libraryListItemMetaDatas.insert(key, {
                                                     libraryListItemMetaData.libraryItemPath, // libraryItemPath
                                                     libraryListItemMetaData.lastPlayPosition, // lastPlayPosition
                                                     libraryListItemMetaData.lastLoopEnabled, // lastLoopEnabled
                                                     libraryListItemMetaData.lastLoopStart, // lastLoopStart
                                                     libraryListItemMetaData.lastLoopEnd, // lastLoopEnd
                                                     0, // offset
                                                     libraryListItemMetaData.moneyShotMillis, // moneyShotMillis
                                                     libraryListItemMetaData.bookmarks, // bookmarks
                                                     libraryListItemMetaData.funscripts
                                              });
            foreach(auto bookmark, libraryListItemMetaDatas[key].value<LibraryListItemMetaData258>().bookmarks)
                _libraryListItemMetaDatas[key].bookmarks.append(bookmark);
            foreach(auto funscript, libraryListItemMetaDatas[key].value<LibraryListItemMetaData258>().funscripts)
                _libraryListItemMetaDatas[key].funscripts.append(funscript);
        }
    }
    Save();
    auto fromDir = QApplication::applicationDirPath() + "/thumbs/";
    QDir oldThumbPath(fromDir);
    if(oldThumbPath.exists())
    {
        auto toDir = _appdataLocation + "/thumbs/";
        QDirIterator it(fromDir, QDirIterator::Subdirectories);
        QDir dir(fromDir);
        const int absSourcePathLength = dir.absoluteFilePath(fromDir).length();

        while (it.hasNext()){
            it.next();
            const auto fileInfo = it.fileInfo();
            if(!fileInfo.isHidden()) { //filters dot and dotdot
                const QString subPathStructure = fileInfo.absoluteFilePath().mid(absSourcePathLength);
                const QString constructedAbsolutePath = toDir + subPathStructure;

                if(fileInfo.isDir()){
                    //Create directory in target folder
                    dir.mkpath(constructedAbsolutePath);
                } else if(fileInfo.isFile()) {
                    //Copy File to target directory

                    //Remove file at target location, if it exists, or QFile::copy will fail
                    QFile::remove(constructedAbsolutePath);
                    QFile::copy(fileInfo.absoluteFilePath(), constructedAbsolutePath);
                }
            }
        }
    }
    Load();
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
QString SettingsHandler::getSelectedThumbsDir()
{
    auto customThumbDirExists  = !_selectedThumbsDir.isEmpty() && QFileInfo::exists(_selectedThumbsDir);
    return (customThumbDirExists ? _selectedThumbsDir + "/" : _appdataLocation + "/thumbs/");
}
void SettingsHandler::setSelectedThumbsDir(QWidget* parent)
{
    auto customThumbDirExists  = !_selectedThumbsDir.isEmpty() && QFileInfo::exists(_selectedThumbsDir);
    QString selectedDir = QFileDialog::getExistingDirectory(parent, QFileDialog::tr("Choose thumbnail storage directory"), customThumbDirExists ? _selectedThumbsDir : _appdataLocation + "/thumbs/", QFileDialog::ReadOnly);
    if (selectedDir != Q_NULLPTR)
    {
        _selectedThumbsDir = selectedDir;
        Save();
        requestRestart(parent);
    }
}
void SettingsHandler::setSelectedThumbsDirDefault(QWidget* parent)
{
    _selectedThumbsDir = nullptr;
    requestRestart(parent);
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
void SettingsHandler::setoffSet(int value)
{
    QMutexLocker locker(&mutex);
    offSet = value;
}

int SettingsHandler::getLiveOffSet()
{
    QMutexLocker locker(&mutex);
    return _liveOffset;
}

void SettingsHandler::setLiveOffset(int value)
{
    QMutexLocker locker(&mutex);
    _liveOffset = value;
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
    if(channel == channelNames.Stroke)
        _liveXRangeMin = value;
}
void SettingsHandler::setChannelUserMax(QString channel, int value)
{
    QMutexLocker locker(&mutex);
    _availableAxis[channel].UserMax = value;
    if(channel == channelNames.Stroke)
        _liveXRangeMax = value;
}

float SettingsHandler::getMultiplierValue(QString channel)
{
    QMutexLocker locker(&mutex);
    if(_availableAxis.contains(channel))
        return _availableAxis.value(channel).MultiplierValue;
    return 0.0;
}
void SettingsHandler::setMultiplierValue(QString channel, float value)
{
    QMutexLocker locker(&mutex);
    if(_availableAxis.contains(channel))
        _availableAxis[channel].MultiplierValue = value;
}

bool SettingsHandler::getMultiplierChecked(QString channel)
{
    QMutexLocker locker(&mutex);
    if(_availableAxis.contains(channel))
        return _availableAxis.value(channel).MultiplierEnabled;
    return false;
}
void SettingsHandler::setMultiplierChecked(QString channel, bool value)
{
    QMutexLocker locker(&mutex);
    if(_availableAxis.contains(channel))
        _availableAxis[channel].MultiplierEnabled = value;
}

bool SettingsHandler::getChannelInverseChecked(QString channel)
{
    QMutexLocker locker(&mutex);
    if(_availableAxis.contains(channel))
        return _availableAxis.value(channel).Inverted;
    return false;
}
void SettingsHandler::setChannelInverseChecked(QString channel, bool value)
{
    QMutexLocker locker(&mutex);
    if(_availableAxis.contains(channel))
        _availableAxis[channel].Inverted = value;
}

float SettingsHandler::getDamperValue(QString channel)
{
    QMutexLocker locker(&mutex);
    if(_availableAxis.contains(channel))
        return _availableAxis.value(channel).DamperValue;
    return 0.0;
}
void SettingsHandler::setDamperValue(QString channel, float value)
{
    QMutexLocker locker(&mutex);
    if(_availableAxis.contains(channel))
        _availableAxis[channel].DamperValue = value;
}

bool SettingsHandler::getDamperChecked(QString channel)
{
    QMutexLocker locker(&mutex);
    if(_availableAxis.contains(channel))
        return _availableAxis.value(channel).DamperEnabled;
    return false;
}
void SettingsHandler::setDamperChecked(QString channel, bool value)
{
    QMutexLocker locker(&mutex);
    if(_availableAxis.contains(channel))
        _availableAxis[channel].DamperEnabled = value;
}

bool SettingsHandler::getLinkToRelatedAxisChecked(QString channel)
{
    QMutexLocker locker(&mutex);
    if(_availableAxis.contains(channel))
        return _availableAxis.value(channel).LinkToRelatedMFS;
    return false;
}
void SettingsHandler::setLinkToRelatedAxisChecked(QString channel, bool value)
{
    QMutexLocker locker(&mutex);
    if(_availableAxis.contains(channel))
        _availableAxis[channel].LinkToRelatedMFS = value;
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
    return _inverseStroke;
}
bool SettingsHandler::getInverseTcXRollR2()
{
    QMutexLocker locker(&mutex);
    return _inversePitch;
}
bool SettingsHandler::getInverseTcYRollR1()
{
    QMutexLocker locker(&mutex);
    return _inverseRoll;
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
void SettingsHandler::setLiveActionPaused(bool value)
{
    _liveActionPaused = value;
}
bool SettingsHandler::getLiveActionPaused()
{
    return _liveActionPaused;
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
    TCodeChannels axisNames;
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


void SettingsHandler::setLibrarySortMode(int value)
{
    _librarySortMode = value;
}
LibrarySortMode SettingsHandler::getLibrarySortMode()
{
    return (LibrarySortMode)_librarySortMode;
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
        return _gamepadButtonMap[gamepadAxis];
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

void SettingsHandler::setLibraryView(int value)
{
    QMutexLocker locker(&mutex);
    libraryView = value;
}
LibraryView SettingsHandler::getLibraryView()
{
    QMutexLocker locker(&mutex);
    return (LibraryView)libraryView;
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
    _inverseStroke = value;
}
void SettingsHandler::setInverseTcXRollR2(bool value)
{
    QMutexLocker locker(&mutex);
    _inversePitch = value;
}
void SettingsHandler::setInverseTcYRollR1(bool value)
{
    QMutexLocker locker(&mutex);
    _inverseRoll  = value;
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

void SettingsHandler::addToLibraryExclusions(QString values)
{
    _libraryExclusions.append(values);
}
void SettingsHandler::removeFromLibraryExclusions(QList<int> indexes)
{
    foreach(auto index, indexes)
        _libraryExclusions.removeAt(index);
}
QList<QString> SettingsHandler::getLibraryExclusions()
{
    return _libraryExclusions;
}

QMap<QString, QList<LibraryListItem>> SettingsHandler::getPlaylists()
{
    return _playlists;
}
void SettingsHandler::setPlaylists(QMap<QString, QList<LibraryListItem>> value)
{
    _playlists = value;
}
void SettingsHandler::updatePlaylist(QString name, QList<LibraryListItem> value)
{
    _playlists.insert(name, value);
}
void SettingsHandler::addToPlaylist(QString name, LibraryListItem value)
{
    auto playlist = _playlists.value(name);
    playlist.append(value);
    _playlists.insert(name, playlist);
}
void SettingsHandler::addNewPlaylist(QString name)
{
    QList<LibraryListItem> playlist;
    _playlists.insert(name, playlist);
}
void SettingsHandler::deletePlaylist(QString name)
{
    _playlists.remove(name);
}

QString SettingsHandler::GetHashedPass()
{
    return _hashedPass;
}
void SettingsHandler::SetHashedPass(QString value)
{
    _hashedPass = value;
}

void SettingsHandler::SetupAvailableAxis()
{
    _availableAxis = {
        {channelNames.None, { "None", channelNames.None, channelNames.None, 0, 500, 999, 0, 500, 999, AxisDimension::None, AxisType::None, "", false, 0.01f, false, 0.2f, false, false, "" } },
        {channelNames.Stroke, { "Stroke", channelNames.Stroke, channelNames.Stroke, 0, 500, 999, 0, 500, 999, AxisDimension::Heave, AxisType::Range, "", false, 0.01f, false, 0.2f, false, false, channelNames.Twist } },
        {channelNames.StrokeDown, { "Stroke Down", channelNames.StrokeDown, channelNames.Stroke, 0, 500, 999, 0, 500, 999, AxisDimension::Heave, AxisType::HalfRange, "", false, 0.01f, false, 0.2f, false, false, channelNames.TwistCounterClockwise } },
        {channelNames.StrokeUp, { "Stroke Up", channelNames.StrokeUp, channelNames.Stroke, 0, 500, 999, 0, 500, 999, AxisDimension::Heave, AxisType::HalfRange, "", false, 0.01f, false, 0.2f, false, false, channelNames.TwistClockwise } },
        {channelNames.Sway, { "Sway", channelNames.Sway, channelNames.Sway, 0, 500, 999, 0, 500, 999, AxisDimension::Sway, AxisType::Range, "sway", false, 0.01f, false, 0.2f, false, false, channelNames.Roll } },
        {channelNames.SwayLeft, { "Sway Left", channelNames.SwayLeft, channelNames.Sway, 0, 500, 999, 0, 500, 999, AxisDimension::Sway, AxisType::HalfRange, "", false, 0.01f, false, 0.2f, false, false, channelNames.RollLeft } },
        {channelNames.SwayRight, { "Sway Right", channelNames.SwayRight, channelNames.Sway, 0, 500, 999, 0, 500, 999, AxisDimension::Sway, AxisType::HalfRange, "", false, 0.01f, false, 0.2f, false, false, channelNames.RollRight } },
        {channelNames.Surge, { "Surge", channelNames.Surge, channelNames.Surge, 0, 500, 999, 0, 500, 999, AxisDimension::Surge, AxisType::Range, "surge", false, 0.01f, false, 0.2f, false, false, channelNames.Pitch } },
        {channelNames.SurgeBack, { "Surge Back", channelNames.SurgeBack, channelNames.Surge, 0, 500, 999, 0, 500, 999, AxisDimension::Surge, AxisType::HalfRange, "", false, 0.01f, false, 0.2f, false, false, channelNames.PitchBack } },
        {channelNames.SurgeForward, { "Surge Forward", channelNames.SurgeForward, channelNames.Surge, 0, 500, 999, 0, 500, 999, AxisDimension::Surge, AxisType::HalfRange, "", false, 0.01f, false, 0.2f, false, false, channelNames.PitchForward } },
        {channelNames.Pitch, { "Pitch", channelNames.Pitch, channelNames.Pitch, 0, 500, 999, 0, 500, 999, AxisDimension::Pitch, AxisType::Range, "pitch", false, 0.01f, false, 0.2f, false, false, channelNames.Surge } },
        {channelNames.PitchForward, { "Pitch Forward", channelNames.PitchForward, channelNames.Pitch, 0, 500, 999, 0, 500, 999, AxisDimension::Pitch, AxisType::HalfRange, "", false, 0.01f, false, 0.2f, false, false, channelNames.SurgeForward } },
        {channelNames.PitchBack, { "Pitch Back", channelNames.PitchBack, channelNames.Pitch, 0, 500, 999, 0, 500, 999, AxisDimension::Pitch, AxisType::HalfRange, "", false, 0.01f, false, 0.2f, false, false, channelNames.SurgeBack } },
        {channelNames.Roll, { "Roll", channelNames.Roll, channelNames.Roll, 0, 500, 999, 0, 500, 999, AxisDimension::Roll, AxisType::Range, "roll", false, 0.01f, false, 0.2f, false, false, channelNames.Sway } },
        {channelNames.RollLeft, { "Roll Left", channelNames.RollLeft, channelNames.Roll, 0, 500, 999, 0, 500, 999, AxisDimension::Roll, AxisType::HalfRange, "", false, 0.01f, false, 0.2f, false, false, channelNames.SwayLeft } },
        {channelNames.RollRight, { "Roll Right", channelNames.RollRight, channelNames.Roll, 0, 500, 999, 0, 500, 999, AxisDimension::Roll, AxisType::HalfRange, "", false, 0.01f, false, 0.2f , false, false, channelNames.SwayRight } },
        {channelNames.Twist, { "Twist", channelNames.Twist, channelNames.Twist, 0, 500, 999, 0, 500, 999, AxisDimension::Yaw, AxisType::Range, "twist", false, 0.01f, false, 0.2f, false, false, channelNames.Stroke } },
        {channelNames.TwistClockwise, { "Twist (CW)", channelNames.TwistClockwise, channelNames.Twist, 0, 500, 999, 0, 500, 999, AxisDimension::Yaw, AxisType::HalfRange, "", false, 0.01f, false, 0.2f, false, false, channelNames.StrokeUp } },
        {channelNames.TwistCounterClockwise, { "Twist (CCW)", channelNames.TwistCounterClockwise, channelNames.Twist, 0, 500, 999, 0, 500, 999, AxisDimension::Yaw, AxisType::HalfRange, "", false, 0.01f, false, 0.2f, false, false, channelNames.StrokeDown } },
        {channelNames.Vib, { "Vib", channelNames.Vib, channelNames.Vib, 0, 0, 999, 0, 0, 999, AxisDimension::None, AxisType::Switch, "vib", false, 0.01f, false, 0.2f, false, false, channelNames.Stroke } },
        {channelNames.Lube, { "Lube", channelNames.Lube, channelNames.Lube, 0, 0, 999, 0, 0, 999, AxisDimension::None, AxisType::Switch, "lube", false, 0.01f, false, 0.2f, false, false, channelNames.Stroke } },
        {channelNames.Suck, { "Suck", channelNames.Suck, channelNames.Suck, 0, 500, 999, 0, 500, 999, AxisDimension::None, AxisType::Range, "suck", false, 0.01f, false, 0.2f, false, false, channelNames.Stroke } }
    };
}

//private
void SettingsHandler::SetupGamepadButtonMap()
{
    _gamepadButtonMap = {
        { "None", channelNames.None },
        { gamepadAxisNames.LeftXAxis, channelNames.Twist },
        { gamepadAxisNames.LeftYAxis,  channelNames.Stroke },
        { gamepadAxisNames.RightYAxis ,  channelNames.Pitch  },
        { gamepadAxisNames.RightXAxis, channelNames.Roll  },
        { gamepadAxisNames.RightTrigger, mediaActions.IncreaseXRange },
        { gamepadAxisNames.LeftTrigger, mediaActions.DecreaseXRange },
        { gamepadAxisNames.RightBumper, mediaActions.FastForward },
        { gamepadAxisNames.LeftBumper, mediaActions.Rewind },
        { gamepadAxisNames.Select, mediaActions.FullScreen },
        { gamepadAxisNames.Start, mediaActions.TogglePause },
        { gamepadAxisNames.X, mediaActions.TogglePauseAllDeviceActions },
        { gamepadAxisNames.Y, mediaActions.Loop },
        { gamepadAxisNames.B, mediaActions.Stop },
        { gamepadAxisNames.A, mediaActions.Mute },
        { gamepadAxisNames.DPadUp, mediaActions.IncreaseXRange },
        { gamepadAxisNames.DPadDown, mediaActions.DecreaseXRange },
        { gamepadAxisNames.DPadLeft, mediaActions.TCodeSpeedDown },
        { gamepadAxisNames.DPadRight, mediaActions.TCodeSpeedUp },
        { gamepadAxisNames.RightAxisButton, mediaActions.ToggleAxisMultiplier },
        { gamepadAxisNames.LeftAxisButton, channelNames.None },
        { gamepadAxisNames.Center, channelNames.None },
        { gamepadAxisNames.Guide, channelNames.None }
    };
}
void SettingsHandler::SetupDecoderPriority()
{
#if defined(Q_OS_WIN)
    decoderPriority =
    {
        { "CUDA", true },
        { "D3D11", true },
        { "DXVA", true },
        { "FFmpeg", true },
        { "VAAPI", true }
    };
#elif defined(Q_OS_MAC)
    decoderPriority =
    {
        { "VideoToolbox", true },
        { "FFmpeg", true },
        { "VAAPI", true },
        { "CUDA", true }
    };
#elif defined(Q_OS_LINUX)
    decoderPriority =
    {
        { "CUDA", true },
        { "FFmpeg", true },
        { "VAAPI", true }
    };
#endif
}

void SettingsHandler::setFunscriptLoaded(QString key, bool loaded)
{
    if (_funscriptLoaded.contains(key))
        _funscriptLoaded[key] = loaded;
}
bool SettingsHandler::getFunscriptLoaded(QString key)
{
    if (_funscriptLoaded.contains(key))
        return _funscriptLoaded[key];
    return false;
}

LibraryListItemMetaData258 SettingsHandler::getLibraryListItemMetaData(QString path)
{
    QMutexLocker locker(&mutex);
    if(_libraryListItemMetaDatas.contains(path))
    {
        return _libraryListItemMetaDatas.value(path);
    }
    //Default meta data
    QList<QString> funscripts;
    QList<Bookmark> bookmarks;
    _libraryListItemMetaDatas.insert(path, {
                                         path, // libraryItemPath
                                         -1, // lastPlayPosition
                                         false, // lastLoopEnabled
                                         -1, // lastLoopStart
                                         -1, // lastLoopEnd
                                         0, // offset
                                         -1, // moneyShotMillis
                                         bookmarks, // bookmarks
                                         funscripts
                                     });
    return _libraryListItemMetaDatas.value(path);
}

void SettingsHandler::updateLibraryListItemMetaData(LibraryListItemMetaData258 libraryListItemMetaData)
{
    QMutexLocker locker(&mutex);
    _libraryListItemMetaDatas.insert(libraryListItemMetaData.libraryItemPath, libraryListItemMetaData);
}

QSettings* SettingsHandler::settings;
QMutex SettingsHandler::mutex;
QString SettingsHandler::_appdataLocation;
QHash<QString, bool> SettingsHandler::_funscriptLoaded;
QList<int> SettingsHandler::_mainWindowPos;
QSize SettingsHandler::_maxThumbnailSize = {500, 500};
GamepadAxisNames SettingsHandler::gamepadAxisNames;
TCodeChannels SettingsHandler::channelNames;
MediaActions SettingsHandler::mediaActions;
QHash<QString, QVariant> SettingsHandler::deoDnlaFunscriptLookup;
QString SettingsHandler::selectedTheme;
QString SettingsHandler::selectedLibrary;
QString SettingsHandler::_selectedThumbsDir;
int SettingsHandler::selectedDevice;
int SettingsHandler::_librarySortMode;
int SettingsHandler::playerVolume;
int SettingsHandler::offSet;

int SettingsHandler::libraryView = LibraryView::Thumb;
int SettingsHandler::thumbSize = 175;
int SettingsHandler::thumbSizeList = 50;
int SettingsHandler::videoIncrement = 10;

bool SettingsHandler::_gamePadEnabled;
QMap<QString, QString> SettingsHandler::_gamepadButtonMap;
QMap<QString, ChannelModel> SettingsHandler::_availableAxis;
bool SettingsHandler::_inverseStroke;
bool SettingsHandler::_inversePitch;
bool SettingsHandler::_inverseRoll;
int SettingsHandler::_gamepadSpeed;
int SettingsHandler::_gamepadSpeedStep;
int SettingsHandler::_liveGamepadSpeed;
bool SettingsHandler::_liveGamepadConnected;
bool SettingsHandler::_liveActionPaused;
int SettingsHandler::_liveOffset;

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

QString SettingsHandler::_hashedPass;

QList<DecoderModel> SettingsHandler::decoderPriority;
QList<QString> SettingsHandler::_libraryExclusions;
QMap<QString, QList<LibraryListItem>> SettingsHandler::_playlists;
QHash<QString, LibraryListItemMetaData258> SettingsHandler::_libraryListItemMetaDatas;
