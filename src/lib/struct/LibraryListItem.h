#ifndef LIBRARYLISTITEM_H
#define LIBRARYLISTITEM_H
#include <QString>
#include <QMetaType>
#include <QDate>

struct LibraryListItem
{
    QString path;
    QString name;
    QString nameNoExtension;
    QString script;
    QString scriptNoExtension;
    QString thumbFile;
    QString zipFile;
    QDate modifiedDate;
    quint64 duration;

};

Q_DECLARE_METATYPE(LibraryListItem);
#endif // LIBRARYLISTITEM_H
