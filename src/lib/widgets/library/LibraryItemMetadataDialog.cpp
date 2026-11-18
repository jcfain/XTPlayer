#include "LibraryItemMetadataDialog.h"

#include <cmath>
#include <QCheckBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QUrl>
#include <QVBoxLayout>

#include "lib/handler/loghandler.h"
#include "lib/handler/settingshandler.h"
#include "lib/handler/funscripthandler.h"
#include "lib/handler/xmediastatehandler.h"
#include "lib/struct/LibraryListItemMetaData258.h"

LibraryItemMetadataDialog::LibraryItemMetadataDialog(QWidget *parent) : QDialog(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    m_tabWidget = new QTabWidget(this);
    mainLayout->addWidget(m_tabWidget);

    // ----- Settings tab -----
    QWidget* settingsTab = new QWidget(this);
    int currentRow = 0;
    QGridLayout* layout = new QGridLayout(settingsTab);

    globalOffsetLabel = new QLabel(settingsTab);
    globalOffsetLabel->setText(tr("Global offset"));
    globalOffsetValueLabel = new QLabel(settingsTab);
    // smartOffsetValueLabel = new QLabel(this);
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

    // layout->addWidget(smartOffsetValueLabel, currentRow, 1, 1, 2);
    // currentRow++;

    offsetLabel = new QLabel(settingsTab);
    offsetLabel->setText(tr("Offset"));
    offsetSpinBox = new QSpinBox(settingsTab);
    offsetSpinBox->setSuffix("ms");
    offsetSpinBox->setSingleStep(SettingsHandler::getFunscriptOffsetStep());
    connect(offsetSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        auto playingMediaID = XMediaStateHandler::getPlayingID();
        if(!playingMediaID.isEmpty() && playingMediaID == _libraryListItem->ID)
            FunscriptHandler::setScriptOffset(value);
        updateOffsetLabel();
    });
    //offsetSpinBox->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    offsetSpinBox->setMinimum(std::numeric_limits<int>::lowest());
    offsetSpinBox->setMaximum(std::numeric_limits<int>::max());
    offsetSpinBox->setValue(_libraryListItem->metadata.offset);
    layout->addWidget(offsetLabel, currentRow, 0, 1, 1);
    layout->addWidget(offsetSpinBox, currentRow, 1, 1, 2);
    currentRow++;

    QLabel* funscriptModifierLabel = new QLabel(settingsTab);
    funscriptModifierLabel->setText(tr("Funscript range"));
    funscriptModifierSpinBox = new QDoubleSpinBox(settingsTab);
    funscriptModifierSpinBox->setMinimum(std::numeric_limits<double>::lowest());
    funscriptModifierSpinBox->setMaximum(std::numeric_limits<double>::max());
    funscriptModifierSpinBox->setSuffix("%");
    funscriptModifierSpinBox->setSingleStep(SettingsHandler::getFunscriptModifierStep());
    funscriptModifierSpinBox->setValue(_libraryListItem->metadata.funscriptModifier);
    connect(funscriptModifierSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [](int value) {
        auto playingMediaID = XMediaStateHandler::getPlayingID();
        if(!playingMediaID.isEmpty() && playingMediaID == _libraryListItem->ID)
            FunscriptHandler::setModifier(value);
    });
    layout->addWidget(funscriptModifierLabel, currentRow, 0, 1, 1);
    layout->addWidget(funscriptModifierSpinBox, currentRow, 1, 1, 2);
    currentRow++;

    moneyShotLabel = new QLabel(settingsTab);
    moneyShotLabel->setText(tr("Money shot"));

    moneyShotLineEdit = new QLineEdit(settingsTab);
    connect(moneyShotLineEdit, &QLineEdit::textEdited, this, [](QString value) {
        QRegularExpression re("\\d*");  // a digit (\d), zero or more times (*)
        if (!re.match(value).hasMatch())
            LogHandler::Error("Invalid value, money shot should be in milliseconds");
    });
    QString maxLong = QString::number(INT64_MAX);
    moneyShotLineEdit->setMaxLength(maxLong.length());
    // moneyShotLineEdit->setReadOnly(true);
    moneyShotLineEdit->setText(QString::number(_libraryListItem->metadata.moneyShotMillis));

    resetMoneyShotButton = new QPushButton(settingsTab);
    resetMoneyShotButton->setText(tr("Reset"));
    connect(resetMoneyShotButton, &QPushButton::clicked, this, [this](bool checked) {
        moneyShotLineEdit->setText(QString::number(-1));
        _libraryListItem->metadata.moneyShotMillis = -1;
    });

    layout->addWidget(moneyShotLabel, currentRow, 0, 1, 1);
    layout->addWidget(moneyShotLineEdit, currentRow, 1, 1, 1);
    layout->addWidget(resetMoneyShotButton, currentRow, 2, 1, 1);
    m_tabWidget->addTab(settingsTab, tr("Settings"));

    // ----- Tags tab -----
    QWidget* tagsTab = new QWidget(this);
    QVBoxLayout* tagsTabLayout = new QVBoxLayout(tagsTab);

    QGroupBox* tagsWidget = new QGroupBox(tagsTab);
    tagsWidget->setTitle(tr("Tags"));
    QGridLayout* tagsLayout = new QGridLayout(tagsWidget);
    tagsWidget->setLayout(tagsLayout);

    QStringList userTags = SettingsHandler::getTags();
    int currentTagRow = 0;
    int currentTagColumn = 0;
    int maxRows = round(userTags.count()/4);
    foreach (QString tag, userTags) {
        QCheckBox* tagCheckbox = new QCheckBox(tagsTab);
        tagCheckbox->setText(tag);
        tagCheckbox->setChecked(_libraryListItem->metadata.tags.contains(tag));
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

    tagsTabLayout->addWidget(tagsWidget);
    m_tabWidget->addTab(tagsTab, tr("Tags"));

    // ----- Info tab -----
    m_tabWidget->addTab(buildInfoTab(), tr("Info"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox
            ( QDialogButtonBox::Save | QDialogButtonBox::Cancel,
              Qt::Horizontal, this );
    mainLayout->addWidget(buttonBox);

    bool conn = connect(buttonBox, &QDialogButtonBox::accepted,
                   this, &LibraryItemMetadataDialog::accept);
    Q_ASSERT(conn);
    conn = connect(buttonBox, &QDialogButtonBox::rejected,
                   this, &LibraryItemMetadataDialog::reject);
    Q_ASSERT(conn);
}

QWidget* LibraryItemMetadataDialog::buildInfoTab()
{
    LibraryListItem27* item = _libraryListItem;

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    QWidget* content = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(content);

    QFormLayout* form = new QFormLayout();
    form->addRow(tr("Name:"), new QLabel(item->name));
    form->addRow(tr("Path:"), new QLabel(item->path));
    form->addRow(tr("Date Added:"), new QLabel(item->metadata.dateAdded.isValid() ? item->metadata.dateAdded.toString() : tr("Unknown")));
    form->addRow(tr("Date Created:"), new QLabel(item->modifiedDate.isValid() ? item->modifiedDate.toString() : tr("Unknown")));
    form->addRow(tr("Alt script(s):"), new QLabel(item->metadata.hasAlternate ? tr("Yes") : tr("No")));

    // Scripts: one group box per script (always visible, no collapsible drawers).
    QWidget* scriptsWidget = new QWidget();
    QVBoxLayout* scriptsLayout = new QVBoxLayout(scriptsWidget);
    scriptsLayout->setContentsMargins(0, 0, 0, 0);
    if (item->metadata.scripts.isEmpty())
    {
        scriptsLayout->addWidget(new QLabel(tr("No scripts")));
    }
    for (const ScriptInfo& script : item->metadata.scripts)
    {
        QString title = script.filename;
        if (!script.track.isEmpty())
            title += " " + script.track;
        QGroupBox* scriptBox = new QGroupBox(title);
        QFormLayout* scriptForm = new QFormLayout(scriptBox);
        scriptForm->addRow(tr("Version:"), new QLabel(script.version.isEmpty() ? tr("Unknown") : script.version));
        scriptForm->addRow(tr("Inverted:"), new QLabel(script.hasInverted ? (script.inverted ? tr("Yes") : tr("No")) : tr("Unknown")));
        scriptForm->addRow(tr("Creator:"), new QLabel(script.metadata.creator.isEmpty() ? tr("Unknown") : script.metadata.creator));
        scriptForm->addRow(tr("Original name:"), new QLabel(script.metadata.original_name.isEmpty() ? tr("Unknown") : script.metadata.original_name));
        if (!script.metadata.url.isEmpty())
        {
            QLabel* urlLabel = new QLabel(QString("<a href=\"%1\">%2</a>").arg(QUrl::toPercentEncoding(script.metadata.url), script.metadata.url));
            urlLabel->setOpenExternalLinks(true);
            scriptForm->addRow(tr("URL:"), urlLabel);
        }
        else
        {
            scriptForm->addRow(tr("URL:"), new QLabel(tr("Unknown")));
        }
        if (!script.metadata.url_video.isEmpty())
        {
            QLabel* videoUrlLabel = new QLabel(QString("<a href=\"%1\">%2</a>").arg(QUrl::toPercentEncoding(script.metadata.url_video), script.metadata.url_video));
            videoUrlLabel->setOpenExternalLinks(true);
            scriptForm->addRow(tr("Video URL:"), videoUrlLabel);
        }
        else
        {
            scriptForm->addRow(tr("Video URL:"), new QLabel(tr("Unknown")));
        }
        scriptForm->addRow(tr("Performers:"), new QLabel(script.metadata.performers.isEmpty() ? tr("Unknown") : script.metadata.performers.join(", ")));
        scriptForm->addRow(tr("Tags:"), new QLabel(script.metadata.tags.isEmpty() ? tr("Unknown") : script.metadata.tags.join(", ")));
        scriptForm->addRow(tr("Paid:"), new QLabel(script.metadata.hasPaid ? (script.metadata.paid ? tr("Yes") : tr("No")) : tr("Unknown")));
        scriptForm->addRow(tr("Comment:"), new QLabel(script.metadata.comment.isEmpty() ? tr("Unknown") : script.metadata.comment));
        scriptForm->addRow(tr("Original duration:"), new QLabel(formatOriginalDuration(script.metadata.original_total_duration_ms)));

        // Bookmarks: one line per bookmark.
        QWidget* bookmarksWidget = new QWidget();
        QVBoxLayout* bookmarksLayout = new QVBoxLayout(bookmarksWidget);
        bookmarksLayout->setContentsMargins(0, 0, 0, 0);
        if (script.metadata.bookmarks.isEmpty())
        {
            bookmarksLayout->addWidget(new QLabel(tr("Unknown")));
        }
        for (const FunscriptBookmark& bookmark : script.metadata.bookmarks)
        {
            bookmarksLayout->addWidget(new QLabel(bookmark.name + " — " + formatTime(bookmark.time)));
        }
        scriptForm->addRow(tr("Bookmarks:"), bookmarksWidget);

        // Chapters: one line per chapter.
        QWidget* chaptersWidget = new QWidget();
        QVBoxLayout* chaptersLayout = new QVBoxLayout(chaptersWidget);
        chaptersLayout->setContentsMargins(0, 0, 0, 0);
        if (script.metadata.chapters.isEmpty())
        {
            chaptersLayout->addWidget(new QLabel(tr("Unknown")));
        }
        for (const FunscriptChapter& chapter : script.metadata.chapters)
        {
            chaptersLayout->addWidget(new QLabel(chapter.name + " — " + formatTime(chapter.startTime) + " to " + formatTime(chapter.endTime)));
        }
        scriptForm->addRow(tr("Chapters:"), chaptersWidget);

        scriptsLayout->addWidget(scriptBox);
    }
    form->addRow(tr("Scripts:"), scriptsWidget);

    // Tracks.
    QWidget* tracksWidget = new QWidget();
    QVBoxLayout* tracksLayout = new QVBoxLayout(tracksWidget);
    tracksLayout->setContentsMargins(0, 0, 0, 0);
    if (item->metadata.tracks.isEmpty())
    {
        tracksLayout->addWidget(new QLabel(item->metadata.scripts.isEmpty() ? tr("No tracks") : tr("No additional tracks")));
    }
    else
    {
        for (const QString& track : item->metadata.tracks)
        {
            tracksLayout->addWidget(new QLabel(track));
        }
    }
    form->addRow(tr("Tracks:"), tracksWidget);

    contentLayout->addLayout(form);
    contentLayout->addStretch();
    scrollArea->setWidget(content);
    return scrollArea;
}

QString LibraryItemMetadataDialog::formatTime(qint64 ms)
{
    if (ms < 0)
        return QString();
    int totalSeconds = (int)(ms / 1000);
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;
    if (hours > 0)
        return QString("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    return QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'));
}

QString LibraryItemMetadataDialog::formatOriginalDuration(qint64 ms)
{
    if (ms < 0)
        return tr("Unknown");
    return QString::number(ms) + " (" + formatTime(ms) + ")";
}

void LibraryItemMetadataDialog::getSettings(QWidget *parent, LibraryListItem27* item, SyncHandler* syncHandler, bool *ok)
{
    if(item)
    {
        _libraryListItem = item;
        LibraryItemMetadataDialog *dialog = new LibraryItemMetadataDialog(parent);
        connect(dialog, &LibraryItemMetadataDialog::save, syncHandler, &SyncHandler::updateMetadata);
        showDialog(dialog, ok);
    }
}

void LibraryItemMetadataDialog::showDialog(LibraryItemMetadataDialog *dialog, bool *ok)
{
    dialog->offsetSpinBox->setValue(_libraryListItem->metadata.offset);
    //dialog->setModal(false);
    //dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
    const int ret = dialog->exec();
    if (ret)
    {
        if(dialog->offsetSpinBox->text().isEmpty())
        {
            dialog->offsetSpinBox->setValue(0);
        }
        if(dialog->funscriptModifierSpinBox->text().isEmpty())
        {
            dialog->funscriptModifierSpinBox->setValue(100.0);
        }
//        auto ms = dialog->moneyShotLineEdit->text();
//        QRegExp re("\\d*");  // a digit (\d), zero or more times (*)
//        if (re.exactMatch(ms))
//            _libraryListItemMetaData.moneyShotMillis = ms.isEmpty() ? -1 : ms.toLongLong();
        _libraryListItem->metadata.offset = dialog->offsetSpinBox->value();
        _libraryListItem->metadata.funscriptModifier = dialog->funscriptModifierSpinBox->value();
        qint64 moneyShot = dialog->moneyShotLineEdit->text().toLongLong();
        _libraryListItem->metadata.moneyShotMillis = moneyShot > 0 ? moneyShot : -1;
        foreach (QString tag, dialog->m_itemTagsToRemove) {
            _libraryListItem->metadata.tags.removeAll(tag);
        }
        foreach (QString tag, dialog->m_itemTagsToAdd) {
            _libraryListItem->metadata.tags.removeAll(tag);
            _libraryListItem->metadata.tags.append(tag);
        }
        SettingsHandler::updateLibraryListItemMetaData(*_libraryListItem, true);
        emit dialog->save(_libraryListItem->metadata);
    }
    else
    {
        FunscriptHandler::updateMetadata(_libraryListItem->metadata);
    }
    dialog->deleteLater();
}

void LibraryItemMetadataDialog::updateOffsetLabel()
{
    auto offsetText = QString::number(SettingsHandler::getGlobalOffSet()) + "ms";
    if(_libraryListItem->metadata.offset) {
        globalOffsetValueLabel->setStyleSheet("*{color:red}");
        globalOffsetValueLabel->setText(offsetText + tr(" (overridden)"));
    } else {
        globalOffsetValueLabel->setStyleSheet("*{color:green}");
        globalOffsetValueLabel->setText(offsetText + tr(" (in effect)"));
    }

    // if(SettingsHandler::isSmartOffSet()) {
    //     smartOffsetValueLabel->setText(QString::number(SettingsHandler::getSmartOffSet()) + tr("ms (Smart)"));
    //     smartOffsetValueLabel->setStyleSheet("*{color:green}");
    // } else {
    //     smartOffsetValueLabel->setText(tr("Smart offset disabled"));
    //     smartOffsetValueLabel->setStyleSheet("*{color:red}");
    // }

}

LibraryListItem27* LibraryItemMetadataDialog::_libraryListItem;
