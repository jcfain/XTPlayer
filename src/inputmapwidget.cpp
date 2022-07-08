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

    QList<QPair<QString, QString>> actions;
    QMap<QString, QStringList> inverseGamePadMap = SettingsHandler::getGamePadMapInverse();

    auto tcodeVersionMap = TCodeChannelLookup::GetSelectedVersionMap();
    auto availableAxis = SettingsHandler::getAvailableAxis();
    for(auto __begin = tcodeVersionMap.begin(), __end = tcodeVersionMap.end();  __begin != __end; ++__begin) {
        auto channel = availableAxis->value(TCodeChannelLookup::ToString(__begin.key()));
        actions.append({channel.AxisName, "Channel: " + channel.FriendlyName});
    }

    MediaActions actionsMap;
    for(auto __begin = actionsMap.Values.begin(), __end = actionsMap.Values.end();  __begin != __end; ++__begin) {
        actions.append({__begin.key(), __begin.value()});
    }

    _tableWidget->setRowCount(actions.count());
    int row = 0;
    foreach(auto actionKeyValue, actions)
    {
        QString action = actionKeyValue.first;
        QString actionName = actionKeyValue.second;
        QString gamePadKeys = inverseGamePadMap.value(action).join(", ");
        QTableWidgetItem *actionNameTableItem = new QTableWidgetItem(actionName);
        actionNameTableItem->setFlags(actionNameTableItem->flags() & ~Qt::ItemIsEditable);
        _tableWidget->setItem(row, _COLUMNS::ACTION_NAME, actionNameTableItem);
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
        editButton->setObjectName(action);
        editButton->setMaximumWidth(150);
        QPushButton* clearButton = new QPushButton(this);
        clearButton->setText("Clear");
        clearButton->setObjectName(action);
        clearButton->setMaximumWidth(150);
        layoutBrowse->addWidget(editButton);
        layoutBrowse->addWidget(clearButton);
        layoutBrowse->setAlignment(Qt::AlignCenter); //set Alignment layout
        layoutBrowse->setContentsMargins(4,4,4,4);
        connect(clearButton, &QPushButton::clicked, this, [this, action, actionName]() {
            auto items = _tableWidget->findItems(actionName, Qt::MatchFlag::MatchExactly);
            QMap<QString, QStringList> inverseGamePadMap = SettingsHandler::getGamePadMapInverse();
            auto item = items.first();
            auto gamepadItem = _tableWidget->item(item->row(), _COLUMNS::GAMEPAD);
            auto buttonMapValues = inverseGamePadMap.value(action);
            foreach (auto button, buttonMapValues)
            {
                gamepadItem->setText("");
                SettingsHandler::removeGamePadMapButton(button, action);
            }
        });

        connect(editButton, &QPushButton::clicked, this, [this, action, actionName]() {
            if(_connectionHandler->getGamepadHandler()->isConnected()) {
                connect(_connectionHandler->getGamepadHandler(), &GamepadHandler::onListenForInputRecieve, this, [this, action, actionName](QString button) {
                    QMap<QString, QStringList> gamepadMap = SettingsHandler::getGamePadMap();
                    auto items = _tableWidget->findItems(actionName, Qt::MatchFlag::MatchExactly);
                    auto item = items.first();
                    auto gamepadItem = _tableWidget->item(item->row(), _COLUMNS::GAMEPAD);
                    auto buttonMapValues = gamepadMap.value(button);
                    auto text = gamepadItem->text();
                    if (buttonMapValues.contains(action))
                    {
                        if(!text.isEmpty()) {
                            if(text.contains(", " + button))
                                gamepadItem->setText(text.trimmed().remove(", " + button));
                            else if(text.contains(button + ", "))
                                gamepadItem->setText(text.trimmed().remove(button + ", "));
                            else
                                gamepadItem->setText(text.trimmed().remove(button));
                        }
                        SettingsHandler::removeGamePadMapButton(button, action);
                    }
                    else
                    {
                        gamepadItem->setText(text.isEmpty() ? button : text + ", " + button);
                        SettingsHandler::setGamePadMapButton(button, action);
                    }
                    tableWidget_Changed(gamepadItem);
                });
                _connectionHandler->getGamepadHandler()->listenForInput();
                if(DialogHandler::Dialog(this, "Press a button or key for action "+actionName, true, false) == QDialog::DialogCode::Rejected) {
                    disconnect(_connectionHandler->getGamepadHandler(), &GamepadHandler::onListenForInputRecieve, nullptr, nullptr);
                }
            }
            else
                DialogHandler::Dialog(this, "No gamepad connected!");
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
