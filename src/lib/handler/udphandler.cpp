#include "udphandler.h"

UdpHandler::UdpHandler()
{

}
UdpHandler::~UdpHandler()
{

}

void UdpHandler::init(NetworkAddress address)
{
    QMutexLocker locker(&mutex);
    emit connectionChange({DeviceType::Network, ConnectionStatus::Connecting, "Connecting..."});
    isSelected = true;
    udpSocketSend.reset(new QUdpSocket(this));
    this->address = address;
    addressObj.setAddress(address.address);
    udpSocketSend->connectToHost(addressObj, address.port);
    udpSocketRecieve.reset(new QUdpSocket(this));
    connect(udpSocketRecieve.get(), &QUdpSocket::stateChanged, this, &UdpHandler::onSocketStateChange);
    //connect(udpSocketRecieve.get(), &QUdpSocket::readyRead, this, &UdpHandler::readData);
    if (udpSocketRecieve->bind(QHostAddress::Any, 54000))
    {
        locker.unlock();
        while(!tCodeConnected && isSelected)
        {
            sendTCode("D1");
            QThread::sleep(1);

        }
    }
    else
    {
        emit connectionChange({DeviceType::Network, ConnectionStatus::Error, "Error opening handshake"});
    }
}

void UdpHandler::sendTCode(QString tcode)
{
    QMutexLocker locker(&mutex);
    if(udpSocketSend->isWritable())
    {
        tcode += "\n";
        QByteArray data;
        data.append(tcode);

        // Sends the datagram datagram
        // to the host address and at port.
        // qint64 QUdpSocket::writeDatagram(const QByteArray & datagram,
        //                      const QHostAddress & host, quint16 port)
        LogHandler::Debug("Sending TCode UDP: "+tcode);
        udpSocketSend->writeDatagram(data, addressObj, address.port);
    }
}

void UdpHandler::Dispose()
{
    QMutexLocker locker(&mutex);
    udpSocketSend->close();
    udpSocketRecieve->close();
    tCodeConnected = false;
    emit connectionChange({DeviceType::Network, ConnectionStatus::Disconnected, "Disconnected"});
}



void UdpHandler::readData()
{
    while (udpSocketRecieve->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = udpSocketRecieve->receiveDatagram();

        QString data = QString(datagram.data());
        if(data == SettingsHandler::TCodeVersion)
        {
            tCodeConnected = true;
            emit connectionChange({DeviceType::Network, ConnectionStatus::Connected, "Connected"});
        }
    }
}

void UdpHandler::onSocketStateChange (QAbstractSocket::SocketState state) {
    if ( state == QAbstractSocket::BoundState ) {
        connect(udpSocketRecieve.get(), &QUdpSocket::readyRead, this, &UdpHandler::readData);
    }
}

QMutex UdpHandler::mutex;
