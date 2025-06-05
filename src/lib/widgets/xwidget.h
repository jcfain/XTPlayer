
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
    void doubleClicked(QMouseEvent* e);
    void singleClicked(QMouseEvent* e);
    void keyPressed(QKeyEvent* k);
    void keyReleased(QKeyEvent* k);
    void mouseEnter(QEnterEvent* e);
    void mouseMove(QMouseEvent* e);

private:
    void mouseDoubleClickEvent(QMouseEvent * e) override;
    void mousePressEvent(QMouseEvent * e) override;
    void keyPressEvent(QKeyEvent * e) override;
    void keyReleaseEvent(QKeyEvent * e) override;
    void enterEvent(QEnterEvent * e) override;
    void mouseMoveEvent( QMouseEvent* e ) override;
    //void install_filter(QObject *target, QObject *filter);

};

#endif // XWIDGET_H
