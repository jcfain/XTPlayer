#include "librarywindow.h"

LibraryWindow::LibraryWindow(QMainWindow* parent) :
    QFrame(parent)
{
    //setMinimumSize({1024, 720});
    setWindowFlags(Qt::Window);
    setLayout(new QGridLayout(this));
    _lastSize = {800, 500};
    //resize(_lastSize);
//    auto point = parent->geometry().center() - parent->rect().center();
//    setGeometry(point.rx(), point.ry(), 800, 600);
    //setGeometry(0, 0, 800, 600);
    //move(parent->geometry().center() - parent->rect().center());
}

void LibraryWindow::closeEvent(QCloseEvent *event)
{
    QFrame::closeEvent(event);
    emit closeWindow();
}

void LibraryWindow::showEvent(QShowEvent * event) {
    QFrame::showEvent(event);
    //resize(_lastSize);
    auto window = qobject_cast<QMainWindow*>(parent());
    auto point = window->geometry().center() - window->rect().center();
    setGeometry(point.rx(), point.ry(), 800, 600);
    //move(window->geometry().center() - window->rect().center());
}
void LibraryWindow::resizeEvent(QResizeEvent * event) {
    QFrame::resizeEvent(event);
    //_lastSize = size();
}
