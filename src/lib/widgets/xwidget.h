
#ifndef XWIDGET_H
#define XWIDGET_H


#include <QWidget>
#include <QGraphicsEffect>
#include <QMouseEvent>


class XWidget : public QWidget
{
    Q_OBJECT
public:
    explicit XWidget(QWidget *parent = nullptr);
    void mouseMoveEvent( QMouseEvent* e ) override;
    //bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void mouseMove(QMouseEvent* e);

private:
    //void install_filter(QObject *target, QObject *filter);

};

#endif // XWIDGET_H
