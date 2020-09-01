#ifndef SETTINGSHANDLER_H
#define SETTINGSHANDLER_H
#include <QSettings>
#include <QMutex>
#include "../lookup/enum.h"
#include "../tool/xmath.h"
#include "loghandler.h"
#include "devicehandler.h"

class SettingsHandler
{
public:
    static const QString TCodeVersion;
    static const QString XTPVersion;
    static const float XTPVersionNum;
    static const int maxOffSet;
    static const int minOffSet;
    static const int maxOffSetMap;
    static const int midOffSetMap;
    static const int minOffSetMap;
    static QString getSelectedLibrary();
    static QString getSelectedFunscriptLibrary();
    static QString getSelectedFile();
    static int getSelectedDevice();
    static QString getSerialPort();
    static QString getServerAddress();
    static QString getServerPort();
    static int getPlayerVolume();
    static int getoffSet();
    static int getXMin();
    static int getYRollMin();
    static int getXRollMin();
    static int getXMax();
    static int getYRollMax();
    static int getXRollMax();
    static int getTwistMax();
    static int getTwistMin();

    static bool getYRollMultiplierChecked();
    static float getYRollMultiplierValue();
    static bool getXRollMultiplierChecked();
    static float getXRollMultiplierValue();
    static bool getTwistMultiplierChecked();
    static float getTwistMultiplierValue();
    static bool getVibMultiplierChecked();
    static float getVibMultiplierValue();

    static int getLibraryView();
    static int getThumbSize();
    static int getThumbSizeList();

    static void setSelectedLibrary(QString value);
    static void setSelectedFunscriptLibrary(QString value);
    static void setSelectedFile(QString value);
    static void setSelectedDevice(int value);
    static void setSerialPort(QString value);
    static void setServerAddress(QString value);
    static void setServerPort(QString value);
    static void setPlayerVolume(int value);
    static void setoffSet(int value);
    static void setXMin(int value);
    static void setYRollMin(int value);
    static void setXRollMin(int value);
    static void setXMax(int value);
    static void setYRollMax(int value);
    static void setXRollMax(int value);
    static void setTwistMin(int value);
    static void setTwistMax(int value);

    static void setYRollMultiplierChecked(bool value);
    static void setYRollMultiplierValue(float value);
    static void setXRollMultiplierChecked(bool value);
    static void setXRollMultiplierValue(float value);
    static void setTwistMultiplierChecked(bool value);
    static void setTwistMultiplierValue(float value);
    static void setVibMultiplierChecked(bool value);
    static void setVibMultiplierValue(float value);

    static void setLibraryView(int value);
    static void setThumbSize(int value);
    static void setThumbSizeList(int value);

    static void Load();
    static void Save();
    static void Default();

private:
    SettingsHandler();
    ~SettingsHandler();
    static QString selectedLibrary;
    static QString selectedFunscriptLibrary;
    static QString selectedFile;
    static int selectedDevice;
    static QString serialPort;
    static QString serverAddress;
    static QString serverPort;
    static int playerVolume;
    static int offSet;
    static int xMin;
    static int yRollMin;
    static int xRollMin;
    static int xMax;
    static int yRollMax;
    static int xRollMax;
    static int twistMax;
    static int twistMin;
    static bool yRollMultiplierChecked;
    static float yRollMultiplierValue;
    static bool xRollMultiplierChecked;
    static float xRollMultiplierValue;
    static bool twistMultiplierChecked;
    static float twistMultiplierValue;
    static bool vibMultiplierChecked;
    static float vibMultiplierValue;

    static int libraryView;
    static int thumbSize;
    static int thumbSizeList;

    static QSettings settings;
    static QMutex mutex;
};

#endif // SETTINGSHANDLER_H
