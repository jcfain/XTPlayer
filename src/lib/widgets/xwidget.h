
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
    //bool eventFilter(QObject *watched, QEvent *event) override;
    //void init();

signals:
    void mouseMove(QMouseEvent* e);

private:
    void mouseMoveEvent( QMouseEvent* e ) override;
    //void install_filter(QObject *target, QObject *filter);

};

#endif // XWIDGET_H
