#ifndef LIBRARYLISTITEM_H
#define LIBRARYLISTITEM_H
#include <QString>
#include <QMetaType>
#include <QDate>
#include <QVariant>
#include <QDataStream>

enum LibraryListItemType {
    PlaylistInternal,
    Video,
    Audio,
    FunscriptType,
    VR
};

struct LibraryListItem
{
    LibraryListItemType type;
    QString path;
    QString name;
    QString nameNoExtension;
    QString script;
    QString scriptNoExtension;
    QString mediaExtension;
    QString thumbFile;
    QString zipFile;
    QDate modifiedDate;
    quint64 duration;
    friend QDataStream & operator<<( QDataStream &dataStream, const LibraryListItem &object )
    {
        dataStream << object.type;
        dataStream << object.path;
        dataStream << object.name;
        dataStream << object.nameNoExtension;
        dataStream << object.script;
        dataStream << object.scriptNoExtension;
        dataStream << object.mediaExtension;
        dataStream << object.thumbFile;
        dataStream << object.zipFile;
        dataStream << object.modifiedDate;
        dataStream << object.duration;
        return dataStream;
    }

    friend QDataStream & operator>>(QDataStream &dataStream, LibraryListItem &object)
    {
        dataStream >> object.type;
        dataStream >> object.path;
        dataStream >> object.name;
        dataStream >> object.nameNoExtension;
        dataStream >> object.script;
        dataStream >> object.scriptNoExtension;
        dataStream >> object.mediaExtension;
        dataStream >> object.thumbFile;
        dataStream >> object.zipFile;
        dataStream >> object.modifiedDate;
        dataStream >> object.duration;
        return dataStream;
    }
    friend bool operator==(const LibraryListItem &p1, const LibraryListItem &p2)
    {
       return p1.name == p2.name;
    }

};

Q_DECLARE_METATYPE(LibraryListItem);
#endif // LIBRARYLISTITEM_H
