#ifndef FUNSCRIPTSTRUCT_H
#define FUNSCRIPTSTRUCT_H

#include <QString>
#include <QHash>

struct FunscriptAction {
    qint64 at;
    int pos;
    int speed;
};

struct Funscript {
    QString version;
    bool inverted;
    int range;
    QHash<qint64, int> actions;
};


#endif // FUNSCRIPTSTRUCT_H
