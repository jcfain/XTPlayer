#include "settingshandler.h"

SettingsHandler::SettingsHandler()
{
}

void SettingsHandler::Load() {
    selectedLibrary = settings.value("selectedLibrary").toString();
    selectedDevice = settings.value("selectedDevice").toInt();
    playerVolume = settings.value("playerVolume").toInt();
    selectedFunscriptLibrary = settings.value("selectedFunscriptLibrary").toString();
    serialPort = settings.value("serialPort").toString();
    serverAddress = settings.value("serverAddress").toString();
    serverPort = settings.value("serverPort").toString();
}

void SettingsHandler::Save() {
    settings.setValue("selectedLibrary", selectedLibrary);
    settings.setValue("selectedDevice", selectedDevice);
    settings.setValue("playerVolume", playerVolume);
    settings.setValue("selectedFunscriptLibrary", selectedFunscriptLibrary);
    settings.setValue("serialPort", serialPort);
    settings.setValue("serverAddress", serverAddress);
    settings.setValue("serverPort", serverPort);

}

void SettingsHandler::Default() {
    settings.setValue("selectedLibrary", QVariant::String);
    settings.setValue("playerVolume", 0);
    settings.setValue("selectedFunscriptLibrary", QVariant::String);
    settings.setValue("serialPort", QVariant::String);
    settings.setValue("serverAddress", QVariant::String);
    settings.setValue("serverPort", 0);
}

QString SettingsHandler::TCodeVersion = "TCode v0.2";
QSettings SettingsHandler::settings{"cUrbSide prOd", "playerX"};

QString SettingsHandler::selectedLibrary;
int SettingsHandler::selectedDevice;
int SettingsHandler::playerVolume;
QString SettingsHandler::selectedFunscriptLibrary;
QString SettingsHandler::selectedFile;
QString SettingsHandler::serialPort;
QString SettingsHandler::serverAddress;
QString SettingsHandler::serverPort;
