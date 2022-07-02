#include "inputmapwidget.h"

InputMapWidget::InputMapWidget(ConnectionHandler* connectionHandler, QWidget *parent)
    : QWidget{parent}
{
//    _saveButton = ui.buttonBox->button(QDialogButtonBox::StandardButton::Save);
//    _saveButton->setEnabled(false);
//    _resetButton =  ui.buttonBox->button(QDialogButtonBox::StandardButton::Reset);
//    _resetButton->setEnabled(false);
//    _discardButton =  ui.buttonBox->button(QDialogButtonBox::StandardButton::Discard);
//    _discardButton->setEnabled(false);
    _connectionHandler = connectionHandler;
    _layout = new QGridLayout(this);
    _tableWidget = new QTableWidget(this);
    _tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _tableWidget->setColumnCount(4);
    QStringList tableHeader;
    tableHeader <<"Action"<<"Gamepad"<<"Key"<<"Edit";
    _tableWidget->setHorizontalHeaderLabels(tableHeader);
    _tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    // Widget width 1271
    _tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::ACTION_NAME, QHeaderView::ResizeMode::Stretch);
    //ui.tableWidget->horizontalHeader()->resizeSection(_COLUMNS::SCRIPT_NAME, 350);
    _tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::GAMEPAD, QHeaderView::ResizeMode::Stretch);
    //ui.tableWidget->horizontalHeader()->resizeSection(_COLUMNS::SCRIPT_PATH, 450);
    _tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::KEY, QHeaderView::ResizeMode::Stretch);
    //ui.tableWidget->horizontalHeader()->resizeSection(_COLUMNS::MEDIA_PATH, 275);
    _tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::EDIT, QHeaderView::ResizeMode::Stretch);
    //ui.tableWidget->horizontalHeader()->resizeSection(_COLUMNS::SCRIPT_SELECTION, 196);
    _tableWidget->horizontalHeader()->setStretchLastSection(true);
    _layout->addWidget(_tableWidget);
    setLayout(_layout);
    setUpData();
}

void InputMapWidget::setUpData()
{
    //disconnect(_tableWidget, &QTableWidget::itemChanged, this, &DLNAScriptLinks::on_tableWidget_Changed);
    _tableWidget->setSortingEnabled(false);
    QMap<QString, QStringList>* gamepadMap = SettingsHandler::getGamePadMap();
    QMap<QString, QStringList> inverseGamePadMap;
    foreach (auto key, gamepadMap->keys())
    {
        QStringList actions = gamepadMap->value(key);
        foreach(auto action, actions) {
            QStringList existingKeys;
            if(inverseGamePadMap.contains(action)) {
                existingKeys = inverseGamePadMap.value(action);
            }
            existingKeys << key;
            inverseGamePadMap.insert(action, existingKeys);
        }
    }

    auto tcodeVersionMap = TCodeChannelLookup::GetSelectedVersionMap();
    MediaActions mediaActions;
    _tableWidget->setRowCount(mediaActions.Values.count());
    int row = 0;
    foreach (auto key, mediaActions.Values.keys())
    {
        QString gamePadKeys = inverseGamePadMap.value(key).join(", ");
        QString actionKey = mediaActions.Values.value(key);
        QTableWidgetItem *actionName = new QTableWidgetItem(actionKey);
        actionName->setFlags(actionName->flags() & ~Qt::ItemIsEditable);
        _tableWidget->setItem(row, _COLUMNS::ACTION_NAME, actionName);
        QTableWidgetItem *gamepad = new QTableWidgetItem(gamePadKeys);
        gamepad->setFlags(gamepad->flags() & ~Qt::ItemIsEditable);
        _tableWidget->setItem(row, _COLUMNS::GAMEPAD, gamepad);
        QTableWidgetItem *keyboard = new QTableWidgetItem("Coming soon");
        keyboard->setFlags(keyboard->flags() & ~Qt::ItemIsEditable);
        _tableWidget->setItem(row, _COLUMNS::KEY, keyboard);

        QWidget *editWidget = new QWidget(this); //create QWidget
        QHBoxLayout *layoutBrowse = new QHBoxLayout(editWidget);
        QPushButton* editButton = new QPushButton(this);
        editButton->setText("Edit");
        editButton->setObjectName(key);
        editButton->setMaximumWidth(150);
        layoutBrowse->addWidget(editButton);
        layoutBrowse->setAlignment(Qt::AlignCenter); //set Alignment layout
        layoutBrowse->setContentsMargins(4,4,4,4);
        connect(editButton, &QPushButton::clicked, this, [this, key, actionKey, gamepadMap]() {
            if(_connectionHandler->getGamepadHandler()->isConnected()) {
                connect(_connectionHandler->getGamepadHandler(), &GamepadHandler::onListenForInputRecieve, this, [this, key, actionKey, gamepadMap](QString button) {
                    auto items = _tableWidget->findItems(actionKey, Qt::MatchFlag::MatchExactly);
                    auto item = items.first();
                    auto gamepadItem = _tableWidget->item(item->row(), _COLUMNS::GAMEPAD);
                    auto buttonMapValues = gamepadMap->value(button);
                    if (buttonMapValues.contains(key))
                    {
                        auto text = gamepadItem->text();
                        if(!text.isEmpty()) {
                            if(text.contains(", " + button))
                                gamepadItem->setText(text.trimmed().remove(", " + button));
                            else if(text.contains(button + ", "))
                                gamepadItem->setText(text.trimmed().remove(button + ", "));
                            else
                                gamepadItem->setText(text.trimmed().remove(button));
                        }
                        SettingsHandler::removeGamePadMapButton(button, key);
                    }
                    else
                    {
                        gamepadItem->setText(gamepadItem->text().isEmpty() ? button : gamepadItem->text() + ", " + button);
                        SettingsHandler::setGamePadMapButton(button, key);
                    }
                    tableWidget_Changed(gamepadItem);
                });
                _connectionHandler->getGamepadHandler()->listenForInput();
                if(DialogHandler::Dialog(this, "Press a button or key..", true, false) == QDialog::DialogCode::Rejected) {
                    disconnect(_connectionHandler->getGamepadHandler(), &GamepadHandler::onListenForInputRecieve, nullptr, nullptr);
                }
            }
        });
        _tableWidget->setCellWidget(row, _COLUMNS::EDIT, editWidget);
        row++;
    }
    _tableWidget->setSortingEnabled(true);
    _tableWidget->sortItems(_COLUMNS::ACTION_NAME, Qt::SortOrder::AscendingOrder);
    //connect(_tableWidget, &QTableWidget::itemChanged, this, &DLNAScriptLinks::on_tableWidget_Changed);
}

void InputMapWidget::clearData()
{

}

void InputMapWidget::tableWidget_Changed(QTableWidgetItem *item)
{
    DialogHandler::DialogClose();
    disconnect(_connectionHandler->getGamepadHandler(), &GamepadHandler::onListenForInputRecieve, nullptr, nullptr);
    if(item->column() == 1)
    {
//        QString mediaFileKey = _tableWidget->item(item->row(), _COLUMNS::MEDIA_PATH)->text();
//        QString gamepadKey = _tableWidget->item(item->row(), _COLUMNS::GAMEPAD)->text();
//        _tableWidget->item(item->row(), _COLUMNS::SCRIPT_NAME)->setText(gamepadKey);
//        _editedLinks.remove(mediaFileKey);
//        _editedLinks.insert(mediaFileKey, scriptPath);
//        _saveButton->setEnabled(true);
//        _resetButton->setEnabled(true);
    }
}
