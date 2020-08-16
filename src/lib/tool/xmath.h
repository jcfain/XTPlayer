#ifndef XMATH_H
#define XMATH_H
#include <QtGlobal>

class XMath
{
public:
    static qint64 mapRange(qint64 value, qint64 inStart, qint64 inEnd, qint64 outStart, qint64 outEnd);
};

#endif // XMATH_H
