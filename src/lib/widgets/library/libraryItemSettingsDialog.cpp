#include "libraryItemSettingsDialog.h"

LibraryItemSettingsDialog::LibraryItemSettingsDialog(QWidget *parent) : QDialog(parent)
{
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
    layout->addWidget(offsetSpinBox, 0, 1, 1, 2);

    moneyShotLabel = new QLabel(this);
    moneyShotLabel->setText("Money shot");

    moneyShotLineEdit = new QLineEdit(this);
    connect(moneyShotLineEdit, &QLineEdit::textEdited, this, [](QString value) {
        QRegExp re("\\d*");  // a digit (\d), zero or more times (*)
        if (!re.exactMatch(value))
            LogHandler::Error("Invalid value, money shot should be in milliseconds");
    });
    QString maxLong = "9223372036854775807";
    moneyShotLineEdit->setMaxLength(maxLong.length());
    moneyShotLineEdit->setReadOnly(true);
    moneyShotLineEdit->setText(QString::number(_libraryListItemMetaData.moneyShotMillis));

    resetMoneyShotButton = new QPushButton(this);
    resetMoneyShotButton->setText("Reset");
    connect(resetMoneyShotButton, &QPushButton::clicked, this, [this](bool checked) {
        moneyShotLineEdit->setText(QString::number(-1));
        _libraryListItemMetaData.moneyShotMillis = -1;
    });
    layout->addWidget(moneyShotLabel, 1, 0, 1, 1);
    layout->addWidget(moneyShotLineEdit, 1, 1, 1, 1);
    layout->addWidget(resetMoneyShotButton, 1, 2, 1, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox
            ( QDialogButtonBox::Save | QDialogButtonBox::Cancel,
              Qt::Horizontal, this );
    layout->addWidget(buttonBox, 2, 0, 1, 3);

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
    _libraryListItemMetaData = SettingsHandler::getLibraryListItemMetaData(key);
    LibraryItemSettingsDialog *dialog = new LibraryItemSettingsDialog(parent);
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
//        auto ms = dialog->moneyShotLineEdit->text();
//        QRegExp re("\\d*");  // a digit (\d), zero or more times (*)
//        if (re.exactMatch(ms))
//            _libraryListItemMetaData.moneyShotMillis = ms.isEmpty() ? -1 : ms.toLongLong();
        SettingsHandler::setLiveOffset(_libraryListItemMetaData.offset);
        SettingsHandler::updateLibraryListItemMetaData(_libraryListItemMetaData);
    }
    dialog->deleteLater();
}

LibraryListItemMetaData258 LibraryItemSettingsDialog::_libraryListItemMetaData;
