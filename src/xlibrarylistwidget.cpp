#include "xlibrarylistwidget.h"
XLibraryListWidget::XLibraryListWidget(QWidget* parent) : QListView(parent)
{
    setProperty("id", "libraryList");

    setUniformItemSizes(true);
    setMovement(QListView::Static);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setTextElideMode(Qt::TextElideMode::ElideRight);
    setWordWrap(true);
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

void XLibraryListWidget::updateGeometries()
{
    QListView::updateGeometries();
    verticalScrollBar()->setSingleStep(5);
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
QSize XLibraryListWidget::sizeHint() const
{
    QSize hint = QListView::sizeHint();
    if (model()) {
        // Determine the vertical space allocated beyond the viewport
        const int extraHeight = height() - viewport()->height();

        // Find the bounding rect of the last list item
        const QModelIndex index = model()->index(model()->rowCount() - 1, modelColumn());
        const QRect r = visualRect(index);

        // Size the widget to the height of the bottom of the last item
        // plus the extra determined earlier
        hint.setHeight(r.y() + r.height() + extraHeight);
    }
    return hint;
//    if (model()->rowCount() == 0) return QSize(width(), 0);
//    int nToShow = _nItemsToShow < model()->rowCount() ? _nItemsToShow : model()->rowCount();
//    return QSize(width(), nToShow*sizeHintForRow(0));
}
