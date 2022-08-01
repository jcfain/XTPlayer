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
    _tableWidget->setColumnCount(5);
    QStringList tableHeader;
    tableHeader <<"Action"<<"Gamepad"<<"Key"<<"Invert"<<"Edit";
    _tableWidget->setHorizontalHeaderLabels(tableHeader);
    _tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    // Widget width 1271
    _tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::ACTION_NAME, QHeaderView::ResizeMode::Stretch);
    //ui.tableWidget->horizontalHeader()->resizeSection(_COLUMNS::SCRIPT_NAME, 350);
    _tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::GAMEPAD, QHeaderView::ResizeMode::Stretch);
    //ui.tableWidget->horizontalHeader()->resizeSection(_COLUMNS::SCRIPT_PATH, 450);
    _tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::KEY, QHeaderView::ResizeMode::Stretch);
    _tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::INVERT, QHeaderView::ResizeMode::Stretch);
    //ui.tableWidget->horizontalHeader()->resizeSection(_COLUMNS::MEDIA_PATH, 275);
    _tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::EDIT, QHeaderView::ResizeMode::Stretch);
    //ui.tableWidget->horizontalHeader()->resizeSection(_COLUMNS::SCRIPT_SELECTION, 196);
    _tableWidget->horizontalHeader()->setStretchLastSection(true);
    connect(_tableWidget, &QTableWidget::cellClicked, this, &InputMapWidget::tableItemClicked);
    _layout->addWidget(_tableWidget);
    setLayout(_layout);
    setUpData();
}

void InputMapWidget::setUpData()
{
    //disconnect(_tableWidget, &QTableWidget::itemChanged, this, &DLNAScriptLinks::on_tableWidget_Changed);

    QList<QPair<QString, QString>> actions;
    QMap<QString, QStringList> inverseGamePadMap = SettingsHandler::getGamePadMapInverse();
    QMap<QString, QStringList> inverseKeyboardMap = SettingsHandler::getKeyboardMapInverse();

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
        QString keyboardKeys;
        QStringList keys = inverseKeyboardMap.value(action);
        for(int i=0; i<keys.size(); i++) {
            keyboardKeys += keys[i];
            if(i<keys.size()-1)
                keyboardKeys += ", ";
        }
        QTableWidgetItem *actionNameTableItem = new QTableWidgetItem(actionName);
        actionNameTableItem->setFlags(actionNameTableItem->flags() & ~Qt::ItemIsEditable);
        _tableWidget->setItem(row, _COLUMNS::ACTION_NAME, actionNameTableItem);
        QTableWidgetItem *gamepad = new QTableWidgetItem(gamePadKeys);
        gamepad->setFlags(gamepad->flags() & ~Qt::ItemIsEditable);
        gamepad->setData(Qt::UserRole, QVariant::fromValue(actionKeyValue));
        _tableWidget->setItem(row, _COLUMNS::GAMEPAD, gamepad);
        QTableWidgetItem *keyboard = new QTableWidgetItem(keyboardKeys);
        keyboard->setFlags(keyboard->flags() & ~Qt::ItemIsEditable);
        keyboard->setData(Qt::UserRole, QVariant::fromValue(actionKeyValue));
        _tableWidget->setItem(row, _COLUMNS::KEY, keyboard);

        QWidget *invertWidget = new QWidget(this); //create QWidget
        QHBoxLayout *invertLayout = new QHBoxLayout(invertWidget);
        if(availableAxis->contains(action)) {
            auto channel = availableAxis->value(action);
            if(channel.Type == AxisType::Range) {
                QCheckBox* invertCheckbox = new QCheckBox(this);
                invertCheckbox->setChecked(channel.GamepadInverted);
                invertCheckbox->setObjectName(action + "InvertCheckbox");
                connect(invertCheckbox, &QCheckBox::clicked, this, [action](bool checked) {
                    SettingsHandler::setChannelGamepadInverse(action, checked);
                });
                invertLayout->addWidget(invertCheckbox);
                invertLayout->setAlignment(Qt::AlignCenter); //set Alignment layout
                invertLayout->setContentsMargins(4,4,4,4);
            }
        }
        _tableWidget->setCellWidget(row, _COLUMNS::INVERT, invertWidget);

        QWidget *editWidget = new QWidget(this); //create QWidget
        QHBoxLayout *editLayoutayout = new QHBoxLayout(editWidget);
        if(action != TCodeChannelLookup::None()) {
//            QPushButton* editButton = new QPushButton(this);
//            editButton->setText("Change");
//            editButton->setObjectName(action + "ChangeButton");
//            editButton->setMaximumWidth(150);
            QPushButton* clearButton = new QPushButton(this);
            clearButton->setText("Clear");
            clearButton->setObjectName(action + "ClearButton");
            clearButton->setMaximumWidth(150);
            //editLayoutayout->addWidget(editButton);
            editLayoutayout->addWidget(clearButton);
            editLayoutayout->setAlignment(Qt::AlignCenter); //set Alignment layout
            editLayoutayout->setContentsMargins(4,4,4,4);
            connect(clearButton, &QPushButton::clicked, this, [this, action, actionName]() {
                auto items = _tableWidget->findItems(actionName, Qt::MatchFlag::MatchExactly);
                auto item = items.first();

                //Clear gamepad buttons
                QMap<QString, QStringList> inverseGamePadMap = SettingsHandler::getGamePadMapInverse();
                auto gamepadTableCell = _tableWidget->item(item->row(), _COLUMNS::GAMEPAD);
                auto buttonMapValues = inverseGamePadMap.value(action);
                foreach (auto button, buttonMapValues)
                {
                    gamepadTableCell->setText("");
                    SettingsHandler::removeGamePadMapButton(button, action);
                }
                //Clear keyboard keys
                QMap<QString, QStringList> inverseKeyboardMap = SettingsHandler::getKeyboardMapInverse();
                auto keyboardTableCell = _tableWidget->item(item->row(), _COLUMNS::KEY);
                auto keyboardMapValues = inverseKeyboardMap.value(action);
                foreach (auto key, keyboardMapValues)
                {
                    keyboardTableCell->setText("");
                    SettingsHandler::removeKeyboardMapKey(key, action);
                }
            });

//            connect(editButton, &QPushButton::clicked, this, [this, action, actionName]() {
//                listenForGamepadInput(action, actionName);
//            });
        }
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

void InputMapWidget::tableItemClicked(int row, int column)
{
    if(DialogHandler::IsDialogOpen()) {
        return;
    }
    if(_COLUMNS::GAMEPAD == column)
    {
        auto gamepadTableCell = _tableWidget->item(row, _COLUMNS::GAMEPAD);
        auto actionKeyValue = gamepadTableCell->data(Qt::UserRole).value<QPair<QString, QString>>();
        QString action = actionKeyValue.first;
        if(action != TCodeChannelLookup::None()) {
            QString actionName = actionKeyValue.second;
            listenForGamepadInput(action, actionName);
        }
    }
    else if(_COLUMNS::KEY == column)
    {
        //Listen for key input
        auto keyboardTableCell = _tableWidget->item(row, _COLUMNS::KEY);
        auto actionKeyValue = keyboardTableCell->data(Qt::UserRole).value<QPair<QString, QString>>();
        QString action = actionKeyValue.first;
        if(action != TCodeChannelLookup::None()) {
            QString actionName = actionKeyValue.second;
            listenForKeyboardInput(action, actionName);
        }
    }
}

void InputMapWidget::listenForGamepadInput(QString action, QString actionName)
{
    if(_connectionHandler->getGamepadHandler()->isConnected()) {
        connect(_connectionHandler->getGamepadHandler(), &GamepadHandler::onListenForInputRecieve, this, [this, action, actionName](QString button) {
            QMap<QString, QStringList> gamepadMap = SettingsHandler::getGamePadMap();
            auto items = _tableWidget->findItems(actionName, Qt::MatchFlag::MatchExactly);
            auto item = items.first();
            auto gamepadTableCell = _tableWidget->item(item->row(), _COLUMNS::GAMEPAD);
            auto buttonMapValues = gamepadMap.value(button);
            auto text = gamepadTableCell->text();
            if (buttonMapValues.contains(action))
            {
                if(!text.isEmpty()) {
                    if(text.contains(", " + button))
                        gamepadTableCell->setText(text.trimmed().remove(", " + button));
                    else if(text.contains(button + ", "))
                        gamepadTableCell->setText(text.trimmed().remove(button + ", "));
                    else
                        gamepadTableCell->setText(text.trimmed().remove(button));
                }
                SettingsHandler::removeGamePadMapButton(button, action);
            }
            else
            {
                gamepadTableCell->setText(text.isEmpty() ? button : text + ", " + button);
                SettingsHandler::setGamePadMapButton(button, action);
            }
            tableWidget_Changed(gamepadTableCell);
        });
        _connectionHandler->getGamepadHandler()->listenForInput();
        if(DialogHandler::Dialog(this, "Press a button for action "+actionName, true, false) == QDialog::DialogCode::Rejected) {
            disconnect(_connectionHandler->getGamepadHandler(), &GamepadHandler::onListenForInputRecieve, nullptr, nullptr);
        }
    }
    else
        DialogHandler::Dialog(this, "No gamepad connected!");
}


void InputMapWidget::listenForKeyboardInput(QString action, QString actionName) {
    connect(this, &InputMapWidget::keyRelease, this, [this, action, actionName](QKeyEvent *event) {
        auto key = SettingsHandler::getKeyboardKey(event->key(), event->modifiers());
        if(key.isEmpty())
            return;
        auto keySeq = QKeySequence(event->modifiers()+event->key());
        QString keyString = keySeq.toString(QKeySequence::NativeText);
        auto items = _tableWidget->findItems(actionName, Qt::MatchFlag::MatchExactly);
        auto item = items.first();
        auto keyboardTableCell = _tableWidget->item(item->row(), _COLUMNS::KEY);
        auto actions = SettingsHandler::getKeyboardKeyActionList(event->key(),event->modifiers());
        auto text = keyboardTableCell->text();
        if (actions.contains(action))
        {
            if(!text.isEmpty()) {
                if(text.contains(", " + keyString))
                    keyboardTableCell->setText(text.trimmed().remove(", " + keyString));
                else if(text.contains(keyString + ", "))
                    keyboardTableCell->setText(text.trimmed().remove(keyString + ", "));
                else
                    keyboardTableCell->setText(text.trimmed().remove(keyString));
            }
            SettingsHandler::removeKeyboardMapKey(key, action);
        }
        else
        {
            keyboardTableCell->setText(text.isEmpty() ? keyString : text + ", " + keyString);
            SettingsHandler::setKeyboardMapKey(key, action);
        }
        tableWidget_Changed(keyboardTableCell);
    });
    if(DialogHandler::Dialog(this, "Press key for action "+actionName, false, false) == QDialog::DialogCode::Rejected) {
        disconnect(this, &InputMapWidget::keyRelease, nullptr, nullptr);
        releaseKeyboard();
    }
    setFocus();
    grabKeyboard();
}
void InputMapWidget::keyReleaseEvent(QKeyEvent *e) {
    emit keyRelease(e);
}
void InputMapWidget::tableWidget_Changed(QTableWidgetItem *item)
{
    DialogHandler::DialogClose();
    releaseKeyboard();
    disconnect(_connectionHandler->getGamepadHandler(), &GamepadHandler::onListenForInputRecieve, nullptr, nullptr);
    disconnect(this, &InputMapWidget::keyRelease, nullptr, nullptr);
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
