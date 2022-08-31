#include "libraryManager.h"

LibraryManager::LibraryManager(QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
    ui.listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui.listWidget->addItems(SettingsHandler::getSelectedLibrary());
    setWindowTitle("Media folders");
}

LibraryManager::~LibraryManager()
{

}

void LibraryManager::on_addButton_clicked()
{
//    QUrl selectedLibrary = QFileDialog::getExistingDirectoryUrl(this, tr("Add library exclusion"), QUrl::fromLocalFile(SettingsHandler::getSelectedFunscriptLibrary()), QFileDialog::ReadOnly | QFileDialog::ShowDirsOnly);
//    if (!selectedLibrary.isEmpty())
//    {
//        QString path = selectedLibrary.path();
//        if(path != SettingsHandler::getSelectedFunscriptLibrary())
//        {
//            SettingsHandler::addToLibraryManager(path);
//            ui.listWidget->addItem(path);
//        }
//    }
    QFileDialog file_dialog;
    QString path  = QFileInfo(SettingsHandler::getLastSelectedLibrary()).dir().path();
    file_dialog.setDirectory(path);
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

    auto currentPaths = SettingsHandler::getSelectedLibrary();
    if(file_dialog.exec())
    {
        QStringList paths = file_dialog.selectedFiles();
        foreach(auto path, paths)
        {
            bool duplicate = false;
            foreach (auto currentPath, currentPaths) {
                if(currentPath==path) {
                    DialogHandler::MessageBox(this, "Directory '"+path+"' is already in the selected list!", XLogLevel::Warning);
                    duplicate = true;
                    break;
                } else if(currentPath.startsWith(path)) {
                    DialogHandler::MessageBox(this, "Directory '"+path+"'\nis a parent of\n'"+currentPath+"'", XLogLevel::Warning);
                    duplicate = true;
                    break;
                } else if(path.startsWith(currentPath)) {
                    DialogHandler::MessageBox(this, "Directory '"+path+"'\nis a child of\n'"+currentPath+"'", XLogLevel::Warning);
                    duplicate = true;
                    break;
                }
            }
            if(duplicate)
                continue;
            SettingsHandler::addSelectedLibrary(path);
            ui.listWidget->addItem(path);
        }
    }
}

void LibraryManager::on_removeButton_clicked()
{
    if(ui.listWidget->selectedItems().count() > 0)
    {
        foreach(auto item, ui.listWidget->selectedItems())
        {
            SettingsHandler::removeSelectedLibrary(item->text());
        }
        qDeleteAll(ui.listWidget->selectedItems());
    }
}

void LibraryManager::on_listWidget_activated(const QModelIndex &index)
{

}
