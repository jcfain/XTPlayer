#ifndef LIBRARYEXCLUSIONS_H
#define LIBRARYEXCLUSIONS_H

#include <QObject>
#include <QWidget>
#include <QFileDialog>
#include <lib/handler/settingshandler.h>
#include <lib/handler/loghandler.h>
#include <ui_libraryexclusions.h>

class LibraryExclusions : public QDialog
{
    Q_OBJECT
public:
    LibraryExclusions(QWidget* parent = nullptr);
    ~LibraryExclusions();
private slots:
    void on_addButton_clicked();

    void on_removeButton_clicked();

    void on_listWidget_activated(const QModelIndex &index);

private:
    Ui::LibraryExclusionsDialog ui;
    QList<int> itemsToRemove;
};

#endif // LIBRARYEXCLUSIONS_H
