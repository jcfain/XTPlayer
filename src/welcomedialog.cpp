#include "welcomedialog.h"
#include "ui_welcomedialog.h"

WelcomeDialog::WelcomeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::welcomedialog)
{
    ui->setupUi(this);
    setModal(false);
#ifdef Q_OS_MAC
    setModal(true);// Less than Ideal workaround...
    //setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    //raise();
#endif

}

WelcomeDialog::~WelcomeDialog()
{
    emit close();
    delete ui;
}
void WelcomeDialog::showEvent(QShowEvent *ev)
{
    QDialog::showEvent(ev);
    loadImage(currentIndex);
}
void WelcomeDialog::on_doNotShowAgainCheckBox_toggled(bool checked)
{
    SettingsHandler::setHideWelcomeScreen(checked);
}

void WelcomeDialog::on_welcomeNext_clicked()
{
    currentIndex++;
    if(currentIndex > maxIndex)
        currentIndex = 1;
    loadImage(currentIndex);
}

void WelcomeDialog::on_welcomePrevious_clicked()
{
    currentIndex--;
    if(currentIndex < 1)
        currentIndex = maxIndex;
    loadImage(currentIndex);
}


void WelcomeDialog::loadImage(int index)
{
    QPixmap pixmap("://images/welcome/XTP_welcome-"+QString::number(index)+".png");
    int w = ui->welcomeImageLabel->width();
    int h = ui->welcomeImageLabel->height();
    ui->welcomeImageLabel->setAlignment(Qt::AlignCenter);
    // set a scaled pixmap to a w x h window keeping its aspect ratio
    ui->welcomeImageLabel->setPixmap(pixmap.scaled(w,h,Qt::KeepAspectRatio));
    pixmap = QPixmap();
}
void WelcomeDialog::on_buttonBox_accepted()
{
    //close();
}
