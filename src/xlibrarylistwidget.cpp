#include "xlibrarylistwidget.h"
#include "lib/struct/librarylistviewmodel.h"
XLibraryListWidget::XLibraryListWidget(QWidget* parent) : QListView(parent)
{

}

LibraryListItem27 XLibraryListWidget::selectedItem() {
    QModelIndex selectedItem = selectionModel()->currentIndex();
    return item(selectedItem);
}
int XLibraryListWidget::selectedRow() {
    return selectionModel()->currentIndex().row();
}

void XLibraryListWidget::setCurrentRow(int index) {
    LibraryListViewModel* listModel = qobject_cast<LibraryListViewModel*>(model());
    QModelIndex indexOfTheCellIWant = listModel->index(index, 0);
    this->selectionModel()->setCurrentIndex(indexOfTheCellIWant, QItemSelectionModel::SelectionFlag::Current);
}

LibraryListItem27 XLibraryListWidget::item(QModelIndex index) {
    LibraryListViewModel* listModel = qobject_cast<LibraryListViewModel*>(model());
    return listModel->getItem(index);
}

LibraryListItem27 XLibraryListWidget::item(int index) {
    LibraryListViewModel* listModel = qobject_cast<LibraryListViewModel*>(model());
    return listModel->getItem(index);
}
QModelIndex XLibraryListWidget::itemIndex(int index) {
    LibraryListViewModel* listModel = qobject_cast<LibraryListViewModel*>(model());
    return listModel->index(index, 0);
}
int XLibraryListWidget::count() {
    return this->model()->rowCount();
}

void XLibraryListWidget::keyPressEvent(QKeyEvent *event) {
    emit keyPressed(event);
}
void XLibraryListWidget::keyReleaseEvent(QKeyEvent *event) {
    emit keyReleased(event);
}
