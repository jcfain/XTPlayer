#ifndef INPUTMAPWIDGET_H
#define INPUTMAPWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QKeyEvent>
#include <lib/handler/dialoghandler.h>

#include <lib/handler/settingshandler.h>
#include <lib/handler/loghandler.h>
#include <lib/handler/connectionhandler.h>

class InputMapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit InputMapWidget(ConnectionHandler* connectionHandler, QWidget *parent = nullptr);

signals:
    void keyRelease(QKeyEvent* e);
    void refreshData();

private slots:
    void tableWidget_Changed(QTableWidgetItem *item);
    //void mousePressEvent(QMouseEvent *e) override;
    //void buttonBox_accepted();
    //void buttonBox_clicked(QAbstractButton *button);
    //void tableWidget_itemSelectionChanged();

protected:
    virtual void keyReleaseEvent(QKeyEvent *e);

private:
    QGridLayout* _layout;
    QTableWidget* _tableWidget;
    QList<int> itemsToRemove;
    QPushButton* _saveButton;
    QPushButton* _resetButton;
    QPushButton* _discardButton;
    QHash<QString, QVariant> _editedLinks;
    QHash<QString, QVariant> _removedLinks;
    ConnectionHandler* _connectionHandler;

    enum _COLUMNS {
        ACTION_NAME,
        GAMEPAD,
        KEY,
        INVERT,
        EDIT
    };

    void setUpData();
    void clearAllData(int row);
    void clearGamePadData(int row);
    void clearKeyboardData(int row);
    void tableItemClicked(int row, int column);
    void listenForGamepadInput(QString action, QString actionName);
    void listenForKeyboardInput(QString action, QString actionName);
};

#endif // INPUTMAPWIDGET_H
