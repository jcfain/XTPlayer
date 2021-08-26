#include "libraryItemSettingsDialog.h"

LibraryItemSettingsDialog::LibraryItemSettingsDialog(QString key, QWidget *parent) : QDialog(parent)
{
    _libraryListItemMetaData = SettingsHandler::getLibraryListItemMetaData(key);
    offsetLabel = new QLabel(this);
    offsetLabel->setText("Offset");
    offsetSpinBox = new QSpinBox(this);
    offsetSpinBox->setSuffix("ms");
    offsetSpinBox->setSingleStep(1);
    connect(offsetSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [](int value) {
        SettingsHandler::setLiveOffset(value);
    });
    //offsetSpinBox->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    offsetSpinBox->setMinimum(std::numeric_limits<int>::lowest());
    offsetSpinBox->setMaximum(std::numeric_limits<int>::max());
    offsetSpinBox->setValue(_libraryListItemMetaData.offset);
    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(offsetLabel, 0, 0, 1, 1);
    layout->addWidget(offsetSpinBox, 0, 1, 1, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox
            ( QDialogButtonBox::Save | QDialogButtonBox::Cancel,
              Qt::Horizontal, this );
    layout->addWidget(buttonBox, 1, 0, 1, 2);

    bool conn = connect(buttonBox, &QDialogButtonBox::accepted,
                   this, &LibraryItemSettingsDialog::accept);
    Q_ASSERT(conn);
    conn = connect(buttonBox, &QDialogButtonBox::rejected,
                   this, &LibraryItemSettingsDialog::reject);
    Q_ASSERT(conn);
    setLayout(layout);
}

void LibraryItemSettingsDialog::getSettings(QWidget *parent, QString key, bool *ok)
{
    LibraryItemSettingsDialog *dialog = new LibraryItemSettingsDialog(key, parent);
    showDialog(dialog, ok);
}

void LibraryItemSettingsDialog::showDialog(LibraryItemSettingsDialog *dialog, bool *ok)
{
    dialog->offsetSpinBox->setValue(_libraryListItemMetaData.offset);
    const int ret = dialog->exec();
    if (ret)
    {
        if(dialog->offsetSpinBox->text().isEmpty())
        {
            dialog->offsetSpinBox->setValue(0);
        }
        _libraryListItemMetaData.offset = dialog->offsetSpinBox->value();
        SettingsHandler::setLiveOffset(_libraryListItemMetaData.offset);
        SettingsHandler::updateLibraryListItemMetaData(_libraryListItemMetaData);
    }
    dialog->deleteLater();
}

LibraryListItemMetaData258 LibraryItemSettingsDialog::_libraryListItemMetaData;
