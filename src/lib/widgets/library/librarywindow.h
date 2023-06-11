#ifndef LIBRARYWINDOW_H
#define LIBRARYWINDOW_H
#include <QFrame>
#include <QMainWindow>
#include <QGridLayout>
#include <QResizeEvent>
#include <QMoveEvent>

#include "xtpsettings.h"

class LibraryWindow : public QFrame
{
    Q_OBJECT
signals:
    void closeWindow();
public:
    LibraryWindow(QMainWindow* parent = 0);
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent * event) override;
    void resizeEvent(QResizeEvent * event) override;
    void moveEvent(QMoveEvent * event) override;
private:
    void setDevaultSizeAndPosition();
    bool _isInitialized = false;
};

#endif // LIBRARYWINDOW_H
