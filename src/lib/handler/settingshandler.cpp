#include "settingshandler.h"

SettingsHandler::SettingsHandler()
{
}

void SettingsHandler::Load() {
    selectedLibrary = settings.value("selectedLibrary").toString();
    playerVolume = settings.value("playerVolume").toInt();
}

void SettingsHandler::Save() {
    settings.setValue("selectedLibrary", selectedLibrary);
    settings.setValue("playerVolume", playerVolume);
}

void SettingsHandler::Default() {
    settings.setValue("selectedLibrary", QVariant::String);
    settings.setValue("playerVolume", 0);
}


QSettings SettingsHandler::settings{"cUrbSide prOd", "playerX"};
QString SettingsHandler::selectedLibrary;
QString SettingsHandler::selectedFile;
int SettingsHandler::playerVolume;
