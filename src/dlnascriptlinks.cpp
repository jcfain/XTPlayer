#include "dlnascriptlinks.h"

DLNAScriptLinks::DLNAScriptLinks(QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);

    _saveButton = ui.buttonBox->button(QDialogButtonBox::StandardButton::Save);
    _saveButton->setEnabled(false);
    _resetButton =  ui.buttonBox->button(QDialogButtonBox::StandardButton::Reset);
    _resetButton->setEnabled(false);
    _discardButton =  ui.buttonBox->button(QDialogButtonBox::StandardButton::Discard);
    _discardButton->setEnabled(false);

    ui.tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui.tableWidget->setColumnCount(4);
    QStringList tableHeader;
    tableHeader <<"Script name"<<"Script URL"<<"Media URL"<<"Script selection";
    ui.tableWidget->setHorizontalHeaderLabels(tableHeader);
    ui.tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    // Widget width 1271
    ui.tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::SCRIPT_NAME, QHeaderView::ResizeMode::Stretch);
    //ui.tableWidget->horizontalHeader()->resizeSection(_COLUMNS::SCRIPT_NAME, 350);
    ui.tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::SCRIPT_PATH, QHeaderView::ResizeMode::Stretch);
    //ui.tableWidget->horizontalHeader()->resizeSection(_COLUMNS::SCRIPT_PATH, 450);
    ui.tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::MEDIA_PATH, QHeaderView::ResizeMode::Stretch);
    //ui.tableWidget->horizontalHeader()->resizeSection(_COLUMNS::MEDIA_PATH, 275);
    ui.tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::SCRIPT_SELECTION, QHeaderView::ResizeMode::Stretch);
    //ui.tableWidget->horizontalHeader()->resizeSection(_COLUMNS::SCRIPT_SELECTION, 196);
    ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
    //ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

DLNAScriptLinks::~DLNAScriptLinks()
{

}

void DLNAScriptLinks::showDialog()
{
    setUpData();
    show();
}
void DLNAScriptLinks::setUpData()
{
    disconnect(ui.tableWidget, &QTableWidget::itemChanged, this, &DLNAScriptLinks::on_tableWidget_Changed);
    ui.tableWidget->setSortingEnabled(false);
    auto storedLinks = SettingsHandler::getDeoDnlaFunscripts();
    ui.tableWidget->setRowCount(storedLinks.count());
    int row = 0;
    foreach (auto key, storedLinks.keys())
    {
        QString scriptURL = storedLinks.value(key).toString();
        QString scriptName = QFileInfo(scriptURL).fileName();

        QTableWidgetItem *script = new QTableWidgetItem(scriptName);
        script->setFlags(script->flags() & ~Qt::ItemIsEditable);
        ui.tableWidget->setItem(row, _COLUMNS::SCRIPT_NAME, script);
        QTableWidgetItem *scriptURLItem = new QTableWidgetItem(scriptURL);
        ui.tableWidget->setItem(row, _COLUMNS::SCRIPT_PATH, scriptURLItem);
        QTableWidgetItem *videoURL = new QTableWidgetItem(key);
        videoURL->setFlags(videoURL->flags() & ~Qt::ItemIsEditable);
        ui.tableWidget->setItem(row, _COLUMNS::MEDIA_PATH, videoURL);

        QWidget *browseWidget = new QWidget(); //create QWidget
        QHBoxLayout *layoutBrowse = new QHBoxLayout(browseWidget);
        QPushButton* browseButton = new QPushButton();
        browseButton->setText("Browse");
        browseButton->setObjectName(key);
        browseButton->setMaximumWidth(150);
        layoutBrowse->addWidget(browseButton);
        layoutBrowse->setAlignment(Qt::AlignCenter); //set Alignment layout
        layoutBrowse->setContentsMargins(4,4,4,4);
        connect(browseButton, &QPushButton::clicked, this, [this, key]() {
            QString funscriptPath = QFileDialog::getOpenFileName(this, tr("Choose script for video: ") + key, SettingsHandler::getLastSelectedLibrary(), "Script Files (*.funscript)");
            if (!funscriptPath.isEmpty())
            {
                auto items = ui.tableWidget->findItems(key, Qt::MatchFlag::MatchExactly);
                auto item = items.first();
                auto scriptPathItem = ui.tableWidget->item(item->row(), _COLUMNS::SCRIPT_PATH);
                scriptPathItem->setText(funscriptPath);
                on_tableWidget_Changed(scriptPathItem);
            }
        });
        ui.tableWidget->setCellWidget(row, _COLUMNS::SCRIPT_SELECTION, browseWidget);
        row++;
    }
    ui.tableWidget->setSortingEnabled(true);
    ui.tableWidget->sortItems(_COLUMNS::SCRIPT_NAME, Qt::SortOrder::AscendingOrder);
    connect(ui.tableWidget, &QTableWidget::itemChanged, this, &DLNAScriptLinks::on_tableWidget_Changed);
}
void DLNAScriptLinks::clearData()
{
    ui.tableWidget->clearContents();
    ui.tableWidget->setRowCount(0);
    _editedLinks.clear();
    _removedLinks.clear();
    _saveButton->setEnabled(false);
    _resetButton->setEnabled(false);
    _discardButton->setEnabled(false);
}
void DLNAScriptLinks::on_tableWidget_Changed(QTableWidgetItem *item)
{
    if(item->column() == 1)
    {
        QString mediaFileKey = ui.tableWidget->item(item->row(), _COLUMNS::MEDIA_PATH)->text();
        QString scriptPath = ui.tableWidget->item(item->row(), _COLUMNS::SCRIPT_PATH)->text();
        QString scriptName = QFileInfo(scriptPath).fileName();
        ui.tableWidget->item(item->row(), _COLUMNS::SCRIPT_NAME)->setText(scriptName);
        _editedLinks.remove(mediaFileKey);
        _editedLinks.insert(mediaFileKey, scriptPath);
        _saveButton->setEnabled(true);
        _resetButton->setEnabled(true);
    }
}

void DLNAScriptLinks::on_buttonBox_accepted()
{
    QString modifiedValues = "";
    foreach(auto value, _editedLinks.values())
        modifiedValues += value.toString() + ",\n";
    QString removedValues = "";
    foreach(auto value, _removedLinks.values())
        removedValues += value.toString() + ",\n";
    QMessageBox::StandardButton reply =
            QMessageBox::question(this, tr("Save changes"), tr("Are you sure you want to save the following changes:\n\nModify:\n") + modifiedValues + "\n\nRemove:\n" + removedValues,
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        foreach(auto key, _editedLinks.keys())
            SettingsHandler::setLinkedVRFunscript(key, _editedLinks.value(key).toString());
        foreach(auto key, _removedLinks.keys())
            SettingsHandler::removeLinkedVRFunscript(key);
        SettingsHandler::SaveLinkedFunscripts();
        _saveButton->setEnabled(false);
        _resetButton->setEnabled(false);
        _discardButton->setEnabled(false);
    }
}
#include <QSet>
void DLNAScriptLinks::on_buttonBox_clicked(QAbstractButton *button)
{
    if (ui.buttonBox->buttonRole(button) == QDialogButtonBox::ResetRole)
    {
        clearData();
        setUpData();
    }
    else if (ui.buttonBox->buttonRole(button) == QDialogButtonBox::DestructiveRole)
    {
        auto items = ui.tableWidget->selectedItems();// Gets every column item for every full row selected.
        QHash<int, QString> rows;
        foreach(auto item, items) // Remove duplicates
            if(!rows.contains(item->row()))
                rows.insert(item->row(), ui.tableWidget->item(item->row(), _COLUMNS::MEDIA_PATH)->text());

        foreach(auto row, rows.keys())
        {
            auto mediaPath = rows.value(row);
            if(_editedLinks.contains(mediaPath))
                _editedLinks.remove(mediaPath);
            auto item = ui.tableWidget->findItems(mediaPath, Qt::MatchFlag::MatchExactly).first(); // removing items modifies the existing items row. Find the new row if there is one.
            _removedLinks.insert(mediaPath, ui.tableWidget->item(item->row(), _COLUMNS::SCRIPT_NAME)->text());
            ui.tableWidget->removeRow(item->row());
        }
        _saveButton->setEnabled(true);
        _resetButton->setEnabled(true);
        _discardButton->setEnabled(false);
    }
    else if (ui.buttonBox->buttonRole(button) == QDialogButtonBox::RejectRole)
    {
        clearData();
        close();
    }
}

void DLNAScriptLinks::on_tableWidget_itemSelectionChanged()
{
    _discardButton->setEnabled(ui.tableWidget->selectedItems().length() > 0);
}
