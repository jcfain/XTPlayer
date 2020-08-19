#ifndef SETTINGSHANDLER_H
#define SETTINGSHANDLER_H
#include <QSettings>
#include <QSettings>
#include <QMutex>
#include "../lookup/enum.h"
#include "loghandler.h"

class SettingsHandler
{
public:
    static QString TCodeVersion;
    static QString selectedLibrary;
    static QString selectedFunscriptLibrary;
    static QString selectedFile;
    static int selectedDevice;
    static QString serialPort;
    static QString serverAddress;
    static QString serverPort;
    static int playerVolume;
    static int speed;
    static int xMin;
    static int yRollMin;
    static int xRollMin;
    static int xMax;
    static int yRollMax;
    static int xRollMax;

    static void Load();
    static void Save();
    static void Default();

private:
    SettingsHandler();
    static QSettings settings;
    static QMutex mutex;
};

#endif // SETTINGSHANDLER_H
