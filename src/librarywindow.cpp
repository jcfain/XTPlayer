#include "librarywindow.h"
LibraryWindow::LibraryWindow(QMainWindow* parent) :
    QFrame(parent)
{
    //setMinimumSize({1024, 720});
    setWindowFlags(Qt::Window);
    setLayout(new QGridLayout(this));
    //resize(_lastSize);
//    auto point = parent->geometry().center() - parent->rect().center();
//    setGeometry(point.rx(), point.ry(), 800, 600);
    //setGeometry(0, 0, 800, 600);
    //move(parent->geometry().center() - parent->rect().center());
}

void LibraryWindow::closeEvent(QCloseEvent *event)
{
    emit closeWindow();
    QFrame::closeEvent(event);
}

void LibraryWindow::showEvent(QShowEvent * event) {
    QFrame::showEvent(event);
    if(XTPSettings::getRememberWindowsSettings()) {
        auto pos = XTPSettings::getLibraryWindowPosition();
        if(!pos.isNull()) {
            move(pos);
        }
        auto size = XTPSettings::getLibraryWindowSize();
        if(!size.isNull()) {
            resize(size);
        } else
            setDevaultSizeAndPosition();
    } else {
        setDevaultSizeAndPosition();
    }
    _isInitialized = true;
}
void LibraryWindow::resizeEvent(QResizeEvent* event) {
    QFrame::resizeEvent(event);
    if(_isInitialized)
        XTPSettings::setLibraryWindowSize(event->size());
}

void LibraryWindow::moveEvent(QMoveEvent* event) {
    QFrame::moveEvent(event);
    if(_isInitialized)
        XTPSettings::setLibraryWindowPosition(event->pos());
}

void LibraryWindow::setDevaultSizeAndPosition() {
    auto window = qobject_cast<QMainWindow*>(parent());
    auto point = window->geometry().center() - window->rect().center();
    setGeometry(point.rx(), point.ry(), 800, 600);
}
