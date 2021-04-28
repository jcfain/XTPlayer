#ifndef SETTINGSHANDLER_H
#define SETTINGSHANDLER_H
#include <QSettings>
#include <QMutex>
#include <QHash>
#include "loghandler.h"
#include "../lookup/enum.h"
#include "../lookup/AxisNames.h"
#include "../lookup/GamepadAxisNames.h"
#include "../lookup/MediaActions.h"
#include "../tool/xmath.h"
#include "../struct/ChannelModel.h"
#include "../struct/DecoderModel.h"
#include "../struct/LibraryListItem.h"
#include "../struct/LibraryListItemMetaData.h"

class SettingsHandler
{
public:
    static const QString TCodeVersion;
    static const QString XTPVersion;
    static const float XTPVersionNum;
    static QString getDeoDnlaFunscript(QString key);
    static QString getSelectedTheme();
    static QString getSelectedLibrary();
    static QString getSelectedFunscriptLibrary();
    static int getSelectedDevice();
    static QString getSerialPort();
    static QString getServerAddress();
    static QString getServerPort();
    static int getPlayerVolume();
    static int getoffSet();

    static int getChannelUserMin(QString channel);
    static int getChannelUserMax(QString channel);
    static void setChannelUserMin(QString channel, int value);
    static void setChannelUserMax(QString channel, int value);

    static LibraryView getLibraryView();
    static int getVideoIncrement();

    static bool getGamepadEnabled();
    static QMap<QString, QString>* getGamePadMap();
    static QString getGamePadMapButton(QString gamepadButton);
    static QMap<QString, ChannelModel>* getAvailableAxis();
    static ChannelModel getAxis(QString axis);
    static bool getInverseTcXL0();
    static bool getInverseTcXRollR2();
    static bool getInverseTcYRollR1();
    static int getGamepadSpeed();
    static int getLiveGamepadSpeed();
    static int getGamepadSpeedIncrement();

    static QList<int> getMainWindowSplitterPos();
    static void setMainWindowSplitterPos(QList<int> value);

    static bool getDisableSpeechToText();
    static void setDisableSpeechToText(bool value);

    static void setDeoDnlaFunscript(QString key, QString value);
    static void setSelectedTheme(QString value);
    static void setSelectedLibrary(QString value);
    static void setSelectedFunscriptLibrary(QString value);
    static void setSelectedDevice(int value);
    static void setSerialPort(QString value);
    static void setServerAddress(QString value);
    static void setServerPort(QString value);

    static QString getDeoAddress();
    static QString getDeoPort();
    static bool getDeoEnabled();
    static void setDeoAddress(QString value);
    static void setDeoPort(QString value);
    static void setDeoEnabled(bool value);

    static QString getWhirligigAddress();
    static QString getWhirligigPort();
    static bool getWhirligigEnabled();
    static void setWhirligigAddress(QString value);
    static void setWhirligigPort(QString value);
    static void setWhirligigEnabled(bool value);

    static void setPlayerVolume(int value);
    static void setoffSet(int value);

    static float getMultiplierValue(QString channel);
    static void setMultiplierValue(QString channel, float value);
    static bool getMultiplierChecked(QString channel);
    static void setMultiplierChecked(QString channel, bool value);

    static bool getChannelInverseChecked(QString channel);
    static void setChannelInverseChecked(QString channel, bool value);

    static float getDamperValue(QString channel);
    static void setDamperValue(QString channel, float value);
    static bool getDamperChecked(QString channel);
    static void setDamperChecked(QString channel, bool value);
    static bool getLinkToRelatedAxisChecked(QString channel);
    static void setLinkToRelatedAxisChecked(QString channel, bool value);

    static void setLibraryView(int value);
    static void setThumbSize(int value);
    static int getThumbSize();
    static void setVideoIncrement(int value);

    static void setLibrarySortMode(int value);
    static LibrarySortMode getLibrarySortMode();

    static void setGamepadEnabled(bool value);
    static void setGamePadMapButton(QString gamePadButton, QString axis);
    static void setAxis(QString axis, ChannelModel channel);
    static void addAxis(ChannelModel channel);
    static void deleteAxis(QString axis);
    static void setInverseTcXL0(bool value);
    static void setInverseTcXRollR2(bool value);
    static void setInverseTcYRollR1(bool value);
    static void setGamepadSpeed(int value);
    static void setGamepadSpeedStep(int value);
    static void setLiveGamepadSpeed(int value);

    static void setXRangeStep(int value);
    static int getXRangeStep();


    static void setLiveXRangeMin(int value);
    static void setLiveXRangeMax(int value);
    static int getLiveXRangeMin();
    static int getLiveXRangeMax();
    static void resetLiveXRange();
    static bool getMultiplierEnabled();
    static void setMultiplierEnabled(bool value);
    static void setLiveMultiplierEnabled(bool value);
    static bool getLiveGamepadConnected();
    static void setLiveGamepadConnected(bool value);
    static bool getLiveActionPaused();
    static void setLiveActionPaused(bool value);

    static void setDecoderPriority(QList<DecoderModel> value);
    static QList<DecoderModel> getDecoderPriority();

    static void addToLibraryExclusions(QString values);
    static void removeFromLibraryExclusions(QList<int> indexes);
    static QList<QString> getLibraryExclusions();

    static QMap<QString, QList<LibraryListItem>> getPlaylists();
    static void setPlaylists(QMap<QString, QList<LibraryListItem>> value);
    static void addToPlaylist(QString name, LibraryListItem value);
    static void updatePlaylist(QString name, QList<LibraryListItem> value);
    static void addNewPlaylist(QString name);
    static void deletePlaylist(QString name);

    static void setFunscriptLoaded(QString key, bool loaded);
    static bool getFunscriptLoaded(QString key);

    static LibraryListItemMetaData getLibraryListItemMetaData(QString path);
    static void updateLibraryListItemMetaData(LibraryListItemMetaData libraryListItemMetaData);

    static QString GetHashedPass();
    static void SetHashedPass(QString value);

    static QSize getMaxThumbnailSize();
    static void SetupAvailableAxis();
    static void SetupDecoderPriority();
    static void Load();
    static void Save();
    static void PersistSelectSettings();
    static void Default();
    static void Clear();

private:
    SettingsHandler();
    ~SettingsHandler();
    static void SetMapDefaults();
    static void SetupGamepadButtonMap();
    static void MigrateTo23();
    static void MigrateTo25();
    static void MigrateTo252();
    static GamepadAxisNames gamepadAxisNames;
    static TCodeChannels channelNames;
    static MediaActions mediaActions;
    static QSize _maxThumbnailSize;

    static QHash<QString, QVariant> deoDnlaFunscriptLookup;
    static QString selectedTheme;
    static QString selectedLibrary;
    static QString selectedFunscriptLibrary;
    static QString selectedFile;
    static int selectedDevice;
    static QString serialPort;
    static QString serverAddress;
    static QString serverPort;
    static QString deoAddress;
    static QString deoPort;
    static bool deoEnabled;
    static QString whirligigAddress;
    static QString whirligigPort;
    static bool whirligigEnabled;
    static int playerVolume;
    static int offSet;
    static QList<int> _mainWindowPos;

    static bool _gamePadEnabled;
    static QMap<QString, QString> _gamepadButtonMap;
    static QMap<QString, ChannelModel> _availableAxis;
    static bool _inverseStroke;
    static bool _inversePitch;
    static bool _inverseRoll;
    static int _gamepadSpeed;
    static int _gamepadSpeedStep;
    static int _liveGamepadSpeed;
    static bool _liveGamepadConnected;
    static bool _liveActionPaused;

    static int _xRangeStep;
    static int _liveXRangeMax;
    static int _liveXRangeMin;
    static bool _liveMultiplierEnabled;
    static bool _multiplierEnabled;

    static int libraryView;
    static int _librarySortMode;
    static int thumbSize;
    static int thumbSizeList;
    static int videoIncrement;

    static QList<DecoderModel> decoderPriority;
    static QList<QString> _libraryExclusions;
    static QMap<QString, QList<LibraryListItem>> _playlists;
    static QHash<QString, LibraryListItemMetaData> _libraryListItemMetaDatas;

    static bool disableSpeechToText;
    static bool defaultReset;
    static QString _hashedPass;

    static QHash<QString, bool> _funscriptLoaded;
    static QSettings* settings;
    static QMutex mutex;
};

#endif // SETTINGSHANDLER_H
