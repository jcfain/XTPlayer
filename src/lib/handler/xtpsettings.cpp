#include "xtpsettings.h"
#include "lib/handler/settingshandler.h"

XTPSettings::XTPSettings() {}

void XTPSettings::save(QSettings* settingsToSaveTo) {
    if(!settingsToSaveTo)
        settingsToSaveTo = getSettings();
    settingsToSaveTo->setValue("xwindowPos", m_xwindowPos);
    settingsToSaveTo->setValue("xwindowSize", m_xwindowSize);
    settingsToSaveTo->setValue("xLibraryWindowPos", m_xLibraryPos);
    settingsToSaveTo->setValue("xLibraryWindowSize", m_xLibrarySize);
    settingsToSaveTo->setValue("rememberWindowsSettings", m_rememberWindowsSettings);
    settingsToSaveTo->setValue("libraryWindowOpen", m_libraryWindowOpen);
    settingsToSaveTo->setValue("disableTimeLinePreview", m_disableTimeLinePreview);

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
}

void XTPSettings::import(QSettings* settingsToImportFrom) {
    SettingsHandler::Load(settingsToImportFrom);
    load(settingsToImportFrom);
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

void XTPSettings::resetWindowSize() {
    setXWindowPosition({0,0});
    setXWindowSize({0,0});
    setLibraryWindowPosition({0,0});
    setLibraryWindowSize({0,0});
    save();
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

bool XTPSettings::m_rememberWindowsSettings;
bool XTPSettings::m_libraryWindowOpen;
bool XTPSettings::m_disableTimeLinePreview;
