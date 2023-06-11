#ifndef LIBRARYMANAGER_H
#define LIBRARYMANAGER_H

#include <QObject>
#include <QWidget>
#include <QFileDialog>

#include "lib/handler/settingshandler.h"
#include "lib/handler/loghandler.h"

#include "dialoghandler.h"

#include <ui_libraryexclusions.h>

class LibraryManager : public QDialog
{
    Q_OBJECT
public:
    LibraryManager(QWidget* parent = nullptr);
    ~LibraryManager();
private slots:
    void on_addButton_clicked();

    void on_removeButton_clicked();

    void on_listWidget_activated(const QModelIndex &index);

private:
    Ui::LibraryExclusionsDialog ui;
    QList<int> itemsToRemove;
};

#endif // LIBRARYMANAGER_H
