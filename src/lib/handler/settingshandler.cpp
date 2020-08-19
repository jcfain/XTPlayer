#include "settingshandler.h"

SettingsHandler::SettingsHandler()
{
}

void SettingsHandler::Load()
{
    QMutexLocker locker(&mutex);
    selectedLibrary = settings.value("selectedLibrary").toString();
    selectedDevice = settings.value("selectedDevice").toInt();
    playerVolume = settings.value("playerVolume").toInt();
    speed = settings.value("speed").toInt();
    xMin = settings.value("xMin").toInt();
    yRollMin = settings.value("xRollMin").toInt();
    xRollMin = settings.value("yRollMin").toInt();
    xMax = settings.value("xMax").toInt();
    yRollMax = settings.value("yRollMax").toInt();
    xRollMax = settings.value("xRollMax").toInt();
    selectedFunscriptLibrary = settings.value("selectedFunscriptLibrary").toString();
    serialPort = settings.value("serialPort").toString();
    serverAddress = settings.value("serverAddress").toString();
    serverPort = settings.value("serverPort").toString();
}

void SettingsHandler::Save()
{
    QMutexLocker locker(&mutex);
    settings.setValue("selectedLibrary", selectedLibrary);
    settings.setValue("selectedDevice", selectedDevice);
    settings.setValue("playerVolume", playerVolume);
    settings.setValue("speed", speed == 0 ? 1000 : speed);
    settings.setValue("xMin", xMin == 0 ? 0 : xMin);
    settings.setValue("xMax", xMax == 0 ? 999 : xMax );
    settings.setValue("yRollMin", yRollMin == 0 ? 0 : yRollMin );
    settings.setValue("xRollMin", xRollMin == 0 ? 999 : xRollMin );
    settings.setValue("yRollMax", yRollMax == 0 ? 0 : yRollMax );
    settings.setValue("xRollMax", xRollMax == 0 ? 999 : xRollMax );
    settings.setValue("selectedFunscriptLibrary", selectedFunscriptLibrary);
    settings.setValue("serialPort", serialPort);
    settings.setValue("serverAddress", serverAddress);
    settings.setValue("serverPort", serverPort);

}

void SettingsHandler::Default()
{
    QMutexLocker locker(&mutex);
    settings.setValue("selectedLibrary", QVariant::String);
    settings.setValue("playerVolume", 0);
    settings.setValue("speed", 1000);
    settings.setValue("xMin", 1);
    settings.setValue("yRollMin", 1);
    settings.setValue("xRollMin", 1);
    settings.setValue("xMax", 999);
    settings.setValue("yRollMax", 999);
    settings.setValue("xRollMax", 999);
    settings.setValue("selectedFunscriptLibrary", QVariant::String);
    settings.setValue("serialPort", QVariant::String);
    settings.setValue("serverAddress", QVariant::String);
    settings.setValue("serverPort", 0);
}

QString SettingsHandler::TCodeVersion = "TCode v0.2";
QSettings SettingsHandler::settings{"cUrbSide prOd", "XTPlayer"};
QMutex SettingsHandler::mutex;
QString SettingsHandler::selectedLibrary;
int SettingsHandler::selectedDevice;
int SettingsHandler::playerVolume;
int SettingsHandler::speed;
int SettingsHandler::xMin;
int SettingsHandler::yRollMin;
int SettingsHandler::xRollMin;
int SettingsHandler::xMax;
int SettingsHandler::yRollMax;
int SettingsHandler::xRollMax;
QString SettingsHandler::selectedFunscriptLibrary;
QString SettingsHandler::selectedFile;
QString SettingsHandler::serialPort;
QString SettingsHandler::serverAddress;
QString SettingsHandler::serverPort;
