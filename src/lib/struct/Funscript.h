#ifndef FUNSCRIPTSTRUCT_H
#define FUNSCRIPTSTRUCT_H

#include <QString>
#include <QHash>

struct Funscript {
    QString version;
    bool inverted;
    int range;
    QHash<qint64, int> actions;
};


#endif // FUNSCRIPTSTRUCT_H
