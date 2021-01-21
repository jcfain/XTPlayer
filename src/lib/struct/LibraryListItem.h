#ifndef LIBRARYLISTITEM_H
#define LIBRARYLISTITEM_H
#include <QString>
#include <QMetaType>
#include <QDate>
#include <QVariant>

struct LibraryListItem
{
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
    bool audioOnly;
    friend QDataStream & operator<<( QDataStream &dataStream, const LibraryListItem &object )
    {
        dataStream << object.path;
        dataStream << object.name;
        dataStream << object.nameNoExtension;
        dataStream << object.script;
        dataStream << object.scriptNoExtension;
        dataStream << object.mediaExtension;
        dataStream << object.thumbFile;
        dataStream << object.zipFile;
        dataStream << object.modifiedDate;
        //dataStream << QVariant::fromValue(object.duration);
        //dataStream << object.audioOnly;
        return dataStream;
    }

    friend QDataStream & operator>>(QDataStream &dataStream, LibraryListItem &object)
    {
        dataStream >> object.path;
        dataStream >> object.name;
        dataStream >> object.nameNoExtension;
        dataStream >> object.script;
        dataStream >> object.scriptNoExtension;
        dataStream >> object.mediaExtension;
        dataStream >> object.thumbFile;
        dataStream >> object.zipFile;
        dataStream >> object.modifiedDate;
        //dataStream >> object.duration;
        //dataStream >> object.audioOnly;
        return dataStream;
    }

};

Q_DECLARE_METATYPE(LibraryListItem);
#endif // LIBRARYLISTITEM_H
