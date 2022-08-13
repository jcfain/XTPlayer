#ifndef XLIBRARYLISTWIDGET_H
#define XLIBRARYLISTWIDGET_H

#include <QListView>
#include <QKeyEvent>
#include <QModelIndex>
#include <QScrollbar>
#include "lib/struct/LibraryListItem27.h"
#include "lib/struct/playlistviewmodel.h"
#include "lib/struct/librarysortfilterproxymodel.h"

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
    void updateGeometries() override;
protected:
    void keyPressEvent(QKeyEvent*) override;
    void keyReleaseEvent(QKeyEvent*) override;
    QSize sizeHint() const override;

    LibraryListViewModel* getModel();
};

#endif // XLIBRARYLISTWIDGET_H
