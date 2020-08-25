#include "udphandler.h"

UdpHandler::UdpHandler(QObject *parent) :
    QThread(parent)
{
}
UdpHandler::~UdpHandler()
{
    _mutex.lock();
    _stop = true;
    _cond.wakeOne();
    _mutex.unlock();
    wait();
}

void UdpHandler::init(NetworkAddress address, int waitTimeout)
{
    qRegisterMetaType<ConnectionChangedSignal>();
    emit connectionChange({DeviceType::Network, ConnectionStatus::Connecting, "Connecting..."});
    _mutex.lock();
    _stop = false;
    _isSelected = true;
    _waitTimeout = waitTimeout;
    _address = address;
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
        _stop = true;
        emit connectionChange({DeviceType::Network, ConnectionStatus::Error, "Timed out"});
    }
}

void UdpHandler::sendTCode(QString tcode)
{
    const QMutexLocker locker(&_mutex);
    _tcode = tcode + "\n";
    LogHandler::Debug("Sending TCode UDP: "+tcode);
    if (!isRunning())
        start();
    else
        _cond.wakeOne();
}

void UdpHandler::run()
{
    bool currentAddressChanged = false;

    _mutex.lock();
    _tcode = "";
    QByteArray currentRequest;
    currentRequest.append(_tcode);

    QString currentAddress;
    int currentPort = 0;
    QHostAddress addressObj;
    addressObj.setAddress(_address.address);
    currentAddress = _address.address;
    currentPort = _address.port;
    _isConnected = false;
    currentAddressChanged = true;

    _mutex.unlock();

    QScopedPointer<QUdpSocket> udpSocketSend(new QUdpSocket(this));

    //QScopedPointer<QUdpSocket> udpSocketRecieve(new QUdpSocket(this));
    //if (!udpSocketSend->bind(QHostAddress::Any, 54000))
    //{
        //emit connectionChange({DeviceType::Network, ConnectionStatus::Error, "Error opening handshake"});
    //}
    while (!_stop)
    {
        if (currentAddressChanged)
        {
            udpSocketSend->connectToHost(addressObj, currentPort);
            if(!udpSocketSend->waitForConnected(_waitTimeout))
            {
                emit connectionChange({DeviceType::Network, ConnectionStatus::Error, "Can't connect"});
            }
        }
        if(udpSocketSend->isWritable())
        {
            udpSocketSend->write(currentRequest);
            if (!_isConnected)
            {
                if(udpSocketSend->waitForReadyRead(_waitTimeout))
                {
                    QNetworkDatagram datagram = udpSocketSend->receiveDatagram();

                    QString response = QString(datagram.data());
                    if (response.startsWith(SettingsHandler::TCodeVersion))
                    {
                        emit connectionChange({DeviceType::Network, ConnectionStatus::Connected, "Connected"});
                        _mutex.lock();
                        _isConnected = true;
                        _mutex.unlock();
                    }
                    else
                    {
                        emit connectionChange({DeviceType::Network, ConnectionStatus::Error, "No TCode"});
                    }
                }
            }
        }


        _mutex.lock();
        _cond.wait(&_mutex);
        if (currentAddress != _address.address || currentPort != _address.port)
        {
            currentAddress = _address.address;
            addressObj.setAddress(_address.address);
            currentPort = _address.port;
            currentAddressChanged = true;
            _isConnected = false;
        }
        else
        {
            currentAddressChanged = false;
        }

        currentRequest.clear();
        currentRequest.append(_tcode);
        _mutex.unlock();
    }
}

void UdpHandler::dispose()
{
    QMutexLocker locker(&_mutex);
    _isConnected = false;
    _stop = true;
    emit connectionChange({DeviceType::Network, ConnectionStatus::Disconnected, "Disconnected"});
    _cond.wakeOne();
}

bool UdpHandler::isConnected()
{
    QMutexLocker locker(&_mutex);
    return _isConnected;
}

