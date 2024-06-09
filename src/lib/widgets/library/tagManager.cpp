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
    setWindowTitle(m_smartTagMode ? "Smart tags": "Tags");
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
        DialogHandler::MessageBox(this, "Tag '"+tag+"' is already in the list!", XLogLevel::Warning);
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
            SettingsHandler::removeUserTag(item->text());
        }
        qDeleteAll(ui.listWidget->selectedItems());
        m_modified = true;
    }
}

void TagManager::on_listWidget_activated(const QModelIndex &index)
{

}
