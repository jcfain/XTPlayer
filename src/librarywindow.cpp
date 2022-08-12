#include "librarywindow.h"

LibraryWindow::LibraryWindow(QMainWindow* parent) :
    QFrame(parent)
{
    //setMinimumSize({1024, 720});
    setWindowFlags(Qt::Window);
    setLayout(new QGridLayout(this));
    _lastSize = {800, 500};
    //resize(_lastSize);
    setGeometry(0, 0, 800, 600);
    move(parent->geometry().center() - parent->rect().center());
}

void LibraryWindow::closeEvent(QCloseEvent *event)
{
    QFrame::closeEvent(event);
    emit closeWindow();
}

void LibraryWindow::showEvent(QShowEvent * event) {
    QFrame::showEvent(event);
    //resize(_lastSize);
}
void LibraryWindow::resizeEvent(QResizeEvent * event) {
    QFrame::resizeEvent(event);
    //_lastSize = size();
}
