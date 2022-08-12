#ifndef LIBRARYWINDOW_H
#define LIBRARYWINDOW_H
#include <QFrame>
#include <QMainWindow>
#include <QGridLayout>

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
private:
    QSize _lastSize;
};

#endif // LIBRARYWINDOW_H
