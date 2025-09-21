#ifndef TAGMANAGER_H
#define TAGMANAGER_H

#include <QObject>
#include <QWidget>
#include <QFileDialog>

#include "lib/handler/settingshandler.h"
#include "lib/handler/loghandler.h"

#include "dialoghandler.h"

#include <ui_libraryexclusions.h>

class TagManager : public QDialog
{
    Q_OBJECT
public:
    TagManager(QWidget* parent = nullptr, bool smartTagMode = false);
    ~TagManager();
private slots:
    void on_addButton_clicked();

    void on_removeButton_clicked();

    void on_listWidget_activated(const QModelIndex &index);

private:
    Ui::LibraryExclusionsDialog ui;
    QList<int> itemsToRemove;
    bool m_modified;
    bool m_smartTagMode;
};

#endif // TAGMANAGER_H
