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
    _portName = portName;
    LogHandler::Debug("Connecting to port: "+ portName);
    LogHandler::Debug("Availible ports length: "+ QString::number(available.length()));
    foreach(SerialComboboxItem port, available)
    {
        LogHandler::Debug("Port: "+ port.portName);
    }
    if(portName.isEmpty() && available.count() > 0)
    {
        foreach(SerialComboboxItem port, available)
        {
            if(port.friendlyName.toLower().contains("arduino") || port.friendlyName.toLower().contains("esp32"))
            {
                _portName = portName;
            }
        }
        if(portName.isEmpty())
            _portName = available[0].portName;
    }
    else if(portName.isEmpty() || available.length() == 0)
    {
        //LogHandler::Dialog("No portname specified", XLogLevel::Critical);
        emit connectionChange({DeviceType::Serial, ConnectionStatus::Disconnected, "No COM"});
        return;
    }
    emit connectionChange({DeviceType::Serial, ConnectionStatus::Connecting, "Connecting..."});
    _mutex.lock();
    _stop = false;
    _waitTimeout = waitTimeout;
    _mutex.unlock();
//    int timeouttracker = 0;
//    QElapsedTimer mSecTimer;
//    qint64 time1 = 0;
//    qint64 time2 = 0;
//    mSecTimer.start();
//    LogHandler::Debug("Starting timer: "+ portName);
//    while(!_isConnected && !_stop && timeouttracker <= 10)
//    {
//        if (time2 - time1 >= _waitTimeout + 1000 || timeouttracker == 0)
//        {
//            LogHandler::Debug("Not connected: "+ QString::number(timeouttracker));
//            time1 = time2;
    sendTCode("D1");
//            ++timeouttracker;
//        }
//        time2 = (round(mSecTimer.nsecsElapsed() / 1000000));
//    }
//    if (timeouttracker > 10)
//    {
//        _stop = true;
//        _isConnected = false;
//        emit connectionChange({DeviceType::Serial, ConnectionStatus::Error, "Timed out"});
//    }
}

void SerialHandler::sendTCode(const QString &tcode)
{
    const QMutexLocker locker(&_mutex);
    _tcode = tcode + "\n";
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
                //serial.setRequestToSend(true);
                serial.setDataTerminalReady(true);
            }
        }
        if(currentRequest.startsWith("D1"))
            QThread::sleep(5);
        // write request
        LogHandler::Debug("Sending TCode serial: "+ currentRequest);
        const QByteArray requestData = currentRequest.toUtf8();
        serial.write(requestData);

        if (serial.waitForBytesWritten(_waitTimeout))
        {
            serial.flush();
            // read response
            if ((currentPortNameChanged || !_isConnected))
            {
                serial.waitForReadyRead(currentWaitTimeout);
                QString version = "V?";
                LogHandler::Debug(tr("Bytes read"));
                QByteArray responseData = serial.readAll();
                while (serial.waitForReadyRead(100))
                    responseData += serial.readAll();

                const QString response = QString::fromUtf8(responseData);
                LogHandler::Debug("Serial read: "+ response);
                bool validated = false;
                if(response.contains(SettingsHandler::SupportedTCodeVersions.value(TCodeVersion::v2)))
                {
                    version = "V2";
                    validated = true;
                }
                else if (response.contains(SettingsHandler::SupportedTCodeVersions.value(TCodeVersion::v3)))
                {
                    version = "V3";
                    validated = true;
                }
                if (validated)
                {
                    if(!_isConnected) //temp
                    { // temp
                                emit connectionChange({DeviceType::Serial, ConnectionStatus::Connected, "Connected: "+version});
                                _mutex.lock();
                                _isConnected = true;
                                _mutex.unlock();
                    } // temp
                }
                else
                {
                    //emit connectionChange({DeviceType::Serial, ConnectionStatus::Error, "No TCode"});
                    // Due to issue with connecting to some romeos with validation. Do not block them from using it.
                    emit connectionChange({DeviceType::Serial, ConnectionStatus::Connected, "Connected: "+version});
                    _mutex.lock();
                    _isConnected = true;
                    _mutex.unlock();
                    LogHandler::Error("An INVALID response recieved: ");
                    LogHandler::Error("response: "+response);
                    emit errorOccurred("Warning! You should be able to keep using the program if you have the correct port selected\n\nIt would be greatly appreciated if you could run the program\nin debug mode.\nSend the console output file to Khrull on patreon or discord. Thanks!");
                }
            }
//            else if (currentPortNameChanged || !_isConnected)
//            {

//                LogHandler::Error(tr("Read serial handshake timeout %1")
//                             .arg(QTime::currentTime().toString()));
//                // Due to issue with connecting to some romeos with validation. Do not block them from using it.
//                emit connectionChange({DeviceType::Serial, ConnectionStatus::Connected, "Connected: V?"});
//                _mutex.lock();
//                _isConnected = true;
//                _mutex.unlock();
//                emit errorOccurred("Warning! You should be able to keep using the program if you have the correct port selected\n\nIt would be greatly appreciated if you could run the program\nin debug mode.\nSend the console output file to Khrull on patreon or discord. Thanks!");
//            }
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
