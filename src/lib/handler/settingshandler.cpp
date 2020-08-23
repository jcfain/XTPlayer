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
    offSet = settings.value("offSet").toInt();
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
    settings.setValue("offSet", offSet);
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


QString SettingsHandler::getSelectedLibrary()
{
    return selectedLibrary;
}
 QString SettingsHandler::getSelectedFunscriptLibrary()
{
    return selectedFunscriptLibrary;
}
QString SettingsHandler::getSelectedFile()
{
    return selectedFile;
}
int SettingsHandler::getSelectedDevice()
{
    return selectedDevice;
}
QString SettingsHandler::getSerialPort()
{
    return serialPort;
}
QString SettingsHandler::getServerAddress()
{
    return serverAddress;
}
QString SettingsHandler::getServerPort()
{
    return serverPort;
}
int SettingsHandler::getPlayerVolume()
{
    return playerVolume;
}
int SettingsHandler::getoffSet()
{
    return offSet;
}
int SettingsHandler::getoffSetMap()
{
    return XMath::mapRange(offSet, minOffSet, maxOffSet, minOffSetMap, maxOffSetMap);
}
int SettingsHandler::getXMin()
{
    return xMin;
}
int SettingsHandler::getYRollMin()
{
    return yRollMin;
}
int SettingsHandler::getXRollMin()
{
    return xRollMin;
}
int SettingsHandler::getXMax()
{
    return xMax;
}
int SettingsHandler::getYRollMax()
{
    return yRollMax;
}
int SettingsHandler::getXRollMax()
{
    return xRollMax;
}

void SettingsHandler::setSelectedLibrary(QString value)
{
    QMutexLocker locker(&mutex);
    selectedLibrary = value;
}
void SettingsHandler::setSelectedFunscriptLibrary(QString value)
{
    QMutexLocker locker(&mutex);
    selectedFunscriptLibrary = value;
}
void SettingsHandler::setSelectedFile(QString value)
{
    QMutexLocker locker(&mutex);
    selectedFile = value;
}
void SettingsHandler::setSelectedDevice(int value)
{
    QMutexLocker locker(&mutex);
    selectedDevice = value;
}
void SettingsHandler::setSerialPort(QString value)
{
    QMutexLocker locker(&mutex);
    serialPort = value;
}
void SettingsHandler::setServerAddress(QString value)
{
    QMutexLocker locker(&mutex);
    serverAddress = value;
}
void SettingsHandler::setServerPort(QString value)
{
    QMutexLocker locker(&mutex);
    serverPort = value;
}
void SettingsHandler::setPlayerVolume(int value)
{
    QMutexLocker locker(&mutex);
    playerVolume = value;
}
void SettingsHandler::setoffSet(int value)
{
    QMutexLocker locker(&mutex);
    int rangeValue = XMath::mapRange(value, minOffSetMap, maxOffSetMap, minOffSet, maxOffSet);
    offSet = rangeValue;
}
void SettingsHandler::setXMin(int value)
{
    QMutexLocker locker(&mutex);
    xMin = value;
}
void SettingsHandler::setYRollMin(int value)
{
    QMutexLocker locker(&mutex);
    yRollMin = value;
}
void SettingsHandler::setXRollMin(int value)
{
    QMutexLocker locker(&mutex);
    xRollMin = value;
}
void SettingsHandler::setXMax(int value)
{
    QMutexLocker locker(&mutex);
    xMax = value;
}
void SettingsHandler::setYRollMax(int value)
{
    QMutexLocker locker(&mutex);
    yRollMax = value;
}
void SettingsHandler::setXRollMax(int value)
{
    QMutexLocker locker(&mutex);
    xRollMax = value;
}

const QString SettingsHandler::TCodeVersion = "TCode v0.2";
const int SettingsHandler::minOffSetMap = 1;
const int SettingsHandler::midOffSetMap = 1000;
const int SettingsHandler::maxOffSetMap = 2000;
const int SettingsHandler::minOffSet = -1000;
const int SettingsHandler::maxOffSet = 1000;
QSettings SettingsHandler::settings{"cUrbSide prOd", "XTPlayer"};
QMutex SettingsHandler::mutex;
QString SettingsHandler::selectedLibrary;
int SettingsHandler::selectedDevice;
int SettingsHandler::playerVolume;
int SettingsHandler::offSet;
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
