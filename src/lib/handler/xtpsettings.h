#ifndef XTPSETTINGS_H
#define XTPSETTINGS_H

#include <QPoint>
#include <QSize>
#include <QSettings>

class XTPSettings
{
public:
    XTPSettings();

    static QString XTPVersion;
    static float XTPVersionNum;

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
    static void setDisableTimeLinePreview(bool enabled);
    static bool getDisableTimeLinePreview();

private:
    static QSettings* getSettings();
    static bool m_rememberWindowsSettings;
    static bool m_libraryWindowOpen;
    static QSize m_xwindowSize;
    static QPoint m_xwindowPos;
    static QSize m_xLibrarySize;
    static QPoint m_xLibraryPos;
    static bool m_disableTimeLinePreview;
};

#endif // XTPSETTINGS_H
