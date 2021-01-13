#include "channeltablecomboboxdelegate.h"

ChannelTableComboboxDelegate::ChannelTableComboboxDelegate(QObject *parent)
 : QStyledItemDelegate(parent)
{

}

ChannelTableComboboxDelegate::~ChannelTableComboboxDelegate() { }

void ChannelTableComboboxDelegate::setData(QMap<QString, int> data)
{
    _data = data;
}

QWidget *ChannelTableComboboxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // Create the combobox and populate it
     QComboBox *cb = new QComboBox(parent);
     const int row = index.row();
     foreach(auto item, _data.keys())
     {
        cb->addItem(item, _data[item]);
     }
     return cb;
}

void ChannelTableComboboxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
     Q_ASSERT(cb);
     // get the index of the text in the combobox that matches the current value of the item
     const int currentData = index.data(Qt::DisplayRole).toInt();
     const int cbIndex = cb->findData(currentData);
     // if it is valid, adjust the combobox
     if (cbIndex >= 0)
        cb->setCurrentIndex(cbIndex);
}

void ChannelTableComboboxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    QComboBox *cb = qobject_cast<QComboBox *>(editor);
     Q_ASSERT(cb);
     auto suc = model->setData(index, cb->currentData(), Qt::EditRole);
}
