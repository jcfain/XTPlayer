#include "serialhandler.h"
SerialHandler::SerialHandler(QObject *parent) :
    QThread(parent)
{
}

SerialHandler::~SerialHandler()
{
    _mutex.lock();
    _stop = true;
    _cond.wakeOne();
    _mutex.unlock();
    wait();
}
void SerialHandler::init(const QString &portName, int waitTimeout)
{
    //qRegisterMetaType<ConnectionChangedSignal>();
    emit connectionChange({DeviceType::Serial, ConnectionStatus::Connecting, "Connecting..."});
    _mutex.lock();
    _portName = portName;
    _waitTimeout = waitTimeout;
    _mutex.unlock();
    int timeouttracker = 0;
    while(!_isConnected && !_stop && timeouttracker <= 3)
    {
        sendTCode("D1");
        sleep(_waitTimeout / 1000 + 1);
        ++timeouttracker;
    }
    if (timeouttracker > 3)
    {
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
    bool currentPortNameChanged = false;

    _mutex.lock();
    QString currentPortName;
    if (currentPortName != _portName)
    {
        currentPortName = _portName;
        currentPortNameChanged = true;
        _isConnected = false;
    }

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
            serial.close();
            serial.setPortName(currentPortName);

            if (!serial.open(QIODevice::ReadWrite))
            {
                emit errorOccurred(tr("Can't open %1, error code %2")
                           .arg(_portName).arg(serial.error()));
                return;
            }
            else
            {
                serial.setBaudRate(QSerialPort::Baud115200);
                serial.setParity(QSerialPort::NoParity);
                serial.setStopBits(QSerialPort::OneStop);
                serial.setFlowControl(QSerialPort::NoFlowControl);
            }
        }
        // write request
        const QByteArray requestData = currentRequest.toUtf8();
        serial.write(requestData);

        bool isHandShake = currentRequest.startsWith("D1");
        if (serial.waitForBytesWritten(_waitTimeout))
        {
            // read response
            if (isHandShake && serial.waitForReadyRead(currentWaitTimeout))
            {
                //qRegisterMetaType<ConnectionChangedSignal>();
                QByteArray responseData = serial.readAll();
                while (serial.waitForReadyRead(10))
                    responseData += serial.readAll();

                const QString response = QString::fromUtf8(responseData);
                if (response.startsWith(SettingsHandler::TCodeVersion))
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
            else if (isHandShake)
            {
                emit timeout(tr("Read handshake timeout %1")
                             .arg(QTime::currentTime().toString()));
            }
        }
        else
        {
            emit timeout(tr("Write tcode timeout %1")
                         .arg(QTime::currentTime().toString()));
        }
        _mutex.lock();
        _cond.wait(&_mutex);
        if (currentPortName != _portName)
        {
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
        _tcode = "";
        _mutex.unlock();
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
    const QMutexLocker locker(&_mutex);
    if(!_stop)
        emit connectionChange({DeviceType::Serial, ConnectionStatus::Disconnected, "Disconnected"});
    _stop = true;
}

QList<SerialComboboxItem> SerialHandler::getPorts()
{
    const auto serialPortInfos = QSerialPortInfo::availablePorts();

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
