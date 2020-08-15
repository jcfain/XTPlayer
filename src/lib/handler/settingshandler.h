#ifndef SETTINGSHANDLER_H
#define SETTINGSHANDLER_H
#include <QSettings>

class SettingsHandler
{
public:
    static QString selectedLibrary;
    static QString selectedFile;
    static int playerVolume;

    static void Load();
    static void Save();
    static void Default();

private:
    SettingsHandler();
    static QSettings settings;
};

#endif // SETTINGSHANDLER_H
