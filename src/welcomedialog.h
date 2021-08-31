#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <QDialog>
#include "lib/handler/settingshandler.h"

namespace Ui {
class welcomedialog;
}

class WelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WelcomeDialog(QWidget *parent = nullptr);
    ~WelcomeDialog();

private slots:
    void on_doNotShowAgainCheckBox_toggled(bool checked);

    void on_welcomeNext_clicked();

    void on_welcomePrevious_clicked();

    void on_buttonBox_accepted();

private:
    Ui::welcomedialog *ui;
    int currentIndex = 1;
    int maxIndex = 6;

    void loadImage(int index);
};

#endif // WELCOMEDIALOG_H
