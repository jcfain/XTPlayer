#include "serialhandler.h"
SerialHandler::SerialHandler(QObject *parent) :
    DeviceHandler(parent)
{
    qRegisterMetaType<ConnectionChangedSignal>();
}

SerialHandler::~SerialHandler()
{
}

void SerialHandler::init(const QString &portName, int waitTimeout)
{
    auto available = getPorts();

    LogHandler::Debug("Connecting to port: "+ portName);
    LogHandler::Debug("Availible ports length: "+ QString::number(available.length()));
    foreach(SerialComboboxItem port, available)
    {
        LogHandler::Debug("Port: "+ port.portName);
    }
    if(portName.isEmpty())
    {
        //LogHandler::Dialog("No portname specified", XLogLevel::Critical);
        emit connectionChange({DeviceType::Serial, ConnectionStatus::Disconnected, "Disconnected"});
        return;
    }
    else if(available.length() == 0)
    {
        //LogHandler::Dialog("No ports on machine", XLogLevel::Critical);
        emit connectionChange({DeviceType::Serial, ConnectionStatus::Disconnected, "Disconnected"});
        return;
    }
    else if(!boolinq::from(available).any([portName](const SerialComboboxItem &x) { return x.portName == portName; }))
    {
        //LogHandler::Dialog("Port: "+ portName + " not found", XLogLevel::Critical);
        emit connectionChange({DeviceType::Serial, ConnectionStatus::Disconnected, "Disconnected"});
        return;
    }
    emit connectionChange({DeviceType::Serial, ConnectionStatus::Connecting, "Connecting..."});
    _mutex.lock();
    _stop = false;
    _portName = portName;
    _waitTimeout = waitTimeout;
    _mutex.unlock();
    int timeouttracker = 0;
    QElapsedTimer mSecTimer;
    qint64 time1 = 0;
    qint64 time2 = 0;
    mSecTimer.start();
    LogHandler::Debug("Starting timer: "+ portName);
    while(!_isConnected && !_stop && timeouttracker <= 4)
    {
        if (time2 - time1 >= _waitTimeout + 1000 || timeouttracker == 0)
        {
            LogHandler::Debug("Not connected: "+ QString::number(timeouttracker));
            time1 = time2;
            sendTCode("D1");
            ++timeouttracker;
        }
        time2 = (round(mSecTimer.nsecsElapsed() / 1000000));
    }
    if (timeouttracker > 4)
    {
        _stop = true;
        _isConnected = false;
        emit connectionChange({DeviceType::Serial, ConnectionStatus::Error, "Timed out"});
    }
}

void SerialHandler::sendTCode(const QString &tcode)
{
    const QMutexLocker locker(&_mutex);
    _tcode = tcode + "\n";
    LogHandler::Debug("Sending tcode serial: "+ _tcode);
    if (!isRunning())
        start();
    else
        _cond.wakeOne();
}

void SerialHandler::run()
{
    bool currentPortNameChanged = true;

    _mutex.lock();

    QString currentPortName;
    currentPortName = _portName;
    _isConnected = false;
    int currentWaitTimeout = _waitTimeout;
    QString currentRequest = _tcode;
    _tcode = "";

    _mutex.unlock();

    QSerialPort serial;

    if (currentPortName.isEmpty())
    {
        emit errorOccurred(tr("No port name specified"));
        return;
    }

    while (!_stop)
    {
        if (currentPortNameChanged)
        {
            LogHandler::Debug("Connecting to: "+ currentPortName);
            serial.close();
            serial.setPortName(currentPortName);

            if (!serial.open(QIODevice::ReadWrite))
            {
                LogHandler::Debug("Error opening: "+ currentPortName);
                emit errorOccurred(tr("Can't open %1, error code %2")
                           .arg(_portName).arg(serial.error()));
                dispose();
                return;
            }
            else
            {
                LogHandler::Debug("Setting port params: "+ currentPortName);
                serial.setBaudRate(QSerialPort::Baud115200);
                serial.setParity(QSerialPort::NoParity);
                serial.setStopBits(QSerialPort::OneStop);
                serial.setFlowControl(QSerialPort::NoFlowControl);
                serial.setRequestToSend(true);
                serial.setDataTerminalReady(true);
            }
        }
        // write request
        const QByteArray requestData = currentRequest.toUtf8();
        serial.write(requestData);

        if (serial.waitForBytesWritten(_waitTimeout))
        {
            serial.flush();
            // read response
            if ((currentPortNameChanged || !_isConnected) && serial.waitForReadyRead(currentWaitTimeout))
            {
                QByteArray responseData = serial.readAll();
                while (serial.waitForReadyRead(currentWaitTimeout))
                    responseData += serial.readAll();

                const QString response = QString::fromUtf8(responseData);
                LogHandler::Debug("Serial read: "+ response);
                if (response.contains(SettingsHandler::TCodeVersion))
                {
                    emit connectionChange({DeviceType::Serial, ConnectionStatus::Connected, "Connected"});
                    _mutex.lock();
                    _isConnected = true;
                    _mutex.unlock();
                }
                else
                {
                    emit connectionChange({DeviceType::Serial, ConnectionStatus::Error, "No TCode"});
                }
            }
            else if (currentPortNameChanged || !_isConnected)
            {

                LogHandler::Debug(tr("Read serial handshake timeout %1")
                             .arg(QTime::currentTime().toString()));
            }
        }
        else
        {
            LogHandler::Debug(tr("Write tcode to serial timeout %1")
                         .arg(QTime::currentTime().toString()));
        }
        if (!_stop)
        {
            _mutex.lock();
            _cond.wait(&_mutex);
            if (currentPortName != _portName)
            {
                LogHandler::Debug("Port name change "+ _portName);
                currentPortName = _portName;
                currentPortNameChanged = true;
                _isConnected = false;
            }
            else
            {
                currentPortNameChanged = false;
            }
            currentWaitTimeout = _waitTimeout;
            currentRequest = _tcode;
            _mutex.unlock();
        }
    }
}

//Public
bool SerialHandler::isConnected()
{
    const QMutexLocker locker(&_mutex);
    return _isConnected;
}

void SerialHandler::dispose()
{
    _mutex.lock();
    _stop = true;
    _isConnected = false;
    _mutex.unlock();
    _cond.wakeOne();
    emit connectionChange({DeviceType::Serial, ConnectionStatus::Disconnected, "Disconnected"});
    if(isRunning())
    {
        quit();
        wait();
    }
}

QList<SerialComboboxItem> SerialHandler::getPorts()
{
    const auto serialPortInfos = QSerialPortInfo::availablePorts();

    LogHandler::Debug("QSerialPortInfo::availablePorts() length: "+ QString::number(serialPortInfos.length()));
    QList<SerialComboboxItem> availablePorts;
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
