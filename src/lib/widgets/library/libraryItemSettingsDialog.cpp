#include "libraryItemSettingsDialog.h"

#include <cmath>
#include <QCheckBox>
#include <QGroupBox>

#include "lib/handler/loghandler.h"
#include "lib/handler/settingshandler.h"

LibraryItemSettingsDialog::LibraryItemSettingsDialog(QWidget *parent) : QDialog(parent)
{
    int currentRow = 0;
    QGridLayout* layout = new QGridLayout(this);

    globalOffsetLabel = new QLabel(this);
    globalOffsetLabel->setText("Global offset");
    globalOffsetValueLabel = new QLabel(this);
    smartOffsetValueLabel = new QLabel(this);
    updateOffsetLabel();
//    globalOffsetSpinBox = new QSpinBox(this);
//    globalOffsetSpinBox->setSuffix("ms");
//    globalOffsetSpinBox->setSingleStep(1);
//    globalOffsetSpinBox->setReadOnly(true);
//    globalOffsetSpinBox->setMinimum(std::numeric_limits<int>::lowest());
//    globalOffsetSpinBox->setMaximum(std::numeric_limits<int>::max());
//    globalOffsetSpinBox->setValue(SettingsHandler::getoffSet());
    layout->addWidget(globalOffsetLabel, currentRow, 0, 1, 1);
    layout->addWidget(globalOffsetValueLabel, currentRow, 1, 1, 2);
    currentRow++;

    layout->addWidget(smartOffsetValueLabel, currentRow, 1, 1, 2);
    currentRow++;

    offsetLabel = new QLabel(this);
    offsetLabel->setText("Offset");
    offsetSpinBox = new QSpinBox(this);
    offsetSpinBox->setSuffix("ms");
    offsetSpinBox->setSingleStep(1);
    connect(offsetSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        SettingsHandler::setLiveOffset(value);
        _libraryListItemMetaData.offset = value;
        updateOffsetLabel();
    });
    //offsetSpinBox->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    offsetSpinBox->setMinimum(std::numeric_limits<int>::lowest());
    offsetSpinBox->setMaximum(std::numeric_limits<int>::max());
    offsetSpinBox->setValue(_libraryListItemMetaData.offset);
    layout->addWidget(offsetLabel, currentRow, 0, 1, 1);
    layout->addWidget(offsetSpinBox, currentRow, 1, 1, 2);
    currentRow++;

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

    QGroupBox* tagsWidget = new QGroupBox(this);
    tagsWidget->setTitle("Tags");
    QGridLayout* tagsLayout = new QGridLayout(this);
    tagsWidget->setLayout(tagsLayout);

    QStringList userTags = SettingsHandler::getTags();
    int currentTagRow = 0;
    int currentTagColumn = 0;
    int maxRows = round(userTags.count()/4);
    foreach (QString tag, userTags) {
        QCheckBox* tagCheckbox = new QCheckBox(this);
        tagCheckbox->setText(tag);
        tagCheckbox->setChecked(_libraryListItemMetaData.tags.contains(tag));
        connect(tagCheckbox, &QCheckBox::clicked, this, [this, tag](bool checked){
            if(!checked) {
                m_itemTagsToRemove.append(tag);
                m_itemTagsToAdd.removeAll(tag);
            } else if(checked) {
                m_itemTagsToAdd.append(tag);
                m_itemTagsToRemove.removeAll(tag);
            }
        });
        tagsLayout->addWidget(tagCheckbox, currentTagRow, currentTagColumn);
        currentTagRow++;
        if(currentTagRow > maxRows) {
            currentTagRow = 0;
            currentTagColumn++;
        }
    }

    layout->addWidget(moneyShotLabel, currentRow, 0, 1, 1);
    layout->addWidget(moneyShotLineEdit, currentRow, 1, 1, 1);
    layout->addWidget(resetMoneyShotButton, currentRow, 2, 1, 1);
    currentRow++;
    layout->addWidget(tagsWidget, currentRow, 0, 1, 3);
    currentRow++;

    QDialogButtonBox *buttonBox = new QDialogButtonBox
            ( QDialogButtonBox::Save | QDialogButtonBox::Cancel,
              Qt::Horizontal, this );
    layout->addWidget(buttonBox, currentRow, 0, 1, 3);
    currentRow++;

    bool conn = connect(buttonBox, &QDialogButtonBox::accepted,
                   this, &LibraryItemSettingsDialog::accept);
    Q_ASSERT(conn);
    conn = connect(buttonBox, &QDialogButtonBox::rejected,
                   this, &LibraryItemSettingsDialog::reject);
    Q_ASSERT(conn);
    setLayout(layout);
}

void LibraryItemSettingsDialog::getSettings(QWidget *parent, const LibraryListItem27 item, bool *ok)
{
    _libraryListItemMetaData = SettingsHandler::getLibraryListItemMetaData(item);
    LibraryItemSettingsDialog *dialog = new LibraryItemSettingsDialog(parent);
    showDialog(dialog, ok);
}

void LibraryItemSettingsDialog::showDialog(LibraryItemSettingsDialog *dialog, bool *ok)
{
    dialog->offsetSpinBox->setValue(_libraryListItemMetaData.offset);
    //dialog->setModal(false);
    //dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
    const int ret = dialog->exec();
    if (ret)
    {
        if(dialog->offsetSpinBox->text().isEmpty())
        {
            dialog->offsetSpinBox->setValue(0);
            _libraryListItemMetaData.offset = 0;
        }
//        auto ms = dialog->moneyShotLineEdit->text();
//        QRegExp re("\\d*");  // a digit (\d), zero or more times (*)
//        if (re.exactMatch(ms))
//            _libraryListItemMetaData.moneyShotMillis = ms.isEmpty() ? -1 : ms.toLongLong();
        SettingsHandler::setLiveOffset(_libraryListItemMetaData.offset);
        foreach (QString tag, dialog->m_itemTagsToRemove) {
            _libraryListItemMetaData.tags.removeAll(tag);
        }
        foreach (QString tag, dialog->m_itemTagsToAdd) {
            _libraryListItemMetaData.tags.removeAll(tag);
            _libraryListItemMetaData.tags.append(tag);
        }
        SettingsHandler::updateLibraryListItemMetaData(_libraryListItemMetaData);
    }
    dialog->deleteLater();
}

void LibraryItemSettingsDialog::updateOffsetLabel()
{
    auto offsetText = QString::number(SettingsHandler::getoffSet()) + "ms";
    if(_libraryListItemMetaData.offset) {
        globalOffsetValueLabel->setStyleSheet("*{color:red}");
        globalOffsetValueLabel->setText(offsetText + " (overridden)");
    } else {
        globalOffsetValueLabel->setStyleSheet("*{color:green}");
        globalOffsetValueLabel->setText(offsetText + " (in effect)");
    }

    if(SettingsHandler::isSmartOffSet()) {
        smartOffsetValueLabel->setText(QString::number(SettingsHandler::getSmartOffSet()) + "ms (Smart)");
        smartOffsetValueLabel->setStyleSheet("*{color:green}");
    } else {
        smartOffsetValueLabel->setText("Smart offset disabled");
        smartOffsetValueLabel->setStyleSheet("*{color:red}");
    }

}

LibraryListItemMetaData258 LibraryItemSettingsDialog::_libraryListItemMetaData;
