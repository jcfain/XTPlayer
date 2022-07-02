#ifndef DLNASCRIPTLINKS_H
#define DLNASCRIPTLINKS_H

#include <QObject>
#include <QWidget>
#include <QFileDialog>
#include <QPushButton>
#include <QMessageBox>
#include <lib/handler/settingshandler.h>
#include <lib/handler/loghandler.h>
#include <ui_dlnascriptlinks.h>

class DLNAScriptLinks : public QDialog
{
    Q_OBJECT
public:
    DLNAScriptLinks(QWidget* parent = nullptr);
    ~DLNAScriptLinks();
    void showDialog();
private slots:
    void on_tableWidget_Changed(QTableWidgetItem *item);

    void on_buttonBox_accepted();

    void on_buttonBox_clicked(QAbstractButton *button);

    void on_tableWidget_itemSelectionChanged();

private:
    Ui::DLNAScriptLinksDialog ui;
    QList<int> itemsToRemove;
    QPushButton* _saveButton;
    QPushButton* _resetButton;
    QPushButton* _discardButton;
    QHash<QString, QVariant> _editedLinks;
    QHash<QString, QVariant> _removedLinks;

    enum _COLUMNS {
        SCRIPT_NAME,
        SCRIPT_PATH,
        MEDIA_PATH,
        SCRIPT_SELECTION
    };

    void setUpData();
    void clearData();
};

#endif // DLNASCRIPTLINKS_H
