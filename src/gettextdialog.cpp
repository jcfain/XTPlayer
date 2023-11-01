#include "gettextdialog.h"

#include <QDialogButtonBox>
#include <QGridLayout>

#include "lib/handler/loghandler.h"

#include "dialoghandler.h"

GetTextDialog::GetTextDialog(QString variableName, QString currentValue, QWidget *parent) : QDialog(parent)
{
    m_variableName = variableName;
    m_currentValue = currentValue;
    nameLabel = new QLabel(this);
    nameLabel->setText(m_variableName);
    nameEdit = new QLineEdit(this);
    nameEdit->setText(m_currentValue.isEmpty() ? "" : m_currentValue);
    nameEdit->setFocus();
    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(nameLabel, 0, 0, 1, 1);
    layout->addWidget(nameEdit, 0, 1, 1, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox
            ( QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
              Qt::Horizontal, this );
    layout->addWidget(buttonBox, 1, 0, 1, 2);

    bool conn = connect(buttonBox, &QDialogButtonBox::accepted,
                   this, &GetTextDialog::accept);
    Q_ASSERT(conn);
    conn = connect(buttonBox, &QDialogButtonBox::rejected,
                   this, &GetTextDialog::reject);
    Q_ASSERT(conn);
    setLayout(layout);
}

QString GetTextDialog::show(QWidget *parent, QString variableName, QString currentValue, bool *ok)
{
    GetTextDialog *dialog = new GetTextDialog(variableName, currentValue, parent);
    return getText(dialog, ok);
}

QString GetTextDialog::getText(GetTextDialog *dialog, bool *ok)
{
    const int ret = dialog->exec();
    if (ok)
        *ok = !!ret;
    if (ok)
    {
        bool isValid = true;
        if(dialog->nameEdit->text().isEmpty())
        {
            isValid = false;
            DialogHandler::MessageBox(dialog, m_variableName+" required!", XLogLevel::Critical);
        }
        if (!isValid)
            *ok = false;
    }
    m_variableName = nullptr;
    m_currentValue = nullptr;

    dialog->deleteLater();

    return ok ? dialog->nameEdit->text() : nullptr;
}

QString GetTextDialog::m_variableName;
QString GetTextDialog::m_currentValue;
