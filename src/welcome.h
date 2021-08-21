#ifndef WELCOME_H
#define WELCOME_H

#include <QDialog>

namespace Ui {
class welcome;
}

class welcome : public QDialog
{
    Q_OBJECT

public:
    explicit welcome(QWidget *parent = nullptr);
    ~welcome();

private:
    Ui::welcome *ui;
};

#endif // WELCOME_H
