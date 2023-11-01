#ifndef GETTEXTDIALOG_H
#define GETTEXTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>

class GetTextDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GetTextDialog(QString variableName, QString currentValue = nullptr, QWidget *parent = nullptr);
    static QString show(QWidget *parent, QString variableName, QString currentValue = nullptr, bool *ok = nullptr);

private:
    QLabel* nameLabel;
    QLineEdit* nameEdit;

    static QString m_variableName;
    static QString m_currentValue;
    static QString getText(GetTextDialog *dialog, bool *ok = nullptr);
};

#endif // GETTEXTDIALOG_H
