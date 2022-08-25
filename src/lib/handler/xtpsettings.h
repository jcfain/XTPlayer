#ifndef XTPSETTINGS_H
#define XTPSETTINGS_H

#include <QPoint>
#include <QSize>
#include "lib/handler/settingshandler.h"


class XTPSettings
{
public:
    XTPSettings();

    static void save(QSettings* settingsToSaveTo = 0);
    static void load(QSettings* settingsToLoadFrom = 0);
    static void import(QSettings* settingsToImportFrom = 0);

    static void setXWindowPosition(QPoint position);
    static QPoint getXWindowPosition();
    static void setXWindowSize(QSize position);
    static QSize getXWindowSize();
    static void setLibraryWindowPosition(QPoint position);
    static QPoint getLibraryWindowPosition();
    static void setLibraryWindowSize(QSize position);
    static QSize getLibraryWindowSize();
    static void resetWindowSize();

    static void setRememberWindowsSettings(bool enabled);
    static bool getRememberWindowsSettings();
    static void setLibraryWindowOpen(bool enabled);
    static bool getLibraryWindowOpen();

private:
    static QSettings* getSettings();
    static bool _rememberWindowsSettings;
    static bool _libraryWindowOpen;
    static QSize _xwindowSize;
    static QPoint _xwindowPos;
    static QSize _xLibrarySize;
    static QPoint _xLibraryPos;
};

#endif // XTPSETTINGS_H
