#ifndef FUNSCRIPTHANDLER_H
#define FUNSCRIPTHANDLER_H
#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QIODevice>
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>

#include "../struct/Funscript.h"

class FunscriptHandler : public QObject
{
    Q_OBJECT

signals:
    void errorOccurred(QString error);

public:
    FunscriptHandler();
    ~FunscriptHandler();
    bool load(QString funscript);
    bool exists(QString path);
    int getPosition(qint64 at);


private:
    void JSonToFunscript(QJsonObject jsonDoc);
};

#endif // FUNSCRIPTHANDLER_H
