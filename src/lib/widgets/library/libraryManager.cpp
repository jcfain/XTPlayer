#include "libraryManager.h"
#include "lib/tool/file-util.h"

LibraryManager::LibraryManager(LibraryType libraryType, QWidget* parent) : QDialog(parent),
    m_libraryType(libraryType)
{
    ui.setupUi(this);
    ui.listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui.listWidget->addItems(SettingsHandler::mediaLibrarySettings.get(m_libraryType));
    setWindowTitle("Media folders");
}

LibraryManager::~LibraryManager() { }

void LibraryManager::on_addButton_clicked()
{
    QFileDialog file_dialog;
    QString path  = QFileInfo(SettingsHandler::mediaLibrarySettings.getLast(m_libraryType)).dir().path();
    file_dialog.setDirectory(path);
    file_dialog.setFileMode(QFileDialog::FileMode::Directory);
    file_dialog.setOption(QFileDialog::ShowDirsOnly, true);

    // auto currentPaths = SettingsHandler::getSelectedLibrary();
    // auto currentVRPaths = SettingsHandler::getVRLibrary();
    // auto currentExcludedPaths = SettingsHandler::getLibraryExclusions();
    if(file_dialog.exec())
    {
        QStringList paths = file_dialog.selectedFiles();
        QStringList duplicates;
        foreach(auto path, paths)
        {
            if(SettingsHandler::mediaLibrarySettings.add(m_libraryType, path, duplicates))
                ui.listWidget->addItem(path);
        }
        if(!duplicates.isEmpty())
            DialogHandler::MessageBox(this, duplicates.join("\n"), XLogLevel::Warning);
    }
}

void LibraryManager::on_removeButton_clicked()
{
    if(ui.listWidget->selectedItems().count() > 0)
    {
        foreach(auto item, ui.listWidget->selectedItems())
        {
            SettingsHandler::mediaLibrarySettings.remove(m_libraryType, item->text());
        }
        qDeleteAll(ui.listWidget->selectedItems());
    }
}

void LibraryManager::on_listWidget_activated(const QModelIndex &index)
{

}
