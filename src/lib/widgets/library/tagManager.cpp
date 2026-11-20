#include "tagManager.h"

#include "gettextdialog.h"

TagManager::TagManager(QWidget* parent, bool smartTagMode) : QDialog(parent),
    m_smartTagMode(smartTagMode)
{
    ui.setupUi(this);
    ui.listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    QStringList tags = m_smartTagMode ? SettingsHandler::getUserSmartTags() : SettingsHandler::getUserTags();
    if(!tags.isEmpty())
        ui.listWidget->addItems(tags);
    setWindowTitle(m_smartTagMode ? tr("Smart tags"): "Tags");
}

TagManager::~TagManager() {
    if(m_modified) {
        SettingsHandler::Save();
        m_modified = false;
    }
}

void TagManager::on_addButton_clicked()
{
    QString tag = GetTextDialog::show(this, "Tag");

    if(tag.isEmpty()) {
        return;
    }
    QStringList tags = m_smartTagMode ? SettingsHandler::getUserSmartTags() : SettingsHandler::getUserTags();
    if(tags.contains(tag)) {
        DialogHandler::MessageBox(this, tr("Tag '")+tag+tr("' is already in the list!"), XLogLevel::Warning);
        return;
    }
    QStringList otherTags = !m_smartTagMode ? SettingsHandler::getUserSmartTags() : SettingsHandler::getUserTags();
    if(otherTags.contains(tag)) {
        DialogHandler::MessageBox(this, tr("Tag '")+tag+tr("' is already in the list ")+(!m_smartTagMode ? tr("smart tags") : tr("user tags")) + "!", XLogLevel::Warning);
        return;
    }
    m_modified = true;
    m_smartTagMode ? SettingsHandler::addUserSmartTag(tag) : SettingsHandler::addUserTag(tag);
    ui.listWidget->addItem(tag);
}

void TagManager::on_removeButton_clicked()
{
    if(ui.listWidget->selectedItems().count() > 0)
    {
        foreach(auto item, ui.listWidget->selectedItems())
        {
            if(m_smartTagMode)
            {
                if(SettingsHandler::removeUserSmartTag(item->text()))
                {
                    delete(item);
                    m_modified = true;
                }
            }
            else {
                if(SettingsHandler::removeUserTag(item->text()))
                {
                    delete(item);
                    m_modified = true;
                }
            }
        }
    }
}

void TagManager::on_listWidget_activated(const QModelIndex &index)
{

}
