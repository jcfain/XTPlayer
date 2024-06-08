#include "xtpsettings.h"
#include "lib/handler/settingshandler.h"

const QString XTPSettings::XTPVersion = "0.454b";
const float XTPSettings::XTPVersionNum = 0.454f;
const QString XTPSettings::XTPVersionTimeStamp = QString(XTPVersion +" %1T%2").arg(__DATE__).arg(__TIME__);

XTPSettings::XTPSettings() {}

void XTPSettings::save(QSettings* settingsToSaveTo) {
    LogHandler::Info("Saving XTP settings");
    if(!settingsToSaveTo)
        settingsToSaveTo = getSettings();
    settingsToSaveTo->setValue("selectedTheme", m_selectedTheme);
    settingsToSaveTo->setValue("rememberWindowsSettings", m_rememberWindowsSettings);
    settingsToSaveTo->setValue("libraryWindowOpen", m_libraryWindowOpen);
    settingsToSaveTo->setValue("disableTimeLinePreview", m_disableTimeLinePreview);
    settingsToSaveTo->setValue("hideMediaWithoutFunscripts", m_hideMediaWithoutFunscripts);
    settingsToSaveTo->setValue("heatmapDisabled", m_heatmapDisabled);

    QList<QVariant> splitterPos;
    int i = 0;
    foreach(auto pos, m_mainWindowSplitterPos)
    {
        if(i==2)//Bandaid. Dont store over two.
            break;
        splitterPos.append(pos);
        i++;
    }
    settingsToSaveTo->setValue("mainWindowPos", splitterPos);

    settingsToSaveTo->sync();
    //SettingsHandler::Save(settingsToSaveTo);
    LogHandler::Info("Save complete");
}

void XTPSettings::load(QSettings* settingsToLoadFrom) {
    if(!settingsToLoadFrom)
        settingsToLoadFrom = getSettings();

    m_selectedTheme = settingsToLoadFrom->value("selectedTheme").toString();
    m_selectedTheme = m_selectedTheme.isEmpty() || !QFile::exists(m_selectedTheme) ? QCoreApplication::applicationDirPath() + "/themes/dark.css" : m_selectedTheme;
    m_xwindowPos = settingsToLoadFrom->value("xwindowPos").toPoint();
    m_xwindowSize = settingsToLoadFrom->value("xwindowSize").toSize();
    m_xLibraryPos = settingsToLoadFrom->value("xLibraryWindowPos").toPoint();
    m_xLibrarySize = settingsToLoadFrom->value("xLibraryWindowSize").toSize();
    m_rememberWindowsSettings = settingsToLoadFrom->value("rememberWindowsSettings").toBool();
    m_libraryWindowOpen = settingsToLoadFrom->value("libraryWindowOpen").toBool();
    m_disableTimeLinePreview = settingsToLoadFrom->value("disableTimeLinePreview").toBool();
    if(SettingsHandler::getFirstLoad())
        m_disableTimeLinePreview = true;
    m_hideMediaWithoutFunscripts = settingsToLoadFrom->value("hideMediaWithoutFunscripts").toBool();
    m_heatmapDisabled = settingsToLoadFrom->value("heatmapDisabled").toBool();

    auto splitterSizes = settingsToLoadFrom->value("mainWindowPos").toList();
    if(splitterSizes.isEmpty()) {
        splitterSizes.append(398);
        splitterSizes.append(782);
    }
    int i = 0;
    m_mainWindowSplitterPos.clear();
    foreach (auto splitterPos, splitterSizes)
    {
        if(i==2)//Bandaid. Dont store over two.
            break;
        m_mainWindowSplitterPos.append(splitterPos.value<int>());
        i++;
    }
}

void XTPSettings::import(QSettings* settingsToImportFrom) {
    SettingsHandler::Load(settingsToImportFrom);
    load(settingsToImportFrom);
}


QString XTPSettings::getSelectedTheme()
{
    QMutexLocker locker(&m_mutex);
    return m_selectedTheme;
}
void XTPSettings::setSelectedTheme(QString value)
{
    QMutexLocker locker(&m_mutex);
    m_selectedTheme = value;
}

QList<int> XTPSettings::getMainWindowSplitterPos()
{
    QMutexLocker locker(&m_mutex);
    return m_mainWindowSplitterPos;
}
void XTPSettings::setMainWindowSplitterPos(QList<int> value)
{
    QMutexLocker locker(&m_mutex);
    m_mainWindowSplitterPos = value;
}

void XTPSettings::setXWindowPosition(QPoint position) {
    m_xwindowPos = position;
}
QPoint XTPSettings::getXWindowPosition() {
    return m_xwindowPos;
}
void XTPSettings::setXWindowSize(QSize size) {
    m_xwindowSize = size;
}
QSize XTPSettings::getXWindowSize() {
    return m_xwindowSize;
}
void XTPSettings::setLibraryWindowPosition(QPoint position) {
    m_xLibraryPos = position;
}
QPoint XTPSettings::getLibraryWindowPosition() {
    return m_xLibraryPos;
}
void XTPSettings::setLibraryWindowSize(QSize size) {
    m_xLibrarySize = size;
}
QSize XTPSettings::getLibraryWindowSize() {
    return m_xLibrarySize;
}

void XTPSettings::setHideMediaWithoutFunscripts(bool enabled) {
    m_hideMediaWithoutFunscripts = enabled;
    emit SettingsHandler::instance()->settingsChanged(true);
}
bool XTPSettings::getHideMediaWithoutFunscripts() {
    return m_hideMediaWithoutFunscripts;
}

void XTPSettings::resetWindowSize() {
    setXWindowPosition({0,0});
    setXWindowSize({0,0});
    setLibraryWindowPosition({0,0});
    setLibraryWindowSize({0,0});
    m_mainWindowSplitterPos.clear();
}

void XTPSettings::setRememberWindowsSettings(bool enabled) {
    m_rememberWindowsSettings = enabled;
    emit SettingsHandler::instance()->settingsChanged(true);
}
bool XTPSettings::getRememberWindowsSettings() {
    return m_rememberWindowsSettings;
}

void XTPSettings::setLibraryWindowOpen(bool open) {
    m_libraryWindowOpen = open;
}
bool XTPSettings::getLibraryWindowOpen() {
    return m_libraryWindowOpen;
}

void XTPSettings::setDisableTimeLinePreview(bool enabled) {
    m_disableTimeLinePreview = enabled;
}
bool XTPSettings::getDisableTimeLinePreview() {
   return m_disableTimeLinePreview;
}

void XTPSettings::setHeatmapDisabled(bool checked) {
    m_heatmapDisabled = checked;
}

bool XTPSettings::getHeatmapDisabled() {
    return m_heatmapDisabled;
}

//Private
QSettings* XTPSettings::getSettings() {
    return SettingsHandler::getSettings();
}
QString XTPSettings::m_selectedTheme;
QSize XTPSettings::m_xwindowSize = {0,0};
QPoint XTPSettings::m_xwindowPos = {0,0};
QSize XTPSettings::m_xLibrarySize = {0,0};
QPoint XTPSettings::m_xLibraryPos = {0,0};
QList<int> XTPSettings::m_mainWindowSplitterPos;

bool XTPSettings::m_rememberWindowsSettings;
bool XTPSettings::m_libraryWindowOpen;
bool XTPSettings::m_disableTimeLinePreview;
bool XTPSettings::m_hideMediaWithoutFunscripts;
bool XTPSettings::m_heatmapDisabled;
QMutex XTPSettings::m_mutex;
