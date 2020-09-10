#include "deohandler.h"

DeoHandler::DeoHandler(QObject *parent) :
    QObject(parent)
{
    emit connectionChange({DeviceType::Deo, ConnectionStatus::Disconnected, "Disconnected"});
}

DeoHandler::~DeoHandler()
{
    _isConnected = false;
    if (tcpSocket != nullptr)
    {
//        if(tcpSocket->isOpen())
//            tcpSocket->close();
        delete tcpSocket;
    }
    if (keepAliveTimer != nullptr)
        delete keepAliveTimer;
    if (currentDeoPacket != nullptr)
        delete currentDeoPacket;
}

void DeoHandler::init(NetworkAddress address, int waitTimeout)
{
    qRegisterMetaType<ConnectionChangedSignal>();
    qRegisterMetaType<DeoPacket>();
    emit connectionChange({DeviceType::Deo, ConnectionStatus::Connecting, "Connecting..."});
    _waitTimeout = waitTimeout;
    _address = address;

    QHostAddress addressObj;
    addressObj.setAddress(_address.address);
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, &QTcpSocket::stateChanged, this, &DeoHandler::onSocketStateChange);
    connect(tcpSocket, &QTcpSocket::errorOccurred, this, &DeoHandler::tcpErrorOccured);
    tcpSocket->connectToHost(addressObj, _address.port);
}

void DeoHandler::sendKeepAlive()
{
    if (_isConnected)
    {
        send(nullptr);
    }
    else
    {
        keepAliveTimer->stop();
    }
}

void DeoHandler::send(const QString &command)
{
    const QMutexLocker locker(&_mutex);
    _sendCommand = command;
    LogHandler::Debug("Sending to Deo: "+command);
    if (command != nullptr)
    {
        QByteArray currentRequest;
        currentRequest.append(command);
        tcpSocket->write(currentRequest);
    }
    else
    {
        char str[4];
        QByteArray currentRequest(str, 4);
        tcpSocket->write(currentRequest);
    }
    tcpSocket->flush();
}

void DeoHandler::dispose()
{
    if (tcpSocket != nullptr)
    {
        disconnect(tcpSocket, &QTcpSocket::stateChanged, this, &DeoHandler::onSocketStateChange);
        disconnect(tcpSocket, &QTcpSocket::errorOccurred, this, &DeoHandler::tcpErrorOccured);
    }
    const QMutexLocker locker(&_mutex);
    _isConnected = false;
    if (keepAliveTimer != nullptr && keepAliveTimer->isActive())
        keepAliveTimer->stop();
//    if (tcpSocket != nullptr && tcpSocket->isOpen())
//        tcpSocket->close();
    emit connectionChange({DeviceType::Deo, ConnectionStatus::Disconnected, "Disconnected"});
}

void DeoHandler::readData()
{
    //LogHandler::Debug("Deo packet recieved");
    QByteArray datagram = tcpSocket->readAll();
    QByteArray header = datagram.remove(0, 4);
    //LogHandler::Debug("Deo response: "+QString::fromUtf8(datagram));

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(datagram, &error);
    if (error.errorString() != "no error occurred")
    {
        LogHandler::Debug("Deo json response error: "+error.errorString());
        //emit connectionChange({DeviceType::Deo, ConnectionStatus::Error, "Read error: " + error.errorString()});
    }
    else
    {
        QJsonObject jsonObject = doc.object();
        QString path = jsonObject["path"].toString();
        qint64 duration = jsonObject["duration"].toDouble() * 1000;
        qint64 currentTime = jsonObject["currentTime"].toDouble() * 1000;
        _currentTime = currentTime;
        float playbackSpeed = jsonObject["playbackSpeed"].toDouble() * 1.0;
        bool playing = jsonObject["playerState"].toInt() == 0 ? true : false;
        //LogHandler::Debug("Deo path: "+path);
        //LogHandler::Debug("Deo duration: "+QString::number(duration));
        LogHandler::Debug("Deo currentTime: "+QString::number(currentTime));
    //    LogHandler::Debug("Deo playbackSpeed: "+QString::number(playbackSpeed));
    //    LogHandler::Debug("Deo playing: "+QString::number(playing));
        _mutex.lock();
        currentDeoPacket = new DeoPacket
        {
            path,
            duration,
            currentTime,
            playbackSpeed,
            playing
       };
        _mutex.unlock();
        emit messageRecieved({
                                 path,
                                 duration,
                                 currentTime,
                                 playbackSpeed,
                                 playing
                            });
    }
}

bool DeoHandler::isConnected()
{
    return _isConnected;
}

DeoPacket* DeoHandler::getCurrentDeoPacket()
{
    //const QMutexLocker locker(&_mutex);
    return currentDeoPacket;
}

void DeoHandler::onSocketStateChange (QAbstractSocket::SocketState state)
{
    //const QMutexLocker locker(&_mutex);
    switch(state) {
        case QAbstractSocket::SocketState::ConnectedState:
        {
            //_mutex.lock();
            _isConnected = true;
            send(nullptr);
            connect(tcpSocket, &QTcpSocket::readyRead, this, &DeoHandler::readData);
            if (keepAliveTimer != nullptr && keepAliveTimer->isActive())
                keepAliveTimer->stop();
            keepAliveTimer = new QTimer(this);
            //_mutex.unlock();
            connect(keepAliveTimer, &QTimer::timeout, this, &DeoHandler::sendKeepAlive);
            keepAliveTimer->start(1000);
            LogHandler::Debug("Deo connected");
            emit connectionChange({DeviceType::Deo, ConnectionStatus::Connected, "Connected"});
            break;
        }
        case QAbstractSocket::SocketState::UnconnectedState:
        {
            //_mutex.lock();
            _isConnected = false;
            //_mutex.unlock();
            if (keepAliveTimer != nullptr && keepAliveTimer->isActive())
                keepAliveTimer->stop();
            LogHandler::Debug("Deo disconnected");
            emit connectionChange({DeviceType::Deo, ConnectionStatus::Disconnected, "Disconnected"});
            break;
        }
        case QAbstractSocket::SocketState::ConnectingState:
        {
            LogHandler::Debug("Deo connecting");
            emit connectionChange({DeviceType::Deo, ConnectionStatus::Connecting, "Connecting..."});
            break;
        }
        case QAbstractSocket::SocketState::BoundState:
        {
            LogHandler::Debug("Deo bound");
            break;
        }
        case QAbstractSocket::SocketState::ListeningState:
        {
            LogHandler::Debug("Deo listening");
            break;
        }
        case QAbstractSocket::SocketState::HostLookupState:
        {
            LogHandler::Debug("Deo host look up");
            break;
        }
        case QAbstractSocket::SocketState::ClosingState:
        {
            LogHandler::Debug("Deo closing");
            break;
        }
    }
}

void DeoHandler::tcpErrorOccured(QAbstractSocket::SocketError state)
{

    switch(state)
    {
        case QAbstractSocket::SocketError::AddressInUseError:
        {
            LogHandler::Error("Deo AddressInUseError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ConnectionRefusedError:
        {
            LogHandler::Error("Deo ConnectionRefusedError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::DatagramTooLargeError:
        {
            LogHandler::Error("Deo DatagramTooLargeError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::HostNotFoundError:
        {
            LogHandler::Error("Deo HostNotFoundError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::NetworkError:
        {
            LogHandler::Error("Deo NetworkError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::OperationError:
        {
            LogHandler::Error("Deo OperationError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ProxyAuthenticationRequiredError:
        {
            LogHandler::Error("Deo ProxyAuthenticationRequiredError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ProxyConnectionClosedError:
        {
            LogHandler::Error("Deo ProxyConnectionClosedError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ProxyConnectionRefusedError:
        {
            LogHandler::Error("Deo ProxyConnectionRefusedError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ProxyConnectionTimeoutError:
        {
            LogHandler::Error("Deo ProxyConnectionTimeoutError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ProxyNotFoundError:
        {
            LogHandler::Error("Deo ProxyNotFoundError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::ProxyProtocolError:
        {
            LogHandler::Error("Deo ProxyProtocolError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::RemoteHostClosedError:
        {
            LogHandler::Error("Deo RemoteHostClosedError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SocketAccessError:
        {
            LogHandler::Error("Deo SocketAccessError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SocketAddressNotAvailableError:
        {
            LogHandler::Error("Deo SocketAddressNotAvailableError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SocketResourceError:
        {
            LogHandler::Error("Deo SocketResourceError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SocketTimeoutError:
        {
            LogHandler::Error("Deo SocketTimeoutError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SslHandshakeFailedError:
        {
            LogHandler::Error("Deo SslHandshakeFailedError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SslInternalError:
        {
            LogHandler::Error("Deo SslInternalError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::SslInvalidUserDataError:
        {
            LogHandler::Error("Deo SslInvalidUserDataError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::TemporaryError:
        {
            LogHandler::Error("Deo TemporaryError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::UnfinishedSocketOperationError:
        {
            LogHandler::Error("Deo UnfinishedSocketOperationError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::UnknownSocketError:
        {
            LogHandler::Error("Deo UnknownSocketError: "+tcpSocket->errorString());
            break;
        }
        case QAbstractSocket::SocketError::UnsupportedSocketOperationError:
        {
            LogHandler::Error("Deo UnsupportedSocketOperationError: "+tcpSocket->errorString());
            break;
        }
    }
}
//#include <QNetworkRequest>
//#include <QNetworkAccessManager>
//#include <QNetworkReply>
//#include <QEventLoop>
//QString m_device; //!< Device uuid.
//QNetworkReply::NetworkError m_error = QNetworkReply::NoError; //!< Network error.
//QNetworkAccessManager* m_naMgr = nullptr; //!< The current netword access manager. see CActionManager (QNetworkAccessManager* naMgr, QObject* parent).

//static int m_elapsedTime; //!< The time to execute the last action.
//static QString m_lastError; //!< The error generated by the last action.
//bool DeoHandler::post (QString const & device, QUrl const & url, int timeout)
//{
//    if (m_naMgr == nullptr)
//    {
//      m_naMgr = new QNetworkAccessManager (this);
//    }
//  m_lastError.clear ();
//  m_device     = device;
//  bool success = false;
//  if (!isRunning ())
//  {
//    QNetworkRequest req (url);
//    req.setPriority (QNetworkRequest::HighPriority);
//     // To fix a problem with DSM6 (Synology). If User-Agent exists DSM send only the full precision
//     // image not the thumbnails.
//    req.setHeader (QNetworkRequest::UserAgentHeader, " ");
//    req.setHeader (QNetworkRequest::ContentTypeHeader, QString ("text/xml; charset=\"utf-8\""));
//    req.setRawHeader ("Accept-Encoding", "*");
//    req.setRawHeader ("Accept-Language", "*");
//    req.setRawHeader ("Connection", "Close");
//    QString const & actionName    = info.actionName ();
//    QString         soapActionHdr = QString ("\"%1#%2\"").arg (info.serviceID (), actionName);
//    req.setRawHeader ("SOAPAction", soapActionHdr.toUtf8 ());

//    QTime time;
//    time.start ();

//    m_naMgr->setNetworkAccessible (QNetworkAccessManager::Accessible);
//    QNetworkReply* reply = m_naMgr->post (req, info.message ().toUtf8 ());
//    connect (reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
//    connect (reply, SIGNAL(finished()), this, SLOT(finished()));

//    m_error     = QNetworkReply::NoError;
//    int idTimer = startTimer (timeout);
//    int retCode = exec (QEventLoop::ExcludeUserInputEvents/* | QEventLoop::ExcludeSocketNotifiers*/);
//    success     =  retCode == 0 && m_error == QNetworkReply::NoError;
//    killTimer (idTimer);
//    if (success)
//    {
//      info.setResponse (reply->readAll ());
//    }
//    else
//    {
//      if (m_error != QNetworkReply::NoError)
//      {
//        qint32 statusCode = reply->attribute (QNetworkRequest::HttpStatusCodeAttribute).toInt ();
//        m_lastError       = reply->attribute (QNetworkRequest::HttpReasonPhraseAttribute).toString ();
//        QString message   = QString ("Action failed. Response from the server: %1, %2").arg (statusCode).arg (m_lastError);
//        message          += '\n';
//        message          += url.toString () + '\n';
//        message          += info.message () + "\n\n";
//        qDebug () << "CActionManager::post:" << message;
//        m_lastError.prepend (QString ("(%1) ").arg (statusCode));
//      }
//    }

//    reply->deleteLater ();
//    m_elapsedTime = time.elapsed ();
//  }

//  return success;
//}
