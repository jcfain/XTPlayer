
#ifndef XWIDGET_H
#define XWIDGET_H


#include <QWidget>
#include <QGraphicsEffect>


class XWidget : public QWidget
{
    Q_OBJECT
public:
    explicit XWidget(QWidget *parent = nullptr);
    void mouseMoveEvent( QMouseEvent* e ) override;

signals:
    void mouseMove(QMouseEvent* e);

};

#endif // XWIDGET_H
