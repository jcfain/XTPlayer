#include "inputmapwidget.h"

#include "lib/struct/OutputConnectionPacket.h"

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
    _tableWidget->setColumnCount(6);
    QStringList tableHeader;
    tableHeader <<"Action"<<"Gamepad"<<"Key"<<"TCode"<<"Invert"<<"Clear";
    _tableWidget->setHorizontalHeaderLabels(tableHeader);
    _tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    // Widget width 1271
    _tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::ACTION_NAME, QHeaderView::ResizeMode::ResizeToContents);
    _tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::GAMEPAD, QHeaderView::ResizeMode::ResizeToContents);
    _tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::KEY, QHeaderView::ResizeMode::ResizeToContents);
    _tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::TCODE, QHeaderView::ResizeMode::ResizeToContents);
    _tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::INVERT, QHeaderView::ResizeMode::ResizeToContents);
    _tableWidget->horizontalHeader()->setSectionResizeMode(_COLUMNS::EDIT, QHeaderView::ResizeMode::ResizeToContents);
    //_tableWidget->horizontalHeader()->setStretchLastSection(true);
    connect(_tableWidget, &QTableWidget::cellClicked, this, &InputMapWidget::tableItemClicked);
    connect(this, &InputMapWidget::refreshData, this, &InputMapWidget::setUpData);
    _layout->addWidget(_tableWidget);
    setLayout(_layout);
    setUpData();
}

void InputMapWidget::setUpData()
{
    //disconnect(_tableWidget, &QTableWidget::itemChanged, this, &DLNAScriptLinks::on_tableWidget_Changed);

    _tableWidget->setSortingEnabled(false);
    _tableWidget->clearContents();
    _tableWidget->setRowCount(0);

    QList<QPair<QString, QString>> actions;
    auto tcodeVersionMap = TCodeChannelLookup::GetSelectedVersionMap();
    for(auto __begin = tcodeVersionMap.begin(), __end = tcodeVersionMap.end();  __begin != __end; ++__begin) {
        auto channel = TCodeChannelLookup::getChannel(TCodeChannelLookup::ToString(__begin.key()));
        if(channel)
            actions.append({channel->ChannelName, "Channel: " + channel->FriendlyName});
    }

    MediaActions actionsMap;
    for(auto __begin = actionsMap.Values.begin(), __end = actionsMap.Values.end();  __begin != __end; ++__begin) {
        actions.append({__begin.key(), __begin.value()});
    }

    auto otherActions = MediaActions::GetOtherActions();
    for(auto __begin = otherActions.begin(), __end = otherActions.end();  __begin != __end; ++__begin) {
        actions.append({__begin.key(), __begin.value()});
    }

    QMap<QString, QStringList> inverseGamePadMap = SettingsHandler::getGamePadMapInverse();
    QMap<QString, QStringList> inverseKeyboardMap = SettingsHandler::getKeyboardMapInverse();
    QMap<QString, QStringList> inverseTCodeCommandMap = SettingsHandler::getTCodeCommandMapInverse();

    _tableWidget->setRowCount(actions.count() + 1);
    int row = 0;
    foreach(auto actionKeyValue, actions)
    {
        QString action = actionKeyValue.first;

        if(action == TCodeChannelLookup::None())
            continue;
        QString actionName = actionKeyValue.second;
        QString gamePadKeys = inverseGamePadMap.value(action).join(", ");
        QString keyboardKeys;
        QStringList keys = inverseKeyboardMap.value(action);
        for(int i=0; i<keys.size(); i++) {
            keyboardKeys += keys[i];
            if(i<keys.size()-1)
                keyboardKeys += ", ";
        }
        QString tcodeCommands = inverseTCodeCommandMap.value(action).join(", ");
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
        QTableWidgetItem *tcode = new QTableWidgetItem(tcodeCommands);
        tcode->setFlags(tcode->flags() & ~Qt::ItemIsEditable);
        tcode->setData(Qt::UserRole, QVariant::fromValue(actionKeyValue));
        _tableWidget->setItem(row, _COLUMNS::TCODE, tcode);


        QWidget *invertWidget = new QWidget(this);
        QHBoxLayout *invertLayout = new QHBoxLayout(invertWidget);
        if(TCodeChannelLookup::hasChannel(action)) {
            auto channel = TCodeChannelLookup::getChannel(action);
            if(channel->Type == ChannelType::Oscillate) {
                QCheckBox* invertCheckbox = new QCheckBox(this);
                invertCheckbox->setChecked(channel->GamepadInverted);
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

        QWidget *editWidget = new QWidget(this);
        QHBoxLayout *editLayoutayout = new QHBoxLayout(editWidget);
        if(action != TCodeChannelLookup::None()) {
            QPushButton* clearRowButton = new QPushButton(this);
            clearRowButton->setText("Clear Row");
            clearRowButton->setObjectName(action + "ClearRowButton");
            clearRowButton->setMaximumWidth(150);
            //editLayoutayout->addWidget(editButton);
            connect(clearRowButton, &QPushButton::clicked, this, [this, action, actionName]() {
                QMessageBox::StandardButton reply = QMessageBox::question(this, "WARNING!", "Are you sure you want to clear ALL bindings for: "+actionName +"?",
                                              QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                    auto items = _tableWidget->findItems(actionName, Qt::MatchFlag::MatchExactly);
                    auto item = items.first();
                    clearAllData(item->row());
                }
            });
            QPushButton* clearGamePadButton = new QPushButton(this);
            clearGamePadButton->setText("Clear Gamepad");
            clearGamePadButton->setObjectName(action + "ClearGamepadButton");
            clearGamePadButton->setMaximumWidth(150);
            connect(clearGamePadButton, &QPushButton::clicked, this, [this, action, actionName]() {
                QMessageBox::StandardButton reply = QMessageBox::question(this, "WARNING!", "Are you sure you want to clear GAMEPAD bindings for: "+actionName +"?",
                                              QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                    auto items = _tableWidget->findItems(actionName, Qt::MatchFlag::MatchExactly);
                    auto item = items.first();
                    clearGamePadData(item->row());
                }
            });
            QPushButton* clearKeyboardButton = new QPushButton(this);
            clearKeyboardButton->setText("Clear Keys");
            clearKeyboardButton->setObjectName(action + "ClearKeysButton");
            clearKeyboardButton->setMaximumWidth(150);
            connect(clearKeyboardButton, &QPushButton::clicked, this, [this, action, actionName]() {
                QMessageBox::StandardButton reply = QMessageBox::question(this, "WARNING!", "Are you sure you want to clear KEYBOARD bindings for: "+actionName +"?",
                                              QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                    auto items = _tableWidget->findItems(actionName, Qt::MatchFlag::MatchExactly);
                    auto item = items.first();
                    clearKeyboardData(item->row());
                }
            });

            QPushButton* clearTCodeCommandButton = new QPushButton(this);
            clearTCodeCommandButton->setText("Clear TCode");
            clearTCodeCommandButton->setObjectName(action + "ClearTCodeButton");
            clearTCodeCommandButton->setMaximumWidth(150);
            connect(clearTCodeCommandButton, &QPushButton::clicked, this, [this, action, actionName]() {
                QMessageBox::StandardButton reply = QMessageBox::question(this, "WARNING!", "Are you sure you want to clear TCode command bindings for: "+actionName +"?",
                                                                          QMessageBox::Yes|QMessageBox::No);
                if (reply == QMessageBox::Yes) {
                    auto items = _tableWidget->findItems(actionName, Qt::MatchFlag::MatchExactly);
                    auto item = items.first();
                    clearTCodeCommandData(item->row());
                }
            });

            editLayoutayout->addWidget(clearRowButton);
            editLayoutayout->addWidget(clearGamePadButton);
            editLayoutayout->addWidget(clearKeyboardButton);
            editLayoutayout->addWidget(clearTCodeCommandButton);
            editLayoutayout->setAlignment(Qt::AlignCenter);
            editLayoutayout->setContentsMargins(4,4,4,4);
        }
        _tableWidget->setCellWidget(row, _COLUMNS::EDIT, editWidget);
        row++;
    }

    QTableWidgetItem *actionNameTableItem = new QTableWidgetItem("(Default)");
    actionNameTableItem->setFlags(actionNameTableItem->flags() & ~Qt::ItemIsEditable);
    _tableWidget->setItem(row, _COLUMNS::ACTION_NAME, actionNameTableItem);

    QWidget *defaultGamePadWidget = new QWidget(this);
    QHBoxLayout *defaultGamePadLayout = new QHBoxLayout(defaultGamePadWidget);
    QPushButton* defaultGamePadButton = new QPushButton(defaultGamePadWidget);
    defaultGamePadButton->setText("Default All gamepad");
    defaultGamePadButton->setObjectName("DefaultAllGamepadButton");
    defaultGamePadButton->setMaximumWidth(150);
    //editLayoutayout->addWidget(editButton);
    connect(defaultGamePadButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "WARNING!", "Are you sure you want to default ALL GAMEPAD bindings?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            SettingsHandler::SetGamepadMapDefaults();
            emit refreshData();
        }
    });
    defaultGamePadLayout->addWidget(defaultGamePadButton);
    defaultGamePadLayout->setAlignment(Qt::AlignCenter);
    defaultGamePadLayout->setContentsMargins(4,4,4,4);
    _tableWidget->setCellWidget(row, _COLUMNS::GAMEPAD, defaultGamePadWidget);

    QWidget *defaultKeyboardWidget = new QWidget(this);
    QHBoxLayout *defaultKeyboardLayout = new QHBoxLayout(defaultKeyboardWidget);
    QPushButton* defaultKeyboardButton = new QPushButton(defaultKeyboardWidget);
    defaultKeyboardButton->setText("Default All keys");
    defaultKeyboardButton->setObjectName("DefaultAllKeyboardButton");
    defaultKeyboardButton->setMaximumWidth(150);
    //editLayoutayout->addWidget(editButton);
    connect(defaultKeyboardButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "WARNING!", "Are you sure you want to default ALL KEYBOARD bindings?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            SettingsHandler::SetKeyboardKeyDefaults();
            emit refreshData();
        }
    });
    defaultKeyboardLayout->addWidget(defaultKeyboardButton);
    defaultKeyboardLayout->setAlignment(Qt::AlignCenter);
    defaultKeyboardLayout->setContentsMargins(4,4,4,4);
    _tableWidget->setCellWidget(row, _COLUMNS::KEY, defaultKeyboardWidget);

    QWidget *defaultTCodeCommandWidget = new QWidget(this);
    QHBoxLayout *defaultTCodeCommandLayout = new QHBoxLayout(defaultTCodeCommandWidget);
    QPushButton* defaultTCodeCommandButton = new QPushButton(defaultTCodeCommandWidget);
    defaultTCodeCommandButton->setText("Default All TCode");
    defaultTCodeCommandButton->setObjectName("DefaultAllTCodeButton");
    defaultTCodeCommandButton->setMaximumWidth(150);
    connect(defaultTCodeCommandButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "WARNING!", "Are you sure you want to default ALL TCode command bindings?",
                                                                  QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            SettingsHandler::SetTCodeCommandMapDefaults();
            emit refreshData();
        }
    });
    defaultTCodeCommandLayout->addWidget(defaultTCodeCommandButton);
    defaultTCodeCommandLayout->setAlignment(Qt::AlignCenter);
    defaultTCodeCommandLayout->setContentsMargins(4,4,4,4);
    _tableWidget->setCellWidget(row, _COLUMNS::TCODE, defaultTCodeCommandWidget);

    QWidget *defaultInvertWidget = new QWidget(this);
    _tableWidget->setCellWidget(row, _COLUMNS::INVERT, defaultInvertWidget);

    QWidget *defaultAllWidget = new QWidget(this);
    QHBoxLayout *defaultAllLayout = new QHBoxLayout(defaultAllWidget);
    QPushButton* defaultAllButton = new QPushButton(defaultAllWidget);
    defaultAllButton->setText("Default All");
    defaultAllButton->setObjectName("DefaultAllButton");
    defaultAllButton->setMaximumWidth(150);
    //editLayoutayout->addWidget(editButton);
    connect(defaultAllButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "WARNING!", "Are you sure you want to default ALL bindings?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            SettingsHandler::SetGamepadMapDefaults();
            SettingsHandler::SetKeyboardKeyDefaults();
            SettingsHandler::SetTCodeCommandMapDefaults();
            emit refreshData();
        }
    });

    defaultAllLayout->addWidget(defaultAllButton);
    defaultAllLayout->setAlignment(Qt::AlignCenter);
    defaultAllLayout->setContentsMargins(4,4,4,4);
    _tableWidget->setCellWidget(row, _COLUMNS::EDIT, defaultAllWidget);

    _tableWidget->setSortingEnabled(true);
    _tableWidget->sortItems(_COLUMNS::ACTION_NAME, Qt::SortOrder::AscendingOrder);
}

void InputMapWidget::clearAllData(int row)
{
    clearGamePadData(row);
    clearKeyboardData(row);
}
void InputMapWidget::clearGamePadData(int row)
{
    auto tableCell = _tableWidget->item(row, _COLUMNS::GAMEPAD);
    auto actionKeyValue = tableCell->data(Qt::UserRole).value<QPair<QString, QString>>();
    QString action = actionKeyValue.first;
    QMap<QString, QStringList> inverseGamePadMap = SettingsHandler::getGamePadMapInverse();
    auto buttonMapValues = inverseGamePadMap.value(action);
    foreach (auto button, buttonMapValues)
    {
        SettingsHandler::removeGamePadMapButton(button, action);
    }
    tableCell->setText("");
}
void InputMapWidget::clearKeyboardData(int row)
{
    auto tableCell = _tableWidget->item(row, _COLUMNS::KEY);
    auto actionKeyValue = tableCell->data(Qt::UserRole).value<QPair<QString, QString>>();
    QString action = actionKeyValue.first;
    QMap<QString, QStringList> inverseKeyboardMap = SettingsHandler::getKeyboardMapInverse();
    auto keyboardMapValues = inverseKeyboardMap.value(action);
    foreach (auto key, keyboardMapValues)
    {
        SettingsHandler::removeKeyboardMapKey(key, action);
    }
    tableCell->setText("");
}

void InputMapWidget::clearTCodeCommandData(int row)
{
    auto tableCell = _tableWidget->item(row, _COLUMNS::TCODE);
    auto actionKeyValue = tableCell->data(Qt::UserRole).value<QPair<QString, QString>>();
    QString action = actionKeyValue.first;
    QMap<QString, QStringList> inverseMap = SettingsHandler::getTCodeCommandMapInverse();
    auto mapValues = inverseMap.value(action);
    foreach (auto key, mapValues)
    {
        SettingsHandler::removeTCodeCommandMapKey(key, action);
    }
    tableCell->setText("");
}

void InputMapWidget::tableItemClicked(int row, int column)
{
    if(DialogHandler::IsDialogOpen()) {
        return;
    }
    if(_COLUMNS::GAMEPAD == column)
    {
        auto gamepadTableCell = _tableWidget->item(row, _COLUMNS::GAMEPAD);
        if(gamepadTableCell) {
            auto actionKeyValue = gamepadTableCell->data(Qt::UserRole).value<QPair<QString, QString>>();
            QString action = actionKeyValue.first;
            if(action != TCodeChannelLookup::None()) {
                QString actionName = actionKeyValue.second;
                listenForGamepadInput(action, actionName);
            }
        }
    }
    else if(_COLUMNS::KEY == column)
    {
        //Listen for key input
        auto keyboardTableCell = _tableWidget->item(row, _COLUMNS::KEY);
        if(keyboardTableCell) {
            auto actionKeyValue = keyboardTableCell->data(Qt::UserRole).value<QPair<QString, QString>>();
            QString action = actionKeyValue.first;
            if(action != TCodeChannelLookup::None()) {
                QString actionName = actionKeyValue.second;
                listenForKeyboardInput(action, actionName);
            }
        }
    }
    else if(_COLUMNS::TCODE == column)
    {
        //Listen for key input
        auto tableCell = _tableWidget->item(row, _COLUMNS::TCODE);
        if(tableCell) {
            auto actionKeyValue = tableCell->data(Qt::UserRole).value<QPair<QString, QString>>();
            QString action = actionKeyValue.first;
            if(action != TCodeChannelLookup::None()) {
                QString actionName = actionKeyValue.second;
                listenForTCodeCommandInput(action, actionName);
            }
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
            disconnect(_connectionHandler->getGamepadHandler(), &GamepadHandler::onListenForInputRecieve, this, nullptr);
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
        auto keyValue = event->key();
        auto modifiers = event->modifiers();
        auto keySeq = QKeySequence(modifiers | keyValue);
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
        disconnect(this, &InputMapWidget::keyRelease, this, nullptr);
        releaseKeyboard();
    }
    setFocus();
    grabKeyboard();
}

void InputMapWidget::listenForTCodeCommandInput(QString action, QString actionName)
{
    auto outputDevice = _connectionHandler->getSelectedOutputConnection();
    QCheckBox* onReleaseChackbox = new QCheckBox();
    if(outputDevice && outputDevice->isConnected()) {
        connect(outputDevice, &OutputConnectionHandler::commandRecieve, this, [this, action, actionName, onReleaseChackbox](OutputConnectionPacket commandInput) {
            bool onRelease = onReleaseChackbox->isChecked();
            if(commandInput.original.isEmpty() ||
                (commandInput.type == OutputDeviceCommandType::BUTTON &&
                    onRelease &&
                    commandInput.value > 0)) {
                return;
            }

            QString command = commandInput.type == OutputDeviceCommandType::BUTTON ? commandInput.original : commandInput.command;
            QMap<QString, QStringList> tcodeCommandMap = SettingsHandler::getTCodeCommandMap();
            auto items = _tableWidget->findItems(actionName, Qt::MatchFlag::MatchExactly);
            auto item = items.first();
            auto tableCell = _tableWidget->item(item->row(), _COLUMNS::TCODE);
            auto mapValues = tcodeCommandMap.value(command);
            auto text = tableCell->text();
            if (mapValues.contains(action))
            {
                if(!text.isEmpty()) {
                    if(text.contains(", " + command))
                        tableCell->setText(text.trimmed().remove(", " + command));
                    else if(text.contains(command + ", "))
                        tableCell->setText(text.trimmed().remove(command + ", "));
                    // else if(text.contains(commandInput.command))
                    //     tableCell->setText(text.trimmed().remove(QRegExp(commandInput.command+"*,")));
                    else
                        tableCell->setText(text.trimmed().remove(command));
                }
                SettingsHandler::removeTCodeCommandMapKey(command, action);
            }
            else
            {
                tableCell->setText(text.isEmpty() ? command : text + ", " + command);
                SettingsHandler::setTCodeCommandMapKey(command, action);
            }
            tableWidget_Changed(tableCell);
        });

    }
    else
        DialogHandler::Dialog(this, "No device connected!");

    QFormLayout m_tcodeCommandSelectionLayout;
    m_tcodeCommandSelectionLayout.addRow("On release", onReleaseChackbox);
    if(DialogHandler::Dialog(this, &m_tcodeCommandSelectionLayout, true, false) == QDialog::DialogCode::Rejected) {
        disconnect(outputDevice, &OutputConnectionHandler::commandRecieve, this, nullptr);
        delete onReleaseChackbox;
    }
    // Use dropdown interface. Undecided yet
    // auto commands = SettingsHandler::getTCodeCommands();

    // QFormLayout m_tcodeCommandSelectionLayout;
    // QComboBox m_tcodeCommandSelectionCmb;
    // m_tcodeCommandSelectionLayout.addRow(new QLabel("Command", this), &m_tcodeCommandSelectionCmb);
    // m_tcodeCommandSelectionCmb.clear();
    // foreach (auto command, commands) {
    //     m_tcodeCommandSelectionCmb.addItem(command.command, TCodeCommand::toVariant(command));
    // }
    // if(DialogHandler::Dialog(this, &m_tcodeCommandSelectionLayout, true, true) == QDialog::DialogCode::Accepted) {
    //     auto commandInput = TCodeCommand::fromVariant(m_tcodeCommandSelectionCmb.currentData());
    //     QString command = commandInput.command;
    //     QMap<QString, QStringList> tcodeCommandMap = SettingsHandler::getTCodeCommandMap();
    //     auto items = _tableWidget->findItems(actionName, Qt::MatchFlag::MatchExactly);
    //     auto item = items.first();
    //     auto tableCell = _tableWidget->item(item->row(), _COLUMNS::TCODE);
    //     auto mapValues = tcodeCommandMap.value(command);
    //     auto text = tableCell->text();
    //     if (mapValues.contains(action))
    //     {
    //         if(!text.isEmpty()) {
    //             if(text.contains(", " + command))
    //                 tableCell->setText(text.trimmed().remove(", " + command));
    //             else if(text.contains(command + ", "))
    //                 tableCell->setText(text.trimmed().remove(command + ", "));
    //             else
    //                 tableCell->setText(text.trimmed().remove(command));
    //         }
    //         SettingsHandler::removeTCodeCommandMapKey(command, action);
    //     }
    //     else
    //     {
    //         tableCell->setText(text.isEmpty() ? command : text + ", " + command);
    //         SettingsHandler::setTCodeCommandMapKey(command, action);
    //     }
    //     tableWidget_Changed(tableCell);
    // }
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
