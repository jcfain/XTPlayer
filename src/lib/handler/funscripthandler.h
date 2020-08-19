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
#include "loghandler.h"

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
    FunscriptAction* getPosition(qint64 at);


private:
    void JSonToFunscript(QJsonObject jsonDoc);
    qint64 getClosest(qint64 val1, qint64 val2, qint64 target);
    qint64 findClosest(QList<qint64>, qint64 n, qint64 target);
};

#endif // FUNSCRIPTHANDLER_H
