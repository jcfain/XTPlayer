#include "libraryManager.h"
#include "lib/tool/file-util.h"

LibraryManager::LibraryManager(LibraryType libraryType, MediaLibraryHandler* medialLibraryHandler, QWidget* parent) : QDialog(parent),
    m_libraryType(libraryType),
    m_medialLibraryHandler(medialLibraryHandler)
{
    ui.setupUi(this);
    ui.listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_oldPaths = SettingsHandler::mediaLibrarySettings.get(m_libraryType);
    ui.listWidget->addItems(m_oldPaths);
    setWindowTitle("Manage folders");

    // QString firstPathExists;
    // foreach(auto path, m_oldPaths) {
    //     if(QFileInfo::exists(path)) {
    //         firstPathExists = path;
    //         break;
    //     }
    // }

    // QDir currentDir(firstPathExists);
    // QString defaultPath = !firstPathExists.isEmpty() ? firstPathExists : ".";
    connect(this, &QDialog::rejected, this, &LibraryManager::onClose);
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
            {
                ui.listWidget->addItem(path);
                m_dirty = true;
            }
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
        m_dirty = true;
    }
}

void LibraryManager::on_listWidget_activated(const QModelIndex &index)
{

}

void LibraryManager::onClose()
{
    if(!m_dirty)
        return;
    QStringList currentPaths = SettingsHandler::mediaLibrarySettings.get(m_libraryType);

    if(!currentPaths.isEmpty())
    {
        if(m_oldPaths.isEmpty())
        {
            m_medialLibraryHandler->loadLibraryAsync();
            emit pathsChanged();
            return;
        }
        QSet<QString> additions = QSet<QString>(currentPaths.begin(),currentPaths.end()).subtract(QSet<QString>(m_oldPaths.begin(), m_oldPaths.end()));
        QSet<QString> subtraction = QSet<QString>(m_oldPaths.begin(), m_oldPaths.end()).subtract(QSet<QString>(currentPaths.begin(),currentPaths.end()));
        if(!subtraction.isEmpty() || !additions.empty())
        {
            emit pathsChanged();
            auto message = m_medialLibraryHandler->isLibraryProcessing() ? "Stop current loading process and restart with new list now?" : "Load all libraries now?";
            if(DialogHandler::Dialog(this, message) == QDialog::DialogCode::Accepted)
            {
                m_medialLibraryHandler->loadLibraryAsync();
            }
        }
    }
}
