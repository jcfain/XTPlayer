#include "xlibrarylistwidget.h"
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
    QModelIndex indexOfTheCellIWant = model()->index(index, 0);
    this->selectionModel()->setCurrentIndex(indexOfTheCellIWant, QItemSelectionModel::SelectionFlag::SelectCurrent);
}

LibraryListItem27 XLibraryListWidget::item(QModelIndex index) {
    return index.data(Qt::UserRole).value<LibraryListItem27>();
}

LibraryListItem27 XLibraryListWidget::item(int index) {
    return model()->index(index, 0).data(Qt::UserRole).value<LibraryListItem27>();
}
QModelIndex XLibraryListWidget::itemIndex(int index) {
    return model()->index(index, 0);
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

LibraryListViewModel* XLibraryListWidget::getModel() {
    auto playlistViewModel = qobject_cast<PlaylistViewModel*>(model());
    if(!playlistViewModel)
        return (LibraryListViewModel*)((LibrarySortFilterProxyModel*)model())->sourceModel();
    return playlistViewModel;
}
