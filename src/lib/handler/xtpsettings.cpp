#include "xtpsettings.h"
#include "lib/handler/settingshandler.h"


QString XTPSettings::XTPVersion = QString("0.414b_%1T%2").arg(__DATE__).arg(__TIME__);
float XTPSettings::XTPVersionNum = 0.414f;

XTPSettings::XTPSettings() {}

void XTPSettings::save(QSettings* settingsToSaveTo) {
    if(!settingsToSaveTo)
        settingsToSaveTo = getSettings();
    settingsToSaveTo->setValue("rememberWindowsSettings", m_rememberWindowsSettings);
    settingsToSaveTo->setValue("libraryWindowOpen", m_libraryWindowOpen);
    settingsToSaveTo->setValue("disableTimeLinePreview", m_disableTimeLinePreview);
    settingsToSaveTo->setValue("hideMediaWithoutFunscripts", m_hideMediaWithoutFunscripts);


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

    SettingsHandler::Save(settingsToSaveTo);
}

void XTPSettings::load(QSettings* settingsToLoadFrom) {
    if(!settingsToLoadFrom)
        settingsToLoadFrom = getSettings();

    m_xwindowPos = settingsToLoadFrom->value("xwindowPos").toPoint();
    m_xwindowSize = settingsToLoadFrom->value("xwindowSize").toSize();
    m_xLibraryPos = settingsToLoadFrom->value("xLibraryWindowPos").toPoint();
    m_xLibrarySize = settingsToLoadFrom->value("xLibraryWindowSize").toSize();
    m_rememberWindowsSettings = settingsToLoadFrom->value("rememberWindowsSettings").toBool();
    m_libraryWindowOpen = settingsToLoadFrom->value("libraryWindowOpen").toBool();
    m_disableTimeLinePreview = settingsToLoadFrom->value("disableTimeLinePreview").toBool();
    m_hideMediaWithoutFunscripts = settingsToLoadFrom->value("hideMediaWithoutFunscripts").toBool();
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
    emit SettingsHandler::instance().settingsChanged(true);
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
    emit SettingsHandler::instance().settingsChanged(true);
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

//Private
QSettings* XTPSettings::getSettings() {
    return SettingsHandler::getSettings();
}

QSize XTPSettings::m_xwindowSize = {0,0};
QPoint XTPSettings::m_xwindowPos = {0,0};
QSize XTPSettings::m_xLibrarySize = {0,0};
QPoint XTPSettings::m_xLibraryPos = {0,0};
QList<int> XTPSettings::m_mainWindowSplitterPos;

bool XTPSettings::m_rememberWindowsSettings;
bool XTPSettings::m_libraryWindowOpen;
bool XTPSettings::m_disableTimeLinePreview;
bool XTPSettings::m_hideMediaWithoutFunscripts;
QMutex XTPSettings::m_mutex;
