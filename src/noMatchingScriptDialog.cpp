#include "noMatchingScriptDialog.h"

NoMatchingScriptDialog::NoMatchingScriptDialog(QWidget* parent, QString customScript) : QDialog(parent)
{
    label = new QLabel(this);
    label->setText(tr("Error loading script ") + customScript + tr("!\nTry right clicking on the video in the list and loading with another script.\n\n\nNote: media items that have gray titles have no matching script.\n\n"));
    checkbox = new QCheckBox(this);
    checkbox->setText("Dont show again!");
    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(label, 0, 0, 1, 2);
    layout->addWidget(checkbox, 1, 0, 1, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox
            ( QDialogButtonBox::Ok, Qt::Horizontal, this );
    layout->addWidget(buttonBox, 1, 1, 1, 1);

    bool conn = connect(buttonBox, &QDialogButtonBox::accepted,
                   this, &NoMatchingScriptDialog::accept);
    Q_ASSERT(conn);
    setLayout(layout);
}

void NoMatchingScriptDialog::show(QWidget *parent, QString customScript)
{
    NoMatchingScriptDialog *dialog = new NoMatchingScriptDialog(parent, customScript);
    showDialog(dialog);
}

void NoMatchingScriptDialog::showDialog(NoMatchingScriptDialog *dialog)
{
    const int ret = dialog->exec();
    if (ret)
    {
        SettingsHandler::setDisableNoScriptFound(dialog->checkbox->isChecked());
    }
    dialog->deleteLater();
}
