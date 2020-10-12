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
    FunscriptHandler(QString name);
    ~FunscriptHandler();
    void load(QString funscript);
    bool isLoaded();
    void setLoaded(bool value);
    bool exists(QString path);
    Funscript* currentFunscript();
    static bool getInverted();
    static void setInverted(bool value);
    std::shared_ptr<FunscriptAction> getPosition(qint64 at);
    QString channel();


private:
    static QMutex mutexStat;
    static bool _inverted;
    QMutex mutex;
    QString _channel;
    bool _loaded = false;
    void JSonToFunscript(QJsonObject jsonDoc);
    qint64 findClosest(qint64 value, QList<qint64> a);
    qint64 lastActionIndex;
    qint64 nextActionIndex;
    QList<qint64> posList;
    Funscript* funscript = new Funscript();
    int n;
};

#endif // FUNSCRIPTHANDLER_H
