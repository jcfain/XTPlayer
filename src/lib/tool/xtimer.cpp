#include "xtimer.h"

XTimer::XTimer(QObject *parent) :
    QThread(parent)
{

}

XTimer::~XTimer()
{
    stop();
    if(isRunning())
    {
        quit();
        wait();
    }
};
int XTimer::remainingTime()
{
    const QMutexLocker locker(&_mutex);
    return _inValue;
}
void XTimer::init(int mSec)
{
    const QMutexLocker locker(&_mutex);
    _inValue = mSec;
    _stop = false;
    if(!isRunning())
    {
        start();
    }
    else
    {
        _cond.wakeOne();
    }
}
void XTimer::stop()
{
    const QMutexLocker locker(&_mutex);
    _stop = true;
    _cond.wakeOne();
}
void XTimer::run()
{
    _mutex.lock();
    int inValue = _inValue;
    _mutex.unlock();
    qint64 timer1 = QTime::currentTime().msecsSinceStartOfDay();
    qint64 timer2 = QTime::currentTime().msecsSinceStartOfDay();

    while (!_stop)
    {
        if(inValue > 0)
        {
            if (timer2 - timer1 >= 1)
            {
                timer1 = timer2;
                inValue--;
                if(inValue < 0)
                {
                    inValue = 0;
                }
            }
            timer2 = QTime::currentTime().msecsSinceStartOfDay();
        }
        else if (!_stop)
        {
            _mutex.lock();
            _inValue = 0;
            _cond.wait(&_mutex);
            inValue = _inValue;
            _mutex.unlock();
            timer1 = QTime::currentTime().msecsSinceStartOfDay();
            timer2 = QTime::currentTime().msecsSinceStartOfDay();
        }
    }
    _mutex.lock();
    _inValue = -1;
    _mutex.unlock();
}
