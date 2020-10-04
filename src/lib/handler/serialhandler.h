#ifndef SERIALHANDLER_H
#define SERIALHANDLER_H
#include <QSerialPort>
#include <QSerialPortInfo>
#include "../struct/SerialComboboxItem.h"
#include "devicehandler.h"
#include "../tool/boolinq.h"

class SerialHandler : public DeviceHandler
{

public:
    explicit SerialHandler(QObject *parent = nullptr);
    ~SerialHandler();

    QList<SerialComboboxItem> getPorts();
    void sendTCode(const QString &tcode) override;
    void init(const QString &portName, int waitTimeout = 5000);
    void dispose() override;
    bool isConnected() override;


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
