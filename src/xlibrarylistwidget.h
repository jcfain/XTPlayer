#ifndef XLIBRARYLISTWIDGET_H
#define XLIBRARYLISTWIDGET_H

#include <QListWidget>
#include <QKeyEvent>

class XLibraryListWidget : public QListWidget
{
    Q_OBJECT
signals:
    void keyPressed(QKeyEvent* event);
    void keyReleased(QKeyEvent* event);
public:
    XLibraryListWidget(QWidget* parent = nullptr);
protected:
    virtual void keyPressEvent(QKeyEvent*);
    virtual void keyReleaseEvent(QKeyEvent*);
};

#endif // XLIBRARYLISTWIDGET_H
