#include "settingshandler.h"

const QMap<TCodeVersion, QString> SettingsHandler::SupportedTCodeVersions = {
    {TCodeVersion::v2, "TCode v0.2"},
    {TCodeVersion::v3, "TCode v0.3"}
};
const QString SettingsHandler::XTPVersion = "0.271";
const float SettingsHandler::XTPVersionNum = 0.271f;

SettingsHandler::SettingsHandler(){}
SettingsHandler::~SettingsHandler()
{
    delete settings;
}
void SettingsHandler::Load(QSettings* settingsToLoadFrom)
{
    QMutexLocker locker(&mutex);
    QCoreApplication::setOrganizationName("cUrbSide prOd");
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

    _selectedTCodeVersion = (TCodeVersion)(settingsToLoadFrom->value("selectedTCodeVersion").toInt());
    TCodeChannelLookup::setSelectedTCodeVersion(_selectedTCodeVersion);
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
    selectedTheme = selectedTheme.isEmpty() ? QApplication::applicationDirPath() + "/themes/dark.css" : selectedTheme;
    selectedLibrary = settingsToLoadFrom->value("selectedLibrary").toString();
    _selectedThumbsDir = settingsToLoadFrom->value("selectedThumbsDir").toString();
    _hideWelcomeScreen = settingsToLoadFrom->value("hideWelcomeScreen").toBool();
    selectedDevice = settingsToLoadFrom->value("selectedDevice").toInt();
    playerVolume = settingsToLoadFrom->value("playerVolume").toInt();
    offSet = settingsToLoadFrom->value("offSet").toInt();
    _disableSerialTCodeValidation = settingsToLoadFrom->value("disableSerialTCodeValidation").toBool();
    selectedFunscriptLibrary = settingsToLoadFrom->value("selectedFunscriptLibrary").toString();
    serialPort = settingsToLoadFrom->value("serialPort").toString();
    serverAddress = settingsToLoadFrom->value("serverAddress").toString();
    serverAddress = serverAddress.isEmpty() ? "tcode.local" : serverAddress;
    serverPort = settingsToLoadFrom->value("serverPort").toString();
    serverPort = serverPort.isEmpty() ? "8000" : serverPort;
    deoAddress = settingsToLoadFrom->value("deoAddress").toString();
    deoAddress = deoAddress.isEmpty() ? "127.0.0.1" : deoAddress;
    deoPort = settingsToLoadFrom->value("deoPort").toString();
    deoPort = deoPort.isEmpty() ? "23554" : deoPort;
    deoEnabled = settingsToLoadFrom->value("deoEnabled").toBool();

    whirligigAddress = settingsToLoadFrom->value("whirligigAddress").toString();
    whirligigAddress = whirligigAddress.isEmpty() ? "127.0.0.1" : whirligigAddress;
    whirligigPort = settingsToLoadFrom->value("whirligigPort").toString();
    whirligigPort = whirligigPort.isEmpty() ? "2000" : whirligigPort;
    whirligigEnabled = settingsToLoadFrom->value("whirligigEnabled").toBool();

    _xtpWebSyncEnabled = settingsToLoadFrom->value("xtpWebSyncEnabled").toBool();

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
    _availableAxis.clear();
    _funscriptLoaded.clear();
    foreach(auto axis, availableAxis.keys())
    {
        _availableAxis.insert(axis, availableAxis[axis].value<ChannelModel>());
        _funscriptLoaded.insert(axis, false);
        if(!TCodeChannelLookup::ChannelExists(axis))
            TCodeChannelLookup::AddUserAxis(axis);
    }
    _liveXRangeMax = _availableAxis[TCodeChannelLookup::Stroke()].UserMax;
    _liveXRangeMin = _availableAxis[TCodeChannelLookup::Stroke()].UserMin;
    _liveXRangeMid = _availableAxis[TCodeChannelLookup::Stroke()].UserMid;
    QVariantMap gamepadButtonMap = settingsToLoadFrom->value("gamepadButtonMap").toMap();
    _gamepadButtonMap.clear();
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
    _disableVRScriptSelect = settingsToLoadFrom->value("disableVRScriptSelect").toBool();
    _disableNoScriptFound = settingsToLoadFrom->value("disableNoScriptFound").toBool();

    _skipToMoneyShotPlaysFunscript = settingsToLoadFrom->value("skipToMoneyShotPlaysFunscript").toBool();
    _skipToMoneyShotFunscript = settingsToLoadFrom->value("skipToMoneyShotFunscript").toString();
    _skipToMoneyShotSkipsVideo = settingsToLoadFrom->value("skipToMoneyShotSkipsVideo").toBool();
    _skipToMoneyShotStandAloneLoop = settingsToLoadFrom->value("skipToMoneyShotStandAloneLoop").toBool();

    _hideStandAloneFunscriptsInLibrary = settingsToLoadFrom->value("hideStandAloneFunscriptsInLibrary").toBool();
    _skipPlayingSTandAloneFunscriptsInLibrary = settingsToLoadFrom->value("skipPlayingSTandAloneFunscriptsInLibrary").toBool();

    _enableHttpServer = settingsToLoadFrom->value("enableHttpServer").toBool();
    _httpServerRoot = settingsToLoadFrom->value("httpServerRoot").toString();
    if(_httpServerRoot.isEmpty() || !QDir(_httpServerRoot).exists())
    {
#if defined(Q_OS_WIN)
        _httpServerRoot = "www";
#elif defined(Q_OS_MAC)
        _httpServerRoot = QApplication::applicationDirPath() + "/www";
#elif defined(Q_OS_LINUX)
        _httpServerRoot = QApplication::applicationDirPath() + "/www";
#endif
    }
    _vrLibrary = settingsToLoadFrom->value("vrLibrary").toString();
    _httpChunkSize = settingsToLoadFrom->value("httpChunkSize").toLongLong();
    if(!_httpChunkSize)
        _httpChunkSize = 5242880;
    _httpPort = settingsToLoadFrom->value("httpPort").toInt();
    if(!_httpPort)
        _httpPort = 80;

    _funscriptOffsetStep = settingsToLoadFrom->value("funscriptOffsetStep").toInt();
    if(!_funscriptOffsetStep)
        _funscriptOffsetStep = 100;
    _funscriptModifierStep = settingsToLoadFrom->value("funscriptModifierStep").toInt();
    if(!_funscriptModifierStep)
        _funscriptModifierStep = 5;

    QList<QVariant> decoderPriorityvarient = settingsToLoadFrom->value("decoderPriority").toList();
    decoderPriority.clear();
    foreach(auto varient, decoderPriorityvarient)
    {
        decoderPriority.append(varient.value<DecoderModel>());
    }

    _selectedVideoRenderer = (XVideoRenderer)settingsToLoadFrom->value("selectedVideoRenderer").toInt();

    auto splitterSizes = settingsToLoadFrom->value("mainWindowPos").toList();
    int i = 0;
    _mainWindowPos.clear();
    foreach (auto splitterPos, splitterSizes)
    {
        if(i==2)//Bandaid. Dont store over two.
            break;
        _mainWindowPos.append(splitterPos.value<int>());
        i++;
    }
    _libraryExclusions = settingsToLoadFrom->value("libraryExclusions").value<QList<QString>>();

    QVariantMap playlists = settingsToLoadFrom->value("playlists").toMap();
    _playlists.clear();
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
        _libraryListItemMetaDatas.clear();
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

    if(currentVersion != 0 )
    {
        if(currentVersion < 0.2f)
        {
            setupGamepadButtonMap();
        }
        if(currentVersion < 0.23f)
        {
            locker.unlock();
            MigrateTo23();
        }
        if(currentVersion < 0.25f)
        {
            locker.unlock();
            MigrateTo25();
        }
        if(currentVersion < 0.252f)
        {
            locker.unlock();
            MigrateTo252();
        }
        if(currentVersion < 0.2581f)
        {
            locker.unlock();
            setSelectedTCodeVersion();
        }
        if(currentVersion < 0.2615f)
        {
            locker.unlock();
            MigratrTo2615();
        }
        if(currentVersion < 0.263f)
        {
            locker.unlock();
            MigrateTo263();
        }
        if(currentVersion < 0.27f)
        {
            locker.unlock();
            settings->setValue("version", 0.27f);
            SetupDecoderPriority();
            Save();
            Load();
        }

    }
    settingsChangedEvent(false);
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
        settingsToSaveTo->setValue("selectedTCodeVersion", ((int)_selectedTCodeVersion));
        settingsToSaveTo->setValue("hideWelcomeScreen", ((int)_hideWelcomeScreen));
        settingsToSaveTo->setValue("selectedLibrary", selectedLibrary);
        settingsToSaveTo->setValue("selectedTheme", selectedTheme);
        settingsToSaveTo->setValue("selectedThumbsDir", _selectedThumbsDir);
        settingsToSaveTo->setValue("selectedDevice", selectedDevice);
        settingsToSaveTo->setValue("playerVolume", playerVolume);
        settingsToSaveTo->setValue("offSet", offSet);
        settingsToSaveTo->setValue("disableSerialTCodeValidation", _disableSerialTCodeValidation);
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
        settingsToSaveTo->setValue("xtpWebSyncEnabled", _xtpWebSyncEnabled);


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

        settingsToSaveTo->setValue("selectedVideoRenderer", (int)_selectedVideoRenderer);

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
        ;

        settingsToSaveTo->setValue("disableSpeechToText", disableSpeechToText);
        settingsToSaveTo->setValue("disableVRScriptSelect", _disableVRScriptSelect);
        settingsToSaveTo->setValue("disableNoScriptFound", _disableNoScriptFound);

        QList<QVariant> splitterPos;
        int i = 0;
        foreach(auto pos, _mainWindowPos)
        {
            if(i==2)//Bandaid. Dont store over two.
                break;
            splitterPos.append(pos);
            i++;
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

        settingsToSaveTo->setValue("skipToMoneyShotPlaysFunscript", _skipToMoneyShotPlaysFunscript);
        settingsToSaveTo->setValue("skipToMoneyShotFunscript", _skipToMoneyShotFunscript);
        settingsToSaveTo->setValue("skipToMoneyShotSkipsVideo", _skipToMoneyShotSkipsVideo);
        settingsToSaveTo->setValue("skipToMoneyShotStandAloneLoop", _skipToMoneyShotStandAloneLoop);

        settingsToSaveTo->setValue("hideStandAloneFunscriptsInLibrary", _hideStandAloneFunscriptsInLibrary);
        settingsToSaveTo->setValue("skipPlayingSTandAloneFunscriptsInLibrary", _skipPlayingSTandAloneFunscriptsInLibrary);

        settingsToSaveTo->setValue("enableHttpServer", _enableHttpServer);
        settingsToSaveTo->setValue("httpServerRoot", _httpServerRoot);
        settingsToSaveTo->setValue("vrLibrary", _vrLibrary);
        settingsToSaveTo->setValue("httpChunkSize", _httpChunkSize);
        settingsToSaveTo->setValue("httpPort", _httpPort);

        settingsToSaveTo->setValue("funscriptModifierStep", _funscriptModifierStep);
        settingsToSaveTo->setValue("funscriptOffsetStep", _funscriptOffsetStep);

        settingsToSaveTo->sync();

        settingsChangedEvent(false);
    }

}

void SettingsHandler::SaveLinkedFunscripts(QSettings* settingsToSaveTo)
{
    if(settingsToSaveTo == nullptr)
        settingsToSaveTo = settings;
    settingsToSaveTo->setValue("deoDnlaFunscriptLookup", deoDnlaFunscriptLookup);
    settingsToSaveTo->sync();
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
        Save();
        defaultReset = true;
        delete settingsImport;
        requestRestart(parent);
    }
}

void SettingsHandler::requestRestart(QWidget* parent)
{
    int value = QMessageBox::question(parent, "Restart Application", "Changes will take effect on application restart.",
                                  "Exit XTP", "Restart now", 0, 1);
    quit(value);
}
void SettingsHandler::askRestart(QWidget* parent, QString message)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(parent, "Restart?", message,
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
        quit(true);
}
void SettingsHandler::quit(bool restart)
{
    QApplication::quit();
    if(restart)
        QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}

void SettingsHandler::Clear()
{
    QMutexLocker locker(&mutex);
    defaultReset = true;
    settings->clear();
    settingsChangedEvent(true);
}

void SettingsHandler::settingsChangedEvent(bool dirty)
{
    _settingsChanged = dirty;
    emit instance().settingsChanged(dirty);
}
bool SettingsHandler::getSettingsChanged()
{
    return _settingsChanged;
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
    settingsChangedEvent(true);
}

void SettingsHandler::SetMapDefaults()
{
    SetChannelMapDefaults();
    SetGamepadMapDefaults();
}
void SettingsHandler::SetChannelMapDefaults()
{
    setupAvailableAxis();
    setSelectedTCodeVersion();
    QVariantHash availableAxis;
    foreach(auto axis, _availableAxis.keys())
    {
        QVariant axisVariant;
        axisVariant.setValue(_availableAxis[axis]);
        availableAxis.insert(axis, axisVariant);
    }
    settings->setValue("availableAxis", availableAxis);
    settingsChangedEvent(true);
}
void SettingsHandler::SetGamepadMapDefaults()
{
    setupGamepadButtonMap();
    QVariantHash gamepadMap;
    foreach(auto button, _gamepadButtonMap.keys())
    {
        QVariant buttonVariant;
        buttonVariant.setValue(_gamepadButtonMap[button]);
        gamepadMap.insert(button, buttonVariant);
    }
    settings->setValue("gamepadButtonMap", gamepadMap);
    settingsChangedEvent(true);
}

void SettingsHandler::MigrateTo23()
{
    settings->setValue("version", 0.23f);
    setupAvailableAxis();
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
    setupAvailableAxis();
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
void SettingsHandler::MigratrTo2615()
{
    settings->setValue("version", 0.2615f);
    setupAvailableAxis();
    setSelectedTCodeVersion();
    Save();
    Load();
    LogHandler::Dialog("Due to a standards update your CHANNEL SETTINGS\nhave been set to default for a new data structure.\nPlease reset your RANGES and MULTIPLIERS settings before using.", XLogLevel::Information);
}

void SettingsHandler::MigrateTo263() {

    settings->setValue("version", 0.263f);
    foreach(auto axis, _availableAxis.keys())
    {
        int max = _availableAxis.value(axis).UserMax;
        int min = _availableAxis.value(axis).UserMin;
        setChannelUserMid(axis, XMath::middle(min, max));
    }
    Save();
    Load();
}

QString SettingsHandler::getSelectedTCodeVersion()
{
    return SupportedTCodeVersions.value(_selectedTCodeVersion);
}

void SettingsHandler::setSelectedTCodeVersion(TCodeVersion key)
{
    if(_selectedTCodeVersion != key)
    {
        _selectedTCodeVersion = key;
        TCodeChannelLookup::setSelectedTCodeVersion(_selectedTCodeVersion);
        setSelectedTCodeVersion();
        emit instance().tcodeVersionChanged();
        settingsChangedEvent(true);
    }

}

void SettingsHandler::setSelectedTCodeVersion()
{
    foreach(auto axis, _availableAxis.keys())
    {
        if(_selectedTCodeVersion == TCodeVersion::v3)
        {
            _availableAxis[axis].Max = 9999;
            _availableAxis[axis].Mid = _availableAxis[axis].Type == AxisType::Switch ? 0 : 5000;
            _availableAxis[axis].UserMax = XMath::constrain(XMath::mapRange(_availableAxis[axis].UserMax, 0, 999, 0, 9999), 0 ,9999);
            _availableAxis[axis].UserMin = XMath::constrain(XMath::mapRange(_availableAxis[axis].UserMin, 0, 999, 0, 9999), 0 ,9999);
            _availableAxis[axis].UserMid = XMath::constrain(XMath::mapRange(_availableAxis[axis].UserMid, 0, 999, 0, 9999), 0 ,9999);
        }
        else
        {
            _availableAxis[axis].Max = 999;
            _availableAxis[axis].Mid = _availableAxis[axis].Type == AxisType::Switch ? 0 : 500;
            _availableAxis[axis].UserMax = XMath::constrain(XMath::mapRange(_availableAxis[axis].UserMax, 0, 9999, 0, 999), 0 ,999);
            _availableAxis[axis].UserMin = XMath::constrain(XMath::mapRange(_availableAxis[axis].UserMin, 0, 9999, 0, 999), 0 ,999);
            _availableAxis[axis].UserMid = XMath::constrain(XMath::mapRange(_availableAxis[axis].UserMid, 0, 9999, 0, 999), 0 ,999);
        }
    }
    _liveXRangeMax = _availableAxis.value(TCodeChannelLookup::Stroke()).UserMax;
    _liveXRangeMin = _availableAxis.value(TCodeChannelLookup::Stroke()).UserMin;
    _liveXRangeMid = _availableAxis.value(TCodeChannelLookup::Stroke()).UserMid;

//    ChannelModel suckMoreModel = { "Suck more", TCodeChannelLookup::SuckMore(), TCodeChannelLookup::Suck(), 0, 500, 999, 0, 500, 999, AxisDimension::None, AxisType::HalfRange, "suck", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::StrokeUp() };
//    ChannelModel suckLessModel = { "Suck less", TCodeChannelLookup::SuckLess(), TCodeChannelLookup::Suck(), 0, 500, 999, 0, 500, 999, AxisDimension::None, AxisType::HalfRange, "suck", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::StrokeDown() };
    if(_selectedTCodeVersion == TCodeVersion::v3)
    {
        auto v2ChannelMap = TCodeChannelLookup::TCodeVersionMap.value(TCodeVersion::v2);

        auto lubeV2Channel = v2ChannelMap.value(AxisNames::Lube);
        if(_availableAxis.contains(lubeV2Channel))
        {
            _availableAxis.insert(TCodeChannelLookup::Lube(), _availableAxis.value(lubeV2Channel));
            _availableAxis[TCodeChannelLookup::Lube()].AxisName = TCodeChannelLookup::Lube();
            _availableAxis[TCodeChannelLookup::Lube()].Channel = TCodeChannelLookup::Lube();
            _availableAxis.remove(lubeV2Channel);
        }

        auto suckV2Channel = v2ChannelMap.value(AxisNames::Suck);
        if(_availableAxis.contains(suckV2Channel))
        {
            _availableAxis.insert(TCodeChannelLookup::Suck(), _availableAxis.value(suckV2Channel));
            _availableAxis[TCodeChannelLookup::Suck()].AxisName = TCodeChannelLookup::Suck();
            _availableAxis[TCodeChannelLookup::Suck()].Channel = TCodeChannelLookup::Suck();
            _availableAxis.remove(suckV2Channel);
        }

        auto suckMoreV2Channel = v2ChannelMap.value(AxisNames::SuckMore);
        if(_availableAxis.contains(suckMoreV2Channel))
        {
            _availableAxis.insert(TCodeChannelLookup::SuckMore(), _availableAxis.value(suckMoreV2Channel));
            _availableAxis[TCodeChannelLookup::SuckMore()].AxisName = TCodeChannelLookup::SuckMore();
            _availableAxis[TCodeChannelLookup::SuckMore()].Channel = TCodeChannelLookup::Suck();
            _availableAxis.remove(suckMoreV2Channel);
        }

        auto suckLessV2Channel = v2ChannelMap.value(AxisNames::SuckLess);
        if(_availableAxis.contains(suckLessV2Channel))
        {
            _availableAxis.insert(TCodeChannelLookup::SuckLess(), _availableAxis.value(suckLessV2Channel));
            _availableAxis[TCodeChannelLookup::SuckLess()].AxisName = TCodeChannelLookup::SuckLess();
            _availableAxis[TCodeChannelLookup::SuckLess()].Channel = TCodeChannelLookup::Suck();
            _availableAxis.remove(suckLessV2Channel);
        }

        ChannelModel suctionPositionModel = { "Suck manual", TCodeChannelLookup::SuckPosition(), TCodeChannelLookup::SuckPosition(), 0, 5000, 9999, 0, 5000, 9999, AxisDimension::None, AxisType::Range, "suckManual", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::Stroke() };
        ChannelModel suctionMorePositionModel = { "Suck manual more ", TCodeChannelLookup::SuckMorePosition(), TCodeChannelLookup::SuckPosition(), 0, 5000, 9999, 0, 5000, 9999, AxisDimension::None, AxisType::HalfRange, "suckManual", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::StrokeUp() };
        ChannelModel suctionLessPositionModel = { "Suck manual less ", TCodeChannelLookup::SuckLessPosition(), TCodeChannelLookup::SuckPosition(), 0, 5000, 9999, 0, 5000, 9999, AxisDimension::None, AxisType::HalfRange, "suckManual", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::StrokeDown() };
       _availableAxis.insert(TCodeChannelLookup::SuckPosition(), suctionPositionModel);
       _availableAxis.insert(TCodeChannelLookup::SuckMorePosition(), suctionMorePositionModel);
       _availableAxis.insert(TCodeChannelLookup::SuckLessPosition(), suctionLessPositionModel);
    }
    else
    {
        auto v3ChannelMap = TCodeChannelLookup::TCodeVersionMap.value(TCodeVersion::v3);

        auto lubeV3Channel = v3ChannelMap.value(AxisNames::Lube);
        if(_availableAxis.contains(lubeV3Channel))
        {
            _availableAxis.insert(TCodeChannelLookup::Lube(), _availableAxis.value(lubeV3Channel));
            _availableAxis[TCodeChannelLookup::Lube()].AxisName = TCodeChannelLookup::Lube();
            _availableAxis[TCodeChannelLookup::Lube()].Channel = TCodeChannelLookup::Lube();
            _availableAxis.remove(lubeV3Channel);
        }

        auto suckV3Channel = v3ChannelMap.value(AxisNames::Suck);
        if(_availableAxis.contains(suckV3Channel))
        {
            _availableAxis.insert(TCodeChannelLookup::Suck(), _availableAxis.value(suckV3Channel));
            _availableAxis[TCodeChannelLookup::Suck()].AxisName = TCodeChannelLookup::Suck();
            _availableAxis[TCodeChannelLookup::Suck()].Channel = TCodeChannelLookup::Suck();
            _availableAxis.remove(suckV3Channel);
        }

        auto suckMoreV3Channel = v3ChannelMap.value(AxisNames::SuckMore);
        if(_availableAxis.contains(suckMoreV3Channel))
        {
            _availableAxis.insert(TCodeChannelLookup::SuckMore(), _availableAxis.value(suckMoreV3Channel));
            _availableAxis[TCodeChannelLookup::SuckMore()].AxisName = TCodeChannelLookup::SuckMore();
            _availableAxis[TCodeChannelLookup::SuckMore()].Channel = TCodeChannelLookup::Suck();
            _availableAxis.remove(suckMoreV3Channel);
        }

        auto suckLessV3Channel = v3ChannelMap.value(AxisNames::SuckLess);
        if(_availableAxis.contains(suckLessV3Channel))
        {
            _availableAxis.insert(TCodeChannelLookup::SuckLess(), _availableAxis.value(suckLessV3Channel));
            _availableAxis[TCodeChannelLookup::SuckLess()].AxisName = TCodeChannelLookup::SuckLess();
            _availableAxis[TCodeChannelLookup::SuckLess()].Channel = TCodeChannelLookup::Suck();
            _availableAxis.remove(suckLessV3Channel);
        }

        auto suckPositionV3Channel = v3ChannelMap.value(AxisNames::SuckPosition);
        _availableAxis.remove(suckPositionV3Channel);

        auto suckMorePositionV3Channel = v3ChannelMap.value(AxisNames::SuckMorePosition);
        _availableAxis.remove(suckMorePositionV3Channel);

        auto suckLessPositionV3Channel = v3ChannelMap.value(AxisNames::SuckLessPosition);
        _availableAxis.remove(suckLessPositionV3Channel);

    }
}

bool SettingsHandler::getHideWelcomeScreen()
{
    return _hideWelcomeScreen;
}
void SettingsHandler::setHideWelcomeScreen(bool value)
{
    _hideWelcomeScreen = value;
    settingsChangedEvent(true);
}

int SettingsHandler::getTCodePadding()
{
    return _selectedTCodeVersion == TCodeVersion::v3 ? 4 : 3;
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
    settingsChangedEvent(true);
}

bool SettingsHandler::getDisableSerialTCodeValidation()
{
    return _disableSerialTCodeValidation;
}
void SettingsHandler::setDisableSerialTCodeValidation(bool value)
{
    _disableSerialTCodeValidation = value;
    settingsChangedEvent(true);
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
    if(channel == TCodeChannelLookup::Stroke())
        _liveXRangeMin = value;
    settingsChangedEvent(true);
}
void SettingsHandler::setChannelUserMax(QString channel, int value)
{
    QMutexLocker locker(&mutex);
    _availableAxis[channel].UserMax = value;
    if(channel == TCodeChannelLookup::Stroke())
        _liveXRangeMax = value;
    settingsChangedEvent(true);
}
void SettingsHandler::setChannelUserMid(QString channel, int value)
{
    QMutexLocker locker(&mutex);
    _availableAxis[channel].UserMid = value;
    if(channel == TCodeChannelLookup::Stroke())
        _liveXRangeMid = value;
    settingsChangedEvent(true);
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
    settingsChangedEvent(true);
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
    settingsChangedEvent(true);
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
    settingsChangedEvent(true);
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
    settingsChangedEvent(true);
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
    settingsChangedEvent(true);
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
    settingsChangedEvent(true);
}

void SettingsHandler::setLinkToRelatedAxis(QString channel, QString linkedChannel)
{
    QMutexLocker locker(&mutex);
    if(_availableAxis.contains(channel))
        _availableAxis[channel].RelatedChannel = linkedChannel;
    settingsChangedEvent(true);
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
QHash<QString, QVariant> SettingsHandler::getDeoDnlaFunscripts()
{
    QMutexLocker locker(&mutex);
    return deoDnlaFunscriptLookup;
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
    settingsChangedEvent(true);
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
    settingsChangedEvent(true);
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
    settingsChangedEvent(true);
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
    _liveXRangeMid = XMath::middle(_liveXRangeMin, _liveXRangeMax);
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
    _liveXRangeMid = XMath::middle(_liveXRangeMin, _liveXRangeMax);
}

int SettingsHandler::getLiveXRangeMax()
{
    QMutexLocker locker(&mutex);
    return _liveXRangeMax;
}

void SettingsHandler::setLiveXRangeMid(int value)
{
    QMutexLocker locker(&mutex);
    _liveXRangeMid = value;
}

int SettingsHandler::getLiveXRangeMid()
{
    QMutexLocker locker(&mutex);
    return _liveXRangeMid;
}
void SettingsHandler::resetLiveXRange()
{
    QMutexLocker locker(&mutex);
    _liveXRangeMax = _availableAxis.value(TCodeChannelLookup::Stroke()).UserMax;
    _liveXRangeMin = _availableAxis.value(TCodeChannelLookup::Stroke()).UserMin;
    _liveXRangeMid = _availableAxis.value(TCodeChannelLookup::Stroke()).UserMid;
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
    settingsChangedEvent(true);
}

bool SettingsHandler::getDisableSpeechToText()
{
    return disableSpeechToText;
}
void SettingsHandler::setDisableSpeechToText(bool value)
{
    QMutexLocker locker(&mutex);
    disableSpeechToText = value;
    settingsChangedEvent(true);
}

bool SettingsHandler::getDisableNoScriptFound()
{
    return _disableNoScriptFound;
}
void SettingsHandler::setDisableNoScriptFound(bool value)
{
    QMutexLocker locker(&mutex);
    _disableNoScriptFound = value;
    settingsChangedEvent(true);
}

bool SettingsHandler::getDisableVRScriptSelect()
{
    return _disableVRScriptSelect;
}
void SettingsHandler::setDisableVRScriptSelect(bool value)
{
    QMutexLocker locker(&mutex);
    _disableVRScriptSelect = value;
    settingsChangedEvent(true);
}

void SettingsHandler::setLibrarySortMode(int value)
{
    _librarySortMode = value;
    settingsChangedEvent(true);
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
    settingsChangedEvent(true);
}
void SettingsHandler::setSelectedLibrary(QString value)
{
    QMutexLocker locker(&mutex);
    selectedLibrary = value;
    settingsChangedEvent(true);
}
void SettingsHandler::setSelectedFunscriptLibrary(QString value)
{
    QMutexLocker locker(&mutex);
    selectedFunscriptLibrary = value;
    settingsChangedEvent(true);
}
void SettingsHandler::setSelectedDevice(int value)
{
    QMutexLocker locker(&mutex);
    selectedDevice = value;
    settingsChangedEvent(true);
}
void SettingsHandler::setSerialPort(QString value)
{
    QMutexLocker locker(&mutex);
    serialPort = value;
    settingsChangedEvent(true);
}
void SettingsHandler::setServerAddress(QString value)
{
    QMutexLocker locker(&mutex);
    serverAddress = value;
    settingsChangedEvent(true);
}
void SettingsHandler::setServerPort(QString value)
{
    QMutexLocker locker(&mutex);
    serverPort = value;
    settingsChangedEvent(true);
}

void SettingsHandler::setDeoAddress(QString value)
{
    QMutexLocker locker(&mutex);
    deoAddress = value;
    settingsChangedEvent(true);
}
void SettingsHandler::setDeoPort(QString value)
{
    QMutexLocker locker(&mutex);
    deoPort = value;
    settingsChangedEvent(true);
}
void SettingsHandler::setDeoEnabled(bool value)
{
    QMutexLocker locker(&mutex);
    deoEnabled = value;
    settingsChangedEvent(true);
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
    settingsChangedEvent(true);
}
void SettingsHandler::setWhirligigPort(QString value)
{
    QMutexLocker locker(&mutex);
    whirligigPort = value;
    settingsChangedEvent(true);
}
void SettingsHandler::setWhirligigEnabled(bool value)
{
    QMutexLocker locker(&mutex);
    whirligigEnabled = value;
    settingsChangedEvent(true);
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

void SettingsHandler::setXTPWebSyncEnabled(bool value)
{
    QMutexLocker locker(&mutex);
    _xtpWebSyncEnabled = value;
}
bool SettingsHandler::getXTPWebSyncEnabled()
{
    QMutexLocker locker(&mutex);
    return _xtpWebSyncEnabled;
}

void SettingsHandler::setPlayerVolume(int value)
{
    QMutexLocker locker(&mutex);
    playerVolume = value;
    settingsChangedEvent(true);
}

void SettingsHandler::setLibraryView(int value)
{
    QMutexLocker locker(&mutex);
    libraryView = value;
    settingsChangedEvent(true);
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
    settingsChangedEvent(true);
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
    settingsChangedEvent(true);
}

void SettingsHandler::setLinkedVRFunscript(QString key, QString value)
{
    QMutexLocker locker(&mutex);
    deoDnlaFunscriptLookup[key] = value;// Should be saved on edit
}
void SettingsHandler::removeLinkedVRFunscript(QString key)
{
    QMutexLocker locker(&mutex);
    deoDnlaFunscriptLookup.remove(key);// Should be saved on edit
}

void SettingsHandler::setGamepadEnabled(bool value)
{
    QMutexLocker locker(&mutex);
    _gamePadEnabled = value;
    settingsChangedEvent(true);
}

void SettingsHandler::setAxis(QString axis, ChannelModel channel)
{
    QMutexLocker locker(&mutex);
    _availableAxis[axis] = channel;
    settingsChangedEvent(true);
}

void SettingsHandler::addAxis(ChannelModel channel)
{
    QMutexLocker locker(&mutex);
    _availableAxis.insert(channel.AxisName, channel);
    if(!TCodeChannelLookup::ChannelExists(channel.AxisName))
        TCodeChannelLookup::AddUserAxis(channel.AxisName);
    settingsChangedEvent(true);
}

void SettingsHandler::setDecoderPriority(QList<DecoderModel> value)
{
    decoderPriority = value;
    settingsChangedEvent(true);
}
QList<DecoderModel> SettingsHandler::getDecoderPriority()
{
    return decoderPriority;
}
void SettingsHandler::setSelectedVideoRenderer(XVideoRenderer value)
{
    _selectedVideoRenderer = value;
    settingsChangedEvent(true);
}
XVideoRenderer SettingsHandler::getSelectedVideoRenderer()
{
    return _selectedVideoRenderer;
}
void SettingsHandler::deleteAxis(QString axis)
{
    QMutexLocker locker(&mutex);
    _availableAxis.remove(axis);
    settingsChangedEvent(true);
}

void SettingsHandler::setGamePadMapButton(QString gamePadButton, QString axis)
{
    QMutexLocker locker(&mutex);
    _gamepadButtonMap[gamePadButton] = axis;
    settingsChangedEvent(true);
}

void SettingsHandler::setInverseTcXL0(bool value)
{
    QMutexLocker locker(&mutex);
    _inverseStroke = value;
    settingsChangedEvent(true);
}
void SettingsHandler::setInverseTcXRollR2(bool value)
{
    QMutexLocker locker(&mutex);
    _inversePitch = value;
    settingsChangedEvent(true);
}
void SettingsHandler::setInverseTcYRollR1(bool value)
{
    QMutexLocker locker(&mutex);
    _inverseRoll  = value;
    settingsChangedEvent(true);
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
    settingsChangedEvent(true);
}

void SettingsHandler::addToLibraryExclusions(QString values)
{
    _libraryExclusions.append(values);
    settingsChangedEvent(true);
}
void SettingsHandler::removeFromLibraryExclusions(QList<int> indexes)
{
    foreach(auto index, indexes)
        _libraryExclusions.removeAt(index);
    settingsChangedEvent(true);
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
    settingsChangedEvent(true);
}
void SettingsHandler::updatePlaylist(QString name, QList<LibraryListItem> value)
{
    _playlists.insert(name, value);
    settingsChangedEvent(true);
}
void SettingsHandler::addToPlaylist(QString name, LibraryListItem value)
{
    auto playlist = _playlists.value(name);
    playlist.append(value);
    _playlists.insert(name, playlist);
    settingsChangedEvent(true);
}
void SettingsHandler::addNewPlaylist(QString name)
{
    QList<LibraryListItem> playlist;
    _playlists.insert(name, playlist);
    settingsChangedEvent(true);
}
void SettingsHandler::deletePlaylist(QString name)
{
    _playlists.remove(name);
    settingsChangedEvent(true);
}

QString SettingsHandler::GetHashedPass()
{
    return _hashedPass;
}
void SettingsHandler::SetHashedPass(QString value)
{
    _hashedPass = value;
    settingsChangedEvent(true);
}

void SettingsHandler::setupAvailableAxis()
{
    TCodeChannelLookup::setSelectedTCodeVersion(_selectedTCodeVersion);
    _availableAxis = {
        {TCodeChannelLookup::None(), { "None", TCodeChannelLookup::None(), TCodeChannelLookup::None(), 0, 500, 999, 0, 500, 999, AxisDimension::None, AxisType::None, "", false, 2.50f, false, 1.0f, false, false, "" } },
        {TCodeChannelLookup::Stroke(), { "Stroke", TCodeChannelLookup::Stroke(), TCodeChannelLookup::Stroke(), 0, 500, 999, 0, 500, 999, AxisDimension::Heave, AxisType::Range, "", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::Twist() } },
        {TCodeChannelLookup::StrokeDown(), { "Stroke Down", TCodeChannelLookup::StrokeDown(), TCodeChannelLookup::Stroke(), 0, 500, 999, 0, 500, 999, AxisDimension::Heave, AxisType::HalfRange, "", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::TwistCounterClockwise() } },
        {TCodeChannelLookup::StrokeUp(), { "Stroke Up", TCodeChannelLookup::StrokeUp(), TCodeChannelLookup::Stroke(), 0, 500, 999, 0, 500, 999, AxisDimension::Heave, AxisType::HalfRange, "", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::TwistClockwise() } },
        {TCodeChannelLookup::Sway(), { "Sway", TCodeChannelLookup::Sway(), TCodeChannelLookup::Sway(), 0, 500, 999, 0, 500, 999, AxisDimension::Sway, AxisType::Range, "sway", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::Roll() } },
        {TCodeChannelLookup::SwayLeft(), { "Sway Left", TCodeChannelLookup::SwayLeft(), TCodeChannelLookup::Sway(), 0, 500, 999, 0, 500, 999, AxisDimension::Sway, AxisType::HalfRange, "", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::RollLeft() } },
        {TCodeChannelLookup::SwayRight(), { "Sway Right", TCodeChannelLookup::SwayRight(), TCodeChannelLookup::Sway(), 0, 500, 999, 0, 500, 999, AxisDimension::Sway, AxisType::HalfRange, "", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::RollRight() } },
        {TCodeChannelLookup::Surge(), { "Surge", TCodeChannelLookup::Surge(), TCodeChannelLookup::Surge(), 0, 500, 999, 0, 500, 999, AxisDimension::Surge, AxisType::Range, "surge", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::Pitch() } },
        {TCodeChannelLookup::SurgeBack(), { "Surge Back", TCodeChannelLookup::SurgeBack(), TCodeChannelLookup::Surge(), 0, 500, 999, 0, 500, 999, AxisDimension::Surge, AxisType::HalfRange, "", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::PitchBack() } },
        {TCodeChannelLookup::SurgeForward(), { "Surge Forward", TCodeChannelLookup::SurgeForward(), TCodeChannelLookup::Surge(), 0, 500, 999, 0, 500, 999, AxisDimension::Surge, AxisType::HalfRange, "", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::PitchForward() } },
        {TCodeChannelLookup::Pitch(), { "Pitch", TCodeChannelLookup::Pitch(), TCodeChannelLookup::Pitch(), 0, 500, 999, 0, 500, 999, AxisDimension::Pitch, AxisType::Range, "pitch", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::Surge() } },
        {TCodeChannelLookup::PitchForward(), { "Pitch Forward", TCodeChannelLookup::PitchForward(), TCodeChannelLookup::Pitch(), 0, 500, 999, 0, 500, 999, AxisDimension::Pitch, AxisType::HalfRange, "", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::SurgeForward() } },
        {TCodeChannelLookup::PitchBack(), { "Pitch Back", TCodeChannelLookup::PitchBack(), TCodeChannelLookup::Pitch(), 0, 500, 999, 0, 500, 999, AxisDimension::Pitch, AxisType::HalfRange, "", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::SurgeBack() } },
        {TCodeChannelLookup::Roll(), { "Roll", TCodeChannelLookup::Roll(), TCodeChannelLookup::Roll(), 0, 500, 999, 0, 500, 999, AxisDimension::Roll, AxisType::Range, "roll", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::Sway() } },
        {TCodeChannelLookup::RollLeft(), { "Roll Left", TCodeChannelLookup::RollLeft(), TCodeChannelLookup::Roll(), 0, 500, 999, 0, 500, 999, AxisDimension::Roll, AxisType::HalfRange, "", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::SwayLeft() } },
        {TCodeChannelLookup::RollRight(), { "Roll Right", TCodeChannelLookup::RollRight(), TCodeChannelLookup::Roll(), 0, 500, 999, 0, 500, 999, AxisDimension::Roll, AxisType::HalfRange, "", false, 0.01f, false, 0.2f , false, false, TCodeChannelLookup::SwayRight() } },
        {TCodeChannelLookup::Twist(), { "Twist", TCodeChannelLookup::Twist(), TCodeChannelLookup::Twist(), 0, 500, 999, 0, 500, 999, AxisDimension::Yaw, AxisType::Range, "twist", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::Stroke() } },
        {TCodeChannelLookup::TwistClockwise(), { "Twist (CW)", TCodeChannelLookup::TwistClockwise(), TCodeChannelLookup::Twist(), 0, 500, 999, 0, 500, 999, AxisDimension::Yaw, AxisType::HalfRange, "", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::StrokeUp() } },
        {TCodeChannelLookup::TwistCounterClockwise(), { "Twist (CCW)", TCodeChannelLookup::TwistCounterClockwise(), TCodeChannelLookup::Twist(), 0, 500, 999, 0, 500, 999, AxisDimension::Yaw, AxisType::HalfRange, "", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::StrokeDown() } },
        {TCodeChannelLookup::Vib(), { "Vib", TCodeChannelLookup::Vib(), TCodeChannelLookup::Vib(), 0, 0, 999, 0, 0, 999, AxisDimension::None, AxisType::Switch, "vib", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::Stroke() } },
        {TCodeChannelLookup::Lube(), { "Lube", TCodeChannelLookup::Lube(), TCodeChannelLookup::Lube(), 0, 0, 999, 0, 0, 999, AxisDimension::None, AxisType::Switch, "lube", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::Stroke() } },
        {TCodeChannelLookup::Suck(), { "Suck", TCodeChannelLookup::Suck(), TCodeChannelLookup::Suck(), 0, 500, 999, 0, 500, 999, AxisDimension::None, AxisType::Range, "suck", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::Stroke() } },
        {TCodeChannelLookup::SuckMore(), { "Suck more", TCodeChannelLookup::SuckMore(), TCodeChannelLookup::Suck(), 0, 500, 999, 0, 500, 999, AxisDimension::None, AxisType::HalfRange, "suck", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::StrokeUp() } },
        {TCodeChannelLookup::SuckLess(), { "Suck less", TCodeChannelLookup::SuckLess(), TCodeChannelLookup::Suck(), 0, 500, 999, 0, 500, 999, AxisDimension::None, AxisType::HalfRange, "suck", false, 2.50f, false, 1.0f, false, false, TCodeChannelLookup::StrokeDown() } }
    };
}

//private
void SettingsHandler::setupGamepadButtonMap()
{
    _gamepadButtonMap = {
        { "None", TCodeChannelLookup::None() },
        { gamepadAxisNames.LeftXAxis, TCodeChannelLookup::Twist() },
        { gamepadAxisNames.LeftYAxis, TCodeChannelLookup::Stroke() },
        { gamepadAxisNames.RightYAxis, TCodeChannelLookup::Pitch()  },
        { gamepadAxisNames.RightXAxis, TCodeChannelLookup::Roll()  },
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
        { gamepadAxisNames.LeftAxisButton, TCodeChannelLookup::None() },
        { gamepadAxisNames.Center, TCodeChannelLookup::None() },
        { gamepadAxisNames.Guide, TCodeChannelLookup::None() }
    };
}

void SettingsHandler::SetupDecoderPriority()
{

    if(decoderPriority.length() > 0)
        decoderPriority.clear();
    QStringList decs;
    foreach (int id, QtAV::VideoDecoder::registered()) {
        decoderPriority.append({QString::fromLatin1(QtAV::VideoDecoder::name(id)), true });
    }
//#if defined(Q_OS_WIN)
//    decoderPriority =
//    {
//        { "CUDA", true },
//        { "D3D11", true },
//        { "DXVA", true },
//        { "FFmpeg", true },
//        { "VAAPI", true }
//    };
//#elif defined(Q_OS_MAC)
//    decoderPriority =
//    {
//        { "VideoToolbox", true },
//        { "FFmpeg", true },
//        { "VAAPI", true },
//        { "CUDA", true }
//    };
//#elif defined(Q_OS_LINUX)
//    decoderPriority =
//    {
//        { "CUDA", true },
//        { "FFmpeg", true },
//        { "VAAPI", true }
//    };
//#endif
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


bool SettingsHandler::getSkipToMoneyShotPlaysFunscript()
{
    return _skipToMoneyShotPlaysFunscript;
}
void SettingsHandler::setSkipToMoneyShotPlaysFunscript(bool value)
{
    _skipToMoneyShotPlaysFunscript = value;
    settingsChangedEvent(true);
}

QString SettingsHandler::getSkipToMoneyShotFunscript()
{
    return _skipToMoneyShotFunscript;
}
void SettingsHandler::setSkipToMoneyShotFunscript(QString value)
{
    _skipToMoneyShotFunscript = value;
    settingsChangedEvent(true);
}

bool SettingsHandler::getSkipToMoneyShotSkipsVideo()
{
    return _skipToMoneyShotSkipsVideo;
}
void SettingsHandler::setSkipToMoneyShotSkipsVideo(bool value)
{
    _skipToMoneyShotSkipsVideo = value;
    settingsChangedEvent(true);
}
bool SettingsHandler::getSkipToMoneyShotStandAloneLoop()
{
    return _skipToMoneyShotStandAloneLoop;
}
void SettingsHandler::setSkipToMoneyShotStandAloneLoop(bool value)
{
    _skipToMoneyShotStandAloneLoop = value;
    settingsChangedEvent(true);
}
bool SettingsHandler::getHideStandAloneFunscriptsInLibrary()
{
    return _hideStandAloneFunscriptsInLibrary;
}
void SettingsHandler::setHideStandAloneFunscriptsInLibrary(bool value)
{
    _hideStandAloneFunscriptsInLibrary = value;
    settingsChangedEvent(true);
}

bool SettingsHandler::getSkipPlayingStandAloneFunscriptsInLibrary()
{
    return _skipPlayingSTandAloneFunscriptsInLibrary;
}
void SettingsHandler::setSkipPlayingStandAloneFunscriptsInLibrary(bool value)
{
    _skipPlayingSTandAloneFunscriptsInLibrary = value;
    settingsChangedEvent(true);
}

bool SettingsHandler::getEnableHttpServer()
{
    return _enableHttpServer;
}

void SettingsHandler::setEnableHttpServer(bool enable)
{
    QMutexLocker locker(&mutex);
    _enableHttpServer = enable;
    settingsChangedEvent(true);
}

QString SettingsHandler::getHttpServerRoot()
{
    return _httpServerRoot;
}
void SettingsHandler::setHttpServerRoot(QString value)
{
    QMutexLocker locker(&mutex);
    _httpServerRoot = value;
    settingsChangedEvent(true);
}

qint64 SettingsHandler::getHTTPChunkSize()
{
    return _httpChunkSize;
}
void SettingsHandler::setHTTPChunkSize(qint64 value)
{
    QMutexLocker locker(&mutex);
    _httpChunkSize = value;
    settingsChangedEvent(true);
}

int SettingsHandler::getHTTPPort()
{
    return _httpPort;
}
void SettingsHandler::setHTTPPort(int value)
{
    QMutexLocker locker(&mutex);
    _httpPort = value;
    settingsChangedEvent(true);
}

QString SettingsHandler::getVRLibrary()
{
    return _vrLibrary;
}
void SettingsHandler::setVRLibrary(QString value)
{
    QMutexLocker locker(&mutex);
    _vrLibrary = value;
    settingsChangedEvent(true);
}


void  SettingsHandler::setFunscriptModifierStep(int value)
{
    QMutexLocker locker(&mutex);
    _funscriptModifierStep = value;
}
int  SettingsHandler::getFunscriptModifierStep()
{
    QMutexLocker locker(&mutex);
    return _funscriptModifierStep;
}

void  SettingsHandler::setFunscriptOffsetStep(int value)
{
    QMutexLocker locker(&mutex);
    _funscriptOffsetStep = value;
}
int  SettingsHandler::getFunscriptOffsetStep()
{
    QMutexLocker locker(&mutex);
    return _funscriptOffsetStep;
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
    settingsChangedEvent(true);
}

QSettings* SettingsHandler::settings;
SettingsHandler SettingsHandler::m_instance;
bool SettingsHandler::_settingsChanged;
TCodeVersion SettingsHandler::_selectedTCodeVersion;
bool SettingsHandler::_hideWelcomeScreen;
QMutex SettingsHandler::mutex;
QString SettingsHandler::_appdataLocation;
QHash<QString, bool> SettingsHandler::_funscriptLoaded;
QList<int> SettingsHandler::_mainWindowPos;
QSize SettingsHandler::_maxThumbnailSize = {500, 500};
GamepadAxisNames SettingsHandler::gamepadAxisNames;
MediaActions SettingsHandler::mediaActions;
QHash<QString, QVariant> SettingsHandler::deoDnlaFunscriptLookup;
QString SettingsHandler::selectedTheme;
QString SettingsHandler::selectedLibrary;
QString SettingsHandler::_selectedThumbsDir;
int SettingsHandler::selectedDevice;
int SettingsHandler::_librarySortMode;
int SettingsHandler::playerVolume;
int SettingsHandler::offSet;
bool SettingsHandler::_disableSerialTCodeValidation;

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
int SettingsHandler::_liveXRangeMid;
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
bool SettingsHandler::_xtpWebSyncEnabled;
bool SettingsHandler::defaultReset = false;
bool SettingsHandler::disableSpeechToText;
bool SettingsHandler::_disableVRScriptSelect;
bool SettingsHandler::_disableNoScriptFound;

bool SettingsHandler::_skipToMoneyShotPlaysFunscript;
QString SettingsHandler::_skipToMoneyShotFunscript;
bool SettingsHandler::_skipToMoneyShotSkipsVideo;
bool SettingsHandler::_skipToMoneyShotStandAloneLoop;

bool SettingsHandler::_hideStandAloneFunscriptsInLibrary;
bool SettingsHandler::_skipPlayingSTandAloneFunscriptsInLibrary;

bool SettingsHandler::_enableHttpServer;
QString SettingsHandler::_httpServerRoot;
qint64 SettingsHandler::_httpChunkSize;
int SettingsHandler::_httpPort;
QString SettingsHandler::_vrLibrary;

int SettingsHandler::_funscriptModifierStep;
int SettingsHandler::_funscriptOffsetStep;

QString SettingsHandler::_hashedPass;

QList<DecoderModel> SettingsHandler::decoderPriority;
XVideoRenderer SettingsHandler::_selectedVideoRenderer;

QList<QString> SettingsHandler::_libraryExclusions;
QMap<QString, QList<LibraryListItem>> SettingsHandler::_playlists;
QHash<QString, LibraryListItemMetaData258> SettingsHandler::_libraryListItemMetaDatas;
