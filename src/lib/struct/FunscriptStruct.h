#ifndef FUNSCRIPTSTRUCT_H
#define FUNSCRIPTSTRUCT_H

#include <QString>
#include <QVector>

struct FunscriptAction {
    int pos;
    long at;
};

struct Funscript {
    QString version;
    bool inverted;
    int range;
    QVector<FunscriptAction> actions;
};


#endif // FUNSCRIPTSTRUCT_H
