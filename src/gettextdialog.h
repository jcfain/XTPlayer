#ifndef GETTEXTDIALOG_H
#define GETTEXTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>

class GetTextDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GetTextDialog(QStringList variableNames, QStringList currentValue = QStringList(), QWidget *parent = nullptr);
    static QString show(QWidget *parent, QString variableName, QString currentValue = nullptr, bool *ok = nullptr);
    static QStringList show(QWidget *parent, QStringList variableNames, QStringList currentValues = QStringList(), bool *ok = nullptr);

private:
    QList<QLabel*> nameLabels;
    QList<QLineEdit*> nameEdits;

    static QStringList m_variableNames;
    static QStringList m_currentValues;
    static QStringList getText(GetTextDialog *dialog, bool *ok = nullptr);
};

#endif // GETTEXTDIALOG_H
