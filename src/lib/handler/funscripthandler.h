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

public:
    FunscriptHandler(QString name);
    ~FunscriptHandler();
    bool load(QString funscript);
    bool load(QByteArray funscript);
    bool isLoaded();
    void setLoaded(bool value);
    bool exists(QString path);
    Funscript* currentFunscript();
    static bool getInverted();
    static void setInverted(bool value);
    qint64 getMin();
    qint64 getMax();
    std::shared_ptr<FunscriptAction> getPosition(qint64 at);
    QString channel();

    void play(QString funscript);
    void stop();


private:
    static QMutex mutexStat;
    static bool _inverted;
    QMutex mutex;
    QString _channel;
    bool _loaded = false;
    bool _firstActionExecuted;
    void JSonToFunscript(QJsonObject jsonDoc);
    qint64 findClosest(qint64 value, QList<qint64> a);
    qint64 lastActionIndex;
    qint64 nextActionIndex;
    int lastActionPos;
    int lastActionSpeed;
    QList<qint64> posList;
    Funscript* funscript = new Funscript();
    int n;
    qint64 _funscriptMin = 0;
    qint64 _funscriptMax = -1;
};

#endif // FUNSCRIPTHANDLER_H
