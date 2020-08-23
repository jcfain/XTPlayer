#ifndef SETTINGSHANDLER_H
#define SETTINGSHANDLER_H
#include <QSettings>
#include <QSettings>
#include <QMutex>
#include "../lookup/enum.h"
#include "../tool/xmath.h"
#include "loghandler.h"

class SettingsHandler
{
public:
    static const QString TCodeVersion;
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
    static int getoffSetMap();
    static int getXMin();
    static int getYRollMin();
    static int getXRollMin();
    static int getXMax();
    static int getYRollMax();
    static int getXRollMax();

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

    static void Load();
    static void Save();
    static void Default();

private:
    SettingsHandler();
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

    static QSettings settings;
    static QMutex mutex;
};

#endif // SETTINGSHANDLER_H
