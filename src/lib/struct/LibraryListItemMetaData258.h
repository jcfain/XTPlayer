#ifndef LIBRARYLISTITEMMETADATA258_H
#define LIBRARYLISTITEMMETADATA258_H
#include <QString>
#include <QMetaType>
#include <QDate>
#include <QVariant>
#include <QDataStream>
#include <QJsonObject>
#include <QJsonArray>
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

    static LibraryListItemMetaData258 fromVariant(QVariant item)
    {
        QJsonObject obj = item.toJsonObject();
        LibraryListItemMetaData258 newItem;
        newItem.libraryItemPath = obj["libraryItemPath"].toString();
        newItem.lastPlayPosition = obj["lastPlayPosition"].toString().toLongLong();
        newItem.lastLoopEnabled = obj["lastLoopEnabled"].toBool();
        newItem.lastLoopStart = obj["lastLoopStart"].toInt();
        newItem.lastLoopEnd = obj["lastLoopEnd"].toInt();
        newItem.offset = obj["offset"].toInt();
        newItem.moneyShotMillis = obj["moneyShotMillis"].toString().toLongLong();
        foreach(auto bookmark, obj["bookmarks"].toArray())
        {
            Bookmark bookMark;
            bookMark.Name = bookmark["Name"].toString();
            bookMark.Time = bookmark["Time"].toString().toLongLong();
            newItem.bookmarks.append(bookMark);
        }
        foreach(auto funscript, obj["funscripts"].toArray())
        {
            newItem.funscripts.append(funscript.toString());
        }
        return newItem;
    }

    static QVariant toVariant(LibraryListItemMetaData258 item)
    {
        QJsonObject obj;
        obj["libraryItemPath"] = item.libraryItemPath;
        obj["lastPlayPosition"] = QString::number(item.lastPlayPosition);
        obj["lastLoopEnabled"] = item.lastLoopEnabled;
        obj["lastLoopStart"] = item.lastLoopStart;
        obj["lastLoopEnd"] = item.lastLoopEnd;
        obj["offset"] = item.offset;
        obj["moneyShotMillis"] = QString::number(item.moneyShotMillis);
        QJsonArray bookmarks;
        foreach(Bookmark bookmark, item.bookmarks) {
            QJsonObject bookmarkObj;
            bookmarkObj["Name"] = bookmark.Name;
            bookmarkObj["Time"] = QString::number(bookmark.Time);
            bookmarks.append(bookmarkObj);
        }
        QJsonArray funscripts;
        foreach(QString funscript, item.funscripts) {
            funscripts.append(funscript);
        }
        return QVariant::fromValue(obj);
     }
};

Q_DECLARE_METATYPE(LibraryListItemMetaData258);
#endif // LIBRARYLISTITEMMETADATA_H
