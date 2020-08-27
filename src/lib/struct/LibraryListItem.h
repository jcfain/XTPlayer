#ifndef LIBRARYLISTITEM_H
#define LIBRARYLISTITEM_H
#include <QString>
#include <QMetaType>

struct LibraryListItem
{
    QString path;
    QString name;
    QString script;
};

Q_DECLARE_METATYPE(LibraryListItem);
#endif // LIBRARYLISTITEM_H
