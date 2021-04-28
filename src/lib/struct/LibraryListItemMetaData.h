#ifndef LIBRARYLISTITEMMETADATA_H
#define LIBRARYLISTITEMMETADATA_H
#include <QString>
#include <QMetaType>
#include <QDate>
#include <QVariant>
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

struct LibraryListItemMetaData
{
    QString libraryItemPath;
    qint64 lastPlayPosition;
    bool lastLoopEnabled;
    int lastLoopStart;
    int lastLoopEnd;
    qint64 moneyShotMillis;
    QList<Bookmark> bookmarks;
    QList<QString> funscripts;

    friend QDataStream & operator<<(QDataStream &dataStream, const LibraryListItemMetaData &object )
    {
        dataStream << object.libraryItemPath;
        dataStream << object.lastPlayPosition;
        dataStream << object.lastLoopEnabled;
        dataStream << object.lastLoopStart;
        dataStream << object.lastLoopEnd;
        dataStream << object.moneyShotMillis;
        foreach(auto bookmark, object.bookmarks )
            dataStream << bookmark;
        foreach(auto funscript, object.funscripts )
            dataStream << funscript;
        return dataStream;
    }

    friend QDataStream & operator>>(QDataStream &dataStream, LibraryListItemMetaData &object)
    {
        dataStream >> object.libraryItemPath;
        dataStream >> object.lastPlayPosition;
        dataStream >> object.lastLoopEnabled;
        dataStream >> object.lastLoopStart;
        dataStream >> object.lastLoopEnd;
        dataStream >> object.moneyShotMillis;
        foreach(auto bookmark, object.bookmarks )
            dataStream >> bookmark;
        foreach(auto funscript, object.funscripts )
            dataStream >> funscript;
        return dataStream;
    }
    friend bool operator==(const LibraryListItemMetaData &p1, const LibraryListItemMetaData &p2)
    {
       return p1.libraryItemPath == p2.libraryItemPath;
    }

};

Q_DECLARE_METATYPE(LibraryListItemMetaData);
Q_DECLARE_METATYPE(Bookmark);
#endif // LIBRARYLISTITEMMETADATA_H
