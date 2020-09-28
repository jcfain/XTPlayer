#ifndef XTIMER_H
#define XTIMER_H
#include <QThread>
#include <QMutex>
#include <QTime>
#include <QWaitCondition>

class XTimer : QThread
{
public:
    explicit XTimer(QObject *parent = nullptr);
    ~XTimer();
    int remainingTime();
    void init(int mSec);
    void stop();
private:
    int _inValue = -1;
    bool _stop;
    virtual void run() override;
    QMutex _mutex;
    QWaitCondition _cond;
};

#endif // XTIMER_H
