#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QString>
#include <QDataStream>

struct Bookmark
{
    QString Name;
    qint64 Time;

    friend QDataStream & operator<<(QDataStream &dataStream, const Bookmark &object )
    {
        dataStream << object.Name;
        dataStream << object.Time;
        return dataStream;
    }
    friend QDataStream & operator>>(QDataStream &dataStream, Bookmark &object)
    {
        dataStream >> object.Name;
        dataStream >> object.Time;
        return dataStream;
    }
};

#endif // BOOKMARK_H
