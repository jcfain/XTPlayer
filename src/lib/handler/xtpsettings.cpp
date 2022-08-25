#include "xtpsettings.h"

XTPSettings::XTPSettings()
{

}


void XTPSettings::save(QSettings* settingsToSaveTo) {
    if(!settingsToSaveTo)
        settingsToSaveTo = getSettings();
    settingsToSaveTo->setValue("xwindowPos", _xwindowPos);
    settingsToSaveTo->setValue("xwindowSize", _xwindowSize);
    settingsToSaveTo->setValue("xLibraryWindowPos", _xLibraryPos);
    settingsToSaveTo->setValue("xLibraryWindowSize", _xLibrarySize);
    settingsToSaveTo->setValue("rememberWindowsSettings", _rememberWindowsSettings);
    settingsToSaveTo->setValue("libraryWindowOpen", _libraryWindowOpen);

    SettingsHandler::Save(settingsToSaveTo);
}

void XTPSettings::load(QSettings* settingsToLoadFrom) {
    if(!settingsToLoadFrom)
        settingsToLoadFrom = getSettings();

    _xwindowPos = settingsToLoadFrom->value("xwindowPos").toPoint();
    _xwindowSize = settingsToLoadFrom->value("xwindowSize").toSize();
    _xLibraryPos = settingsToLoadFrom->value("xLibraryWindowPos").toPoint();
    _xLibrarySize = settingsToLoadFrom->value("xLibraryWindowSize").toSize();
    _rememberWindowsSettings = settingsToLoadFrom->value("rememberWindowsSettings").toBool();
    _libraryWindowOpen = settingsToLoadFrom->value("libraryWindowOpen").toBool();

}

void XTPSettings::import(QSettings* settingsToImportFrom) {
    SettingsHandler::Load(settingsToImportFrom);
    load(settingsToImportFrom);
}

void XTPSettings::setXWindowPosition(QPoint position) {
    _xwindowPos = position;
}
QPoint XTPSettings::getXWindowPosition() {
    return _xwindowPos;
}
void XTPSettings::setXWindowSize(QSize size) {
    _xwindowSize = size;
}
QSize XTPSettings::getXWindowSize() {
    return _xwindowSize;
}
void XTPSettings::setLibraryWindowPosition(QPoint position) {
    _xLibraryPos = position;
}
QPoint XTPSettings::getLibraryWindowPosition() {
    return _xLibraryPos;
}
void XTPSettings::setLibraryWindowSize(QSize size) {
    _xLibrarySize = size;
}
QSize XTPSettings::getLibraryWindowSize() {
    return _xLibrarySize;
}

void XTPSettings::resetWindowSize() {
    setXWindowPosition({0,0});
    setXWindowSize({0,0});
    setLibraryWindowPosition({0,0});
    setLibraryWindowSize({0,0});
    save();
}

void XTPSettings::setRememberWindowsSettings(bool enabled) {
    _rememberWindowsSettings = enabled;
    emit SettingsHandler::instance().settingsChanged(true);
}
bool XTPSettings::getRememberWindowsSettings() {
    return _rememberWindowsSettings;
}

void XTPSettings::setLibraryWindowOpen(bool open) {
    _libraryWindowOpen = open;
}
bool XTPSettings::getLibraryWindowOpen() {
    return _libraryWindowOpen;
}

//Private
QSettings* XTPSettings::getSettings() {
    return SettingsHandler::getSettings();
}

QSize XTPSettings::_xwindowSize = {0,0};
QPoint XTPSettings::_xwindowPos = {0,0};
QSize XTPSettings::_xLibrarySize = {0,0};
QPoint XTPSettings::_xLibraryPos = {0,0};

bool XTPSettings::_rememberWindowsSettings;
bool XTPSettings::_libraryWindowOpen;
