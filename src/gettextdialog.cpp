#include "gettextdialog.h"

#include <QDialogButtonBox>
#include <QGridLayout>

#include "lib/handler/loghandler.h"

#include "dialoghandler.h"

GetTextDialog::GetTextDialog(QStringList variableNames, QStringList currentValues, QWidget *parent) : QDialog(parent)
{
    QGridLayout* layout = new QGridLayout(this);
    m_variableNames = variableNames;
    m_currentValues = currentValues;
    int rows = 0;
    for (int i=0; i<variableNames.length(); i++)
    {
        QString name = variableNames[i];
        QLabel* nameLabel = new QLabel(this);
        nameLabel->setText(name);
        QLineEdit* nameEdit = new QLineEdit(this);
        nameEdit->setText(m_currentValues.isEmpty() || m_currentValues.length() < i ? "" : m_currentValues[i]);
        if(i == 0)
            nameEdit->setFocus();
        layout->addWidget(nameLabel, rows, 0, 1, 1);
        layout->addWidget(nameEdit, rows, 1, 1, 1);
        nameLabels.append(nameLabel);
        nameEdits.append(nameEdit);
        rows++;
    }
    setMinimumSize(300, 100);
    QDialogButtonBox *buttonBox = new QDialogButtonBox
            ( QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
              Qt::Horizontal, this );
    layout->addWidget(buttonBox, rows, 0, 1, 2);

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
    QStringList currentValues;
    if(!currentValue.isNull())
        currentValues << currentValue;
    GetTextDialog *dialog = new GetTextDialog({variableName}, currentValues, parent);
    QStringList values =  getText(dialog, ok);
    return values.empty() ? nullptr : values.first();
}

///
/// \brief GetTextDialog::show Takes in a set of labels and values
/// currentValues length must be less than or equal variableNames length
/// All values are required to be not empty.
/// \param parent
/// \param variableNames
/// \param currentValues
/// \param ok
/// \return list of values equal to the length of variableNames
///
QStringList GetTextDialog::show(QWidget *parent, QStringList variableNames, QStringList currentValues, bool *ok)
{
    GetTextDialog *dialog = new GetTextDialog(variableNames, currentValues, parent);
    return getText(dialog, ok);
}

QStringList GetTextDialog::getText(GetTextDialog *dialog, bool *ok)
{
    const int ret = dialog->exec();
    QStringList returnValues;
    if (ok)
        *ok = !!ret;
    if (ret == QDialog::Accepted)
    {
        bool isValid = true;
        for (int i=0; i<dialog->nameEdits.length(); i++)
        {
            QString value = dialog->nameEdits[i]->text();
            if(value.isEmpty())
            {
                isValid = false;
                DialogHandler::MessageBox(dialog, dialog->nameLabels[i]->text() + tr(" required!"), XLogLevel::Critical);
                break;
            }
            returnValues.append(value);
        }
        if (!isValid && ok)
            *ok = false;
    }
    m_variableNames.clear();
    m_currentValues.clear();

    dialog->deleteLater();

    return ret == QDialog::Accepted ? returnValues : QStringList();
}

QStringList GetTextDialog::m_variableNames;
QStringList GetTextDialog::m_currentValues;
