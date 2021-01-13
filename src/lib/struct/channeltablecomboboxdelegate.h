#ifndef CHANNELTABLECOMBOBOXDELEGATE_H
#define CHANNELTABLECOMBOBOXDELEGATE_H

#include <QStyledItemDelegate >
#include <QObject>
#include <QComboBox>
#include <QMap>

class ChannelTableComboboxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ChannelTableComboboxDelegate(QObject *parent = nullptr);
    ~ChannelTableComboboxDelegate();

    void setData(QMap<QString, int> data);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

private:
    QMap<QString, int> _data;
};

#endif // CHANNELTABLECOMBOBOXDELEGATE_H
