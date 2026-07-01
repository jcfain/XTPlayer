
#ifndef XCOMBOBOX_H
#define XCOMBOBOX_H


#include <QComboBox>
#include <QMouseEvent>


class XComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit XComboBox(QWidget *parent = nullptr);


private:
    void wheelEvent(QWheelEvent *e) override;

};

#endif // XCOMBOBOX_H
