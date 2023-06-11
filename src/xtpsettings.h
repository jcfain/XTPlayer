#ifndef XTPSETTINGS_H
#define XTPSETTINGS_H

#include <QPoint>
#include <QSize>
#include <QSettings>
#include <QMutex>

class XTPSettings
{
public:
    XTPSettings();

    static const QString XTPVersion;
    static const QString XTPVersionTimeStamp;
    static const float XTPVersionNum;

    static void save(QSettings* settingsToSaveTo = 0);
    static void load(QSettings* settingsToLoadFrom = 0);
    static void import(QSettings* settingsToImportFrom = 0);

    static QString getSelectedTheme();
    static void setSelectedTheme(QString value);

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
    static QList<int> getMainWindowSplitterPos();
    static void setMainWindowSplitterPos(QList<int> value);

    static void setDisableTimeLinePreview(bool enabled);
    static bool getDisableTimeLinePreview();\

    static void setHideMediaWithoutFunscripts(bool enabled);
    static bool getHideMediaWithoutFunscripts();

    static void setHeatmapDisabled(bool checked);
    static bool getHeatmapDisabled();


private:
    static QSettings* getSettings();
    static QString m_selectedTheme;
    static bool m_rememberWindowsSettings;
    static bool m_libraryWindowOpen;
    static QSize m_xwindowSize;
    static QPoint m_xwindowPos;
    static QSize m_xLibrarySize;
    static QPoint m_xLibraryPos;
    static bool m_disableTimeLinePreview;
    static bool m_hideMediaWithoutFunscripts;
    static QList<int> m_mainWindowSplitterPos;
    static bool m_heatmapDisabled;
    static QMutex m_mutex;
};

#endif // XTPSETTINGS_H
