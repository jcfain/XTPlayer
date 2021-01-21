#include "libraryexclusions.h"
#include <QTreeView>

LibraryExclusions::LibraryExclusions(QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
    ui.listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui.listWidget->addItems(SettingsHandler::getLibraryExclusions());
}

LibraryExclusions::~LibraryExclusions()
{

}

void LibraryExclusions::on_addButton_clicked()
{
//    QUrl selectedLibrary = QFileDialog::getExistingDirectoryUrl(this, tr("Add library exclusion"), QUrl::fromLocalFile(SettingsHandler::getSelectedFunscriptLibrary()), QFileDialog::ReadOnly | QFileDialog::ShowDirsOnly);
//    if (!selectedLibrary.isEmpty())
//    {
//        QString path = selectedLibrary.path();
//        if(path != SettingsHandler::getSelectedFunscriptLibrary())
//        {
//            SettingsHandler::addToLibraryExclusions(path);
//            ui.listWidget->addItem(path);
//        }
//    }
    QFileDialog file_dialog;
    QString library = SettingsHandler::getSelectedLibrary();
    file_dialog.setDirectory(library);
    //file_dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    file_dialog.setFileMode(QFileDialog::DirectoryOnly);
    //file_dialog.setOption(QFileDialog::ShowDirsOnly, true);

    //to make it possible to select multiple directories:
//    QListView* file_view = file_dialog.findChild<QListView*>("listView");
//    if (file_view)
//        file_view->setSelectionMode(QAbstractItemView::MultiSelection);
//    QTreeView* f_tree_view = file_dialog.findChild<QTreeView*>();
//    if (f_tree_view)
//        f_tree_view->setSelectionMode(QAbstractItemView::MultiSelection);

    if(file_dialog.exec())
    {
        QStringList paths = file_dialog.selectedFiles();
        if(!paths.contains(library))
        {
            foreach(auto path, paths)
            {
                SettingsHandler::addToLibraryExclusions(path);
                ui.listWidget->addItem(path);
            }
        }
    }
}

void LibraryExclusions::on_removeButton_clicked()
{
    if(ui.listWidget->selectedItems().count() > 0)
    {
        QList<int> itemsToRemove;
        foreach(auto item, ui.listWidget->selectedItems())
        {
            itemsToRemove.append(ui.listWidget->row(item));
        }
        SettingsHandler::removeFromLibraryExclusions(itemsToRemove);
        qDeleteAll(ui.listWidget->selectedItems());
    }
}

void LibraryExclusions::on_listWidget_activated(const QModelIndex &index)
{

}
