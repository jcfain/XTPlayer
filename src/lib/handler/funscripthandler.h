#ifndef FUNSCRIPTHANDLER_H
#define FUNSCRIPTHANDLER_H
#include <QString>
#include <QJsonObject>
#include "../struct/FunscriptStruct.h"


class FunscriptHandler
{
public:
    FunscriptHandler();
    bool load(QString funscript);
    bool exists(QString path);

    ~FunscriptHandler()
    {
        delete funscript;
    }

private:
    Funscript* JSonToFunscript(QJsonObject jsonDoc);
    Funscript* funscript;
};

#endif // FUNSCRIPTHANDLER_H
