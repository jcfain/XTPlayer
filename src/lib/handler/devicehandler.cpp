#include "devicehandler.h"

DeviceHandler::DeviceHandler(QObject *parent) :
    QThread(parent)
{
}
DeviceHandler::~DeviceHandler()
{
}

void DeviceHandler::run(){}

void DeviceHandler::sendTCode(const QString &tcode)
{

}

void DeviceHandler::dispose()
{

}

bool DeviceHandler::isConnected()
{

}
