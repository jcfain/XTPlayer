#include "libraryexclusions.h"
#include <QTreeView>
#include "dialoghandler.h"
#include "lib/tool/file-util.h"

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
    QString library = SettingsHandler::getLastSelectedLibrary();
    file_dialog.setDirectory(library);
    //file_dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    //file_dialog.setFileMode(QFileDialog::DirectoryOnly);
    file_dialog.setFileMode(QFileDialog::FileMode::Directory);
    file_dialog.setOption(QFileDialog::ShowDirsOnly, true);

    if(file_dialog.exec())
    {
        QStringList paths = file_dialog.selectedFiles();
        QStringList duplicates;
        foreach(auto path, paths)
        {
            if(SettingsHandler::addToLibraryExclusions(path, duplicates))
                ui.listWidget->addItem(path);
        }
        if(!duplicates.isEmpty())
            DialogHandler::MessageBox(this, duplicates.join("\n"), XLogLevel::Warning);
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
