#ifndef XMATH_H
#define XMATH_H
#include <QtGlobal>

class XMath
{
public:
    static int mapRange(int value, int inStart, int inEnd, int outStart, int outEnd);
    static qint64 mapRange(qint64 value, qint64 inStart, qint64 inEnd, qint64 outStart, qint64 outEnd);
    static qint64 mapRange(double value, double inStart, double inEnd, int outStart, int outEnd);
    static int constrain(int value, int min, int max);
    static qint64 rand(qint64 min, qint64 max);
};

#endif // XMATH_H
