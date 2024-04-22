#include "libraryManager.h"

LibraryManager::LibraryManager(QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
    ui.listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui.listWidget->addItems(SettingsHandler::getSelectedLibrary());
    setWindowTitle("Media folders");
}

LibraryManager::~LibraryManager() { }

void LibraryManager::on_addButton_clicked()
{
    QFileDialog file_dialog;
    QString path  = QFileInfo(SettingsHandler::getLastSelectedLibrary()).dir().path();
    file_dialog.setDirectory(path);
    file_dialog.setFileMode(QFileDialog::DirectoryOnly);

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
