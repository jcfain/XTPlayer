#include "xmath.h"
#include <QRandomGenerator>
#include <random>
#include <chrono>
qint64 XMath::mapRange(qint64 value, qint64 inStart, qint64 inEnd, qint64 outStart, qint64 outEnd)
{
    double slope = (double)(outEnd - outStart) / (inEnd - inStart);
    return qRound64(outStart + slope * (value - inStart));
}


int  XMath::constrain(int value, int min, int max)
{
    if (value > max)
        return max;
    if (value < min)
        return min;
    return value;
}

qint64 XMath::rand(qint64 min, qint64 max)
{
    unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937_64 mt(seed1);
    std::uniform_int_distribution<qint64> dist(min, max);
    return dist(mt);
}
