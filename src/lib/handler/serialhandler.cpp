#include "serialhandler.h"

SerialHandler::SerialHandler()
{
    m_serial = new QSerialPort(this);
    connect(m_serial, &QSerialPort::errorOccurred, this, &SerialHandler::handleError);
    connect(m_serial, &QSerialPort::readyRead, this, &SerialHandler::readData);
}
SerialHandler::~SerialHandler()
{
    dispose();
}

void SerialHandler::init(SerialComboboxItem portInfo)
{
    emit connectionChange({DeviceType::Serial, ConnectionStatus::Connecting, "Connecting..."});
    if (m_serial->isOpen())
        m_serial->close();
    m_serial->setPortName(portInfo.portName);
    m_serial->setBaudRate(QSerialPort::Baud115200);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);
    if (m_serial->open(QIODevice::ReadWrite)) {
        sendTCode("D1");
    } else {
        emit connectionChange({DeviceType::Serial, ConnectionStatus::Error, "Error opening"});
    }
}

void SerialHandler::start()
{

}

void SerialHandler::stop()
{

}

void SerialHandler::dispose()
{
    if (m_serial->isOpen())
        m_serial->close();
    emit connectionChange({DeviceType::Serial, ConnectionStatus::Disconnected, "Disconnected"});
    delete m_serial;
}

void SerialHandler::sendTCode(QString tcode)
{
    tcode += '\n';
    LogHandler::Debug("Sending TCode: " + tcode);
    m_serial->write(tcode.toUtf8());
}

void SerialHandler::readData()
{
    if(QString(m_serial->readAll()).compare(SettingsHandler::TCodeVersion))
    {
        emit connectionChange({DeviceType::Serial, ConnectionStatus::Connected, "Connected"});
    }
}

QVector<SerialComboboxItem> SerialHandler::getPorts()
{
    const auto serialPortInfos = QSerialPortInfo::availablePorts();

    QVector<SerialComboboxItem> availablePorts;
    for (const QSerialPortInfo &serialPortInfo : serialPortInfos) {
        QString friendlyName = serialPortInfo.portName() + " " + serialPortInfo.description() ;
        QString portName = serialPortInfo.portName();
        if (!friendlyName.isEmpty() && !portName.isEmpty())
        {
            availablePorts.push_back({friendlyName, portName});
        }
        else if (!portName.isEmpty())
        {
            availablePorts.push_back({portName, portName});
        }
    }
    return availablePorts;

}
void SerialHandler::handleError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::SerialPortError::NoError)
    {
        emit errorOccurred("A serial port error occured! " + m_serial->errorString());
    }
}
