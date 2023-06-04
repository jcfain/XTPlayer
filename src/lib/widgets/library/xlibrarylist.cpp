#include "xlibrarylist.h"
XLibraryList::XLibraryList(QWidget* parent) : QListView(parent)
{
    setProperty("id", "libraryList");

    setUniformItemSizes(true);
    setMovement(QListView::Static);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setTextElideMode(Qt::TextElideMode::ElideRight);
    setWordWrap(true);
    setAutoFillBackground( false );
}

LibraryListItem27 XLibraryList::selectedItem() {
    QModelIndex selectedItem = selectionModel()->currentIndex();
    return item(selectedItem);
}
int XLibraryList::selectedRow() {
    return selectionModel()->currentIndex().row();
}

void XLibraryList::setCurrentRow(int index) {
    QModelIndex indexOfTheCellIWant = model()->index(index, 0);
    this->selectionModel()->setCurrentIndex(indexOfTheCellIWant, QItemSelectionModel::SelectionFlag::SelectCurrent);
}

LibraryListItem27 XLibraryList::item(QModelIndex index) {
    return index.data(Qt::UserRole).value<LibraryListItem27>();
}

LibraryListItem27 XLibraryList::item(int index) {
    return model()->index(index, 0).data(Qt::UserRole).value<LibraryListItem27>();
}

QModelIndex XLibraryList::itemIndex(int index) {
    return model()->index(index, 0);
}
int XLibraryList::count() {
    return this->model()->rowCount();
}

void XLibraryList::updateGeometries()
{
    QListView::updateGeometries();
    verticalScrollBar()->setSingleStep(10);
}

void XLibraryList::keyPressEvent(QKeyEvent *event) {
    emit keyPressed(event);
}
void XLibraryList::keyReleaseEvent(QKeyEvent *event) {
    emit keyReleased(event);
}

LibraryListViewModel* XLibraryList::getModel() {
    auto playlistViewModel = qobject_cast<PlaylistViewModel*>(model());
    if(!playlistViewModel)
        return (LibraryListViewModel*)((LibrarySortFilterProxyModel*)model())->sourceModel();
    return playlistViewModel;
}
QSize XLibraryList::sizeHint() const
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

//void XLibraryList::dragMoveEvent(QDragMoveEvent *event)
//{
//    if (event->mimeData()->hasFormat("application/library.list.item.model")
//        && event->answerRect().intersects(geometry())) {

//        auto current = indexAt(QPoint(8, 8));
//        //setCurrentRow(current.row());
//        LogHandler::Debug("Scrolling: " +QString::number( current.row()));
//        scrollTo(current);
//        event->acceptProposedAction();
//    }
//    QListView::dragMoveEvent(event);
//}

//void XLibraryList::dropEvent( QDropEvent* e )
//{
//    if( e->source() != this )
//    {
//        // something comes from the outside
//        // what to do? return?
//        return;
//    }
//    else
//    {
//        auto current = currentIndex();
//        auto currentRow = current.row();
//        //setCurrentRow(current.row());
//        scrollTo(current);
//        QAbstractItemView::dropEvent(e);
//    }
//}
