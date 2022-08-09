#ifndef XLIBRARYLISTWIDGET_H
#define XLIBRARYLISTWIDGET_H

#include <QListView>
#include <QKeyEvent>
#include <QModelIndex>
#include "lib/struct/LibraryListItem27.h"

class XLibraryListWidget : public QListView
{
    Q_OBJECT
signals:
    void keyPressed(QKeyEvent* event);
    void keyReleased(QKeyEvent* event);
public:
    XLibraryListWidget(QWidget* parent = nullptr);
    int count();
    LibraryListItem27 selectedItem();
    int selectedRow();
    void setCurrentRow(int index);
    LibraryListItem27 item(QModelIndex index);
    LibraryListItem27 item(int index);
    QModelIndex itemIndex(int index);
protected:
    virtual void keyPressEvent(QKeyEvent*);
    virtual void keyReleaseEvent(QKeyEvent*);
};

#endif // XLIBRARYLISTWIDGET_H
