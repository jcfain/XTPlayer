#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H
#include <QVector>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTime>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include "../struct/SerialComboboxItem.h"
#include "../struct/ConnectionChangedSignal.h"
#include "../handler/settingshandler.h"
#include "../handler/loghandler.h"
#include "../lookup/enum.h"

class SerialHandler : public QThread
{
    Q_OBJECT

signals:
    void errorOccurred(QString error);
    void connectionChange(ConnectionChangedSignal status);

public:
    explicit SerialHandler(QObject *parent = nullptr);
    ~SerialHandler();

    QList<SerialComboboxItem> getPorts();
    void sendTCode(const QString &tcode);
    void init(const QString &portName, int waitTimeout = 5000);
    void dispose();
    bool isConnected();


private:
    void run() override;
    QString _portName;
    QString _tcode;
    QMutex _mutex;
    QWaitCondition _cond;
    int _waitTimeout = 0;
    bool _stop = false;
    bool _isConnected = false;
};

#endif // SERIALHANDLER_H
