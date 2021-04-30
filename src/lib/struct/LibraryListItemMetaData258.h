#ifndef LIBRARYLISTITEMMETADATA258_H
#define LIBRARYLISTITEMMETADATA258_H
#include <QString>
#include <QMetaType>
#include <QDate>
#include <QVariant>
#include <QDataStream>
#include "Bookmark.h"

struct LibraryListItemMetaData258
{
    QString libraryItemPath;
    qint64 lastPlayPosition;
    bool lastLoopEnabled;
    int lastLoopStart;
    int lastLoopEnd;
    int offset;
    qint64 moneyShotMillis;
    QList<Bookmark> bookmarks;
    QList<QString> funscripts;

    friend QDataStream & operator<<(QDataStream &dataStream, const LibraryListItemMetaData258 &object )
    {
        dataStream << object.libraryItemPath;
        dataStream << object.lastPlayPosition;
        dataStream << object.lastLoopEnabled;
        dataStream << object.lastLoopStart;
        dataStream << object.lastLoopEnd;
        dataStream << object.offset;
        dataStream << object.moneyShotMillis;
        foreach(auto bookmark, object.bookmarks )
            dataStream << bookmark;
        foreach(auto funscript, object.funscripts )
            dataStream << funscript;
        return dataStream;
    }

    friend QDataStream & operator>>(QDataStream &dataStream, LibraryListItemMetaData258 &object)
    {
        dataStream >> object.libraryItemPath;
        dataStream >> object.lastPlayPosition;
        dataStream >> object.lastLoopEnabled;
        dataStream >> object.lastLoopStart;
        dataStream >> object.lastLoopEnd;
        dataStream >> object.offset;
        dataStream >> object.moneyShotMillis;
        foreach(auto bookmark, object.bookmarks )
            dataStream >> bookmark;
        foreach(auto funscript, object.funscripts )
            dataStream >> funscript;
        return dataStream;
    }
    friend bool operator==(const LibraryListItemMetaData258 &p1, const LibraryListItemMetaData258 &p2)
    {
       return p1.libraryItemPath == p2.libraryItemPath;
    }

};

Q_DECLARE_METATYPE(LibraryListItemMetaData258);
#endif // LIBRARYLISTITEMMETADATA_H
