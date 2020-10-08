#include "xmath.h"

int XMath::mapRange(int value, int inStart, int inEnd, int outStart, int outEnd)
{
    double slope = (double)(outEnd - outStart) / (inEnd - inStart);
    return qRound(outStart + slope * (value - inStart));
}

qint64 XMath::mapRange(qint64 value, qint64 inStart, qint64 inEnd, qint64 outStart, qint64 outEnd)
{
    double slope = (double)(outEnd - outStart) / (inEnd - inStart);
    return qRound64(outStart + slope * (value - inStart));
}

qint64 XMath::mapRange(double value, double inStart, double inEnd, int outStart, int outEnd)
{
    double slope = (outEnd - outStart) / (inEnd - inStart);
    return qRound(outStart + slope * (value - inStart));
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

double XMath::rand(double min, double max)
{
    unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937_64 mt(seed1);
    std::uniform_real_distribution<double> dist(min, max);
    return dist(mt);
}

int XMath::randSine(double angle)
{
    int amplitude = rand(0, 100.0);
//    LogHandler::Debug("amplitude: "+ QString::number(amplitude));
//    LogHandler::Debug("angle: "+ QString::number(angle));
    int value = int(amplitude * sin(angle));
    return value < 0 ? -value : value;
}

int XMath::reverseNumber(int num, int min, int max) {
    return (max + min) - num;
}

