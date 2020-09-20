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
#include "settingshandler.h"
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
    void load(QString funscript);
    bool isLoaded();
    void setLoaded(bool value);
    bool exists(QString path);
    std::unique_ptr<FunscriptAction> getPosition(qint64 at);


private:
    QMutex mutex;
    bool _loaded = false;
    void JSonToFunscript(QJsonObject jsonDoc);
    qint64 findClosest(qint64 value, QList<qint64> a);
};

#endif // FUNSCRIPTHANDLER_H
