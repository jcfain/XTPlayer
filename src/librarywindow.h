#ifndef LIBRARYWINDOW_H
#define LIBRARYWINDOW_H
#include <QFrame>
#include <QMainWindow>
#include <QGridLayout>

class LibraryWindow : public QFrame
{
    Q_OBJECT
signals:
    void close();
public:
    LibraryWindow(QMainWindow* parent = 0);
    void closeEvent(QCloseEvent *event) override;
};

#endif // LIBRARYWINDOW_H
