#ifndef LIBRARYLISTITEM27_H
#define LIBRARYLISTITEM27_H
#include <QString>
#include <QMetaType>
#include <QDate>
#include <QVariant>
#include <QDataStream>
#include <QJsonObject>
#include "../lookup/Constants.h"

enum LibraryListItemType {
    PlaylistInternal,
    Video,
    Audio,
    FunscriptType,
    VR
};

class LibraryListItem27
{
public:
    //    LibraryListItem() {}
    //    LibraryListItem(const LibraryListItem &item)
    //    {
    //        type = item.type;
    //        path = item.path;
    //        name = item.name;
    //        nameNoExtension = item.nameNoExtension;
    //        script = item.script;
    //        scriptNoExtension = item.scriptNoExtension;
    //        mediaExtension = item.mediaExtension;
    //        thumbFile = item.thumbFile;
    //        zipFile = item.zipFile;
    //        modifiedDate = item.modifiedDate;
    //        duration = item.duration;
    //    }
    LibraryListItemType type;
    QString path;
    QString name;
    QString nameNoExtension;
    QString script;
    QString scriptNoExtension;
    QString mediaExtension;
    QString thumbFile;
    QString zipFile;
    QDateTime modifiedDate;
    quint64 duration;

    // Live members
    QString ID;
    bool isMFS;
    QString toolTip;
    bool thumbFileExists = false;
    QString thumbFileLoading = LibraryThumbNail::LOADING_IMAGE;// "://images/icons/loading.png";
    QString thumbFileLoadingCurrent = LibraryThumbNail::LOADING_CURRENT_IMAGE;// "://images/icons/loading_current.png";
    QString thumbFileError = LibraryThumbNail::ERROR_IMAGE;// "://images/icons/error.png";

    friend QDataStream & operator<<( QDataStream &dataStream, const LibraryListItem27 &object )
    {
        dataStream << object.ID;
        dataStream << (int)object.type;
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
        dataStream << object.isMFS;
        dataStream << object.toolTip;
        dataStream << object.thumbFileLoading;
        dataStream << object.thumbFileLoadingCurrent;
        return dataStream;
    }

    friend QDataStream & operator>>(QDataStream &dataStream, LibraryListItem27 &object)
    {
        dataStream >> object.ID;
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
        dataStream >> object.isMFS;
        dataStream >> object.toolTip;
        dataStream >> object.thumbFileLoading;
        dataStream >> object.thumbFileLoadingCurrent;
        return dataStream;
    }

    friend bool operator==(const LibraryListItem27 &p1, const LibraryListItem27 &p2)
    {
       return p1.name == p2.name;
    }

    static LibraryListItem27 fromVariant(QVariant item)
    {
        QJsonObject obj = item.toJsonObject();
        LibraryListItem27 newItem;
        //newItem.ID = obj["id"].toInt();
        newItem.path = obj["path"].toString();
        newItem.duration = obj["path"].toString().toLongLong();
        newItem.mediaExtension = obj["mediaExtension"].toString();
        newItem.modifiedDate = QDateTime::fromString(obj["modifiedDate"].toString());
        newItem.name = obj["name"].toString();
        newItem.nameNoExtension = obj["nameNoExtension"].toString();
        newItem.script = obj["script"].toString();
        newItem.scriptNoExtension = obj["scriptNoExtension"].toString();
        newItem.thumbFile = obj["thumbFile"].toString();
        newItem.type = (LibraryListItemType)obj["type"].toInt();
        newItem.zipFile = obj["zipFile"].toString();
//        newItem.isMFS = obj["isMFS"].toBool();
//        newItem.toolTip = obj["tooltip"].toString();
        return newItem;
    }

    static QVariant toVariant(LibraryListItem27 item)
    {
        QJsonObject obj;
        //obj["id"] = item.ID;
        obj["path"] = item.path;
        obj["duration"] = QString::number(item.duration);
        obj["mediaExtension"] = item.mediaExtension;;
        obj["modifiedDate"] = item.modifiedDate.toString();
        obj["name"] = item.name;
        obj["nameNoExtension"] = item.nameNoExtension;
        obj["script"] = item.script;
        obj["scriptNoExtension"] = item.scriptNoExtension;
        obj["thumbFile"] = item.thumbFile;
        obj["type"] = (int)item.type;
        obj["zipFile"] = item.zipFile;
//        obj["isMFS"] = item.isMFS;
//        obj["tooltip"] = item.toolTip;
        return QVariant::fromValue(obj);
     }
//    //waiting ? "://images/icons/loading.png" : "://images/icons/loading_current.png"
};
Q_DECLARE_METATYPE(LibraryListItem27);
#endif // LIBRARYLISTITEM27_H
