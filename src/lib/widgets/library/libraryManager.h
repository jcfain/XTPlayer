#ifndef LIBRARYMANAGER_H
#define LIBRARYMANAGER_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QFileDialog>

#include "lib/handler/medialibraryhandler.h"
#include "lib/handler/settingshandler.h"
#include "lib/handler/loghandler.h"

#include "dialoghandler.h"

#include <ui_libraryexclusions.h>

class LibraryManager : public QDialog
{
    Q_OBJECT
public:
    LibraryManager(LibraryType libraryType, MediaLibraryHandler* medialLibraryHandler, QWidget* parent = nullptr);
    ~LibraryManager();
signals:
    void pathsChanged();
private slots:
    void on_addButton_clicked();

    void on_removeButton_clicked();

    void on_listWidget_activated(const QModelIndex &index);

private:
    Ui::LibraryExclusionsDialog ui;
    MediaLibraryHandler* m_medialLibraryHandler;
    QList<int> itemsToRemove;
    LibraryType m_libraryType;
    QStringList m_oldPaths;
    bool m_dirty = false;

    void onClose();
};

#endif // LIBRARYMANAGER_H
