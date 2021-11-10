#include "librarylistwidgetitem.h"

LibraryListWidgetItem::LibraryListWidgetItem(LibraryListItem &localData, QListWidget* parent) :
    QListWidgetItem(localData.nameNoExtension, parent)
{
    _isMFS = updateToolTip(localData);

    localData.thumbFile = SettingsHandler::getSelectedThumbsDir() + localData.name + ".jpg";
    if(localData.type == LibraryListItemType::Audio)
        localData.thumbFile = "://images/icons/audio.png";
    else if(localData.type == LibraryListItemType::PlaylistInternal)
        localData.thumbFile = "://images/icons/playlist.png";
    else if(localData.type == LibraryListItemType::FunscriptType)
        localData.thumbFile = "://images/icons/funscript.png";
    _thumbFile = localData.thumbFile;
    int thumbSize = SettingsHandler::getThumbSize();
    updateThumbSize({thumbSize, thumbSize});
    if (_isMFS)
    {
        setForeground(QColorConstants::Green);
        setText("(MFS) " + localData.nameNoExtension);
    }
    else
        setText(localData.nameNoExtension);
    QVariant listItem;

    listItem.setValue(localData);
    setData(Qt::UserRole, listItem);
}

LibraryListWidgetItem::~LibraryListWidgetItem()
{

}

bool LibraryListWidgetItem::isMFS()
{
    return _isMFS;
}

bool LibraryListWidgetItem::updateToolTip(LibraryListItem localData)
{
    bool mfs = false;
    QFileInfo scriptInfo(localData.script);
    QFileInfo zipScriptInfo(localData.zipFile);
    QString toolTip = localData.nameNoExtension + "\nMedia:";
    if (localData.type != LibraryListItemType::PlaylistInternal && !scriptInfo.exists() && !zipScriptInfo.exists())
    {
        toolTip = localData.path + "\nNo script file of the same name found.\nRight click and Play with funscript.";
        setForeground(QColorConstants::Gray);
    }
    else if (localData.type != LibraryListItemType::PlaylistInternal)
    {
        toolTip += "\n";
        toolTip += localData.path;
        toolTip += "\n";
        toolTip += "Scripts:\n";
        if(zipScriptInfo.exists())
        {
            toolTip += localData.zipFile;
            mfs = true;
        }
        else
        {
            toolTip += localData.script;
        }
        auto availibleAxis = SettingsHandler::getAvailableAxis();
        foreach(auto axisName, availibleAxis->keys())
        {
            auto track = availibleAxis->value(axisName);
            if(axisName == TCodeChannelLookup::Stroke() || track.Type == AxisType::HalfRange || track.TrackName.isEmpty())
                continue;

            QString script = localData.scriptNoExtension + "." + track.TrackName + ".funscript";
            QFileInfo fileInfo(script);
            if (fileInfo.exists())
            {
                mfs = true;
                toolTip += "\n";
                toolTip += script;
            }
        }
    }
    else if (localData.type == LibraryListItemType::PlaylistInternal)
    {
        auto playlists = SettingsHandler::getPlaylists();
        auto playlist = playlists.value(localData.nameNoExtension);
        for(auto i = 0; i < playlist.length(); i++)
        {
            toolTip += "\n";
            toolTip += QString::number(i + 1);
            toolTip += ": ";
            toolTip += playlist[i].nameNoExtension;
        }
    }
    setToolTip(toolTip);
    return mfs;
}

LibraryListItem LibraryListWidgetItem::getLibraryListItem()
{
    return data(Qt::UserRole).value<LibraryListItem>();
}

LibraryListItemType LibraryListWidgetItem::getType()
{
    return getLibraryListItem().type;
}

bool LibraryListWidgetItem::operator< (const QListWidgetItem & other) const
{
    LibraryListItem otherData = other.data(Qt::UserRole).value<LibraryListItem>();
    LibraryListItem thisData = data(Qt::UserRole).value<LibraryListItem>();

    if(_sortMode != NONE)
    {
        if(thisData.type == LibraryListItemType::PlaylistInternal && otherData.type != LibraryListItemType::PlaylistInternal)
        {
            return true;
        }
        else if(thisData.type != LibraryListItemType::PlaylistInternal && otherData.type == LibraryListItemType::PlaylistInternal)
        {
            return false;
        }
        else if(thisData.type == LibraryListItemType::PlaylistInternal && otherData.type == LibraryListItemType::PlaylistInternal)
        {
            return otherData.nameNoExtension.localeAwareCompare(thisData.nameNoExtension) > 0;
        }
        else if(_sortMode == LibrarySortMode::RANDOM)
        {
    //            qint64 randomValue = XMath::rand(0, 100);
    //            if(randomValue > 50)
    //                return thisData.modifiedDate < otherData.modifiedDate;
            return false;
        }
        else if(_sortMode == LibrarySortMode::CREATED_ASC)
        {
            return thisData.modifiedDate < otherData.modifiedDate;
    //            auto d1 = thisData.modifiedDate;
    //            auto d2 = otherData.modifiedDate;
    //            if (d1.year() < d2.year())
    //                return true;
    //            if (d1.year() == d2.year() && d1.month() < d2.month())
    //                return true;
    //            if (d1.year() == d2.year() && d1.month() == d2.month() && d1.day() < d2.day())
    //                return true;

    //            return false;
        }
        else if(_sortMode == LibrarySortMode::CREATED_DESC)
        {
            return thisData.modifiedDate > otherData.modifiedDate;
    //            auto d1 = thisData.modifiedDate;
    //            auto d2 = otherData.modifiedDate;
    //            if (d1.year() > d2.year())
    //                return true;
    //            if (d1.year() == d2.year() && d1.month() > d2.month())
    //                return true;
    //            if (d1.year() == d2.year() && d1.month() == d2.month() && d1.day() > d2.day())
    //                return true;

    //            return false;
        }
        else if(_sortMode == LibrarySortMode::NAME_DESC)
        {
            return thisData.name.localeAwareCompare(otherData.name) > 0;
        }
        else if(_sortMode == LibrarySortMode::NAME_ASC)
        {

        }
        else if(_sortMode == LibrarySortMode::TYPE_ASC)
        {
            if (thisData.type == otherData.type)
              return otherData.name.localeAwareCompare(thisData.name) > 0;
            return thisData.type < otherData.type;
        }
        else if(_sortMode == LibrarySortMode::TYPE_DESC)
        {
            if (thisData.type == otherData.type)
              return thisData.name.localeAwareCompare(otherData.name) > 0;
            return thisData.type > otherData.type;
        }
    }
    // otherwise just return the comparison result from the base class
    return QListWidgetItem::operator < (other);
}

bool LibraryListWidgetItem::operator == (const LibraryListWidgetItem & other) const
{
    return other.text() == text();
}

void LibraryListWidgetItem::setSortMode(LibrarySortMode sortMode)
{
    _sortMode = sortMode;
}

void LibraryListWidgetItem::updateThumbSize(QSize thumbSize, QString filePath)
{
    if(!filePath.isEmpty())
    {
        QFileInfo thumbFile = QFileInfo(filePath);
        if (thumbFile.exists())
        {
            _thumbFile = filePath;
        }
    }
    auto data = getLibraryListItem();


    QFileInfo thumbFile = QFileInfo(_thumbFile);
    if(thumbFile.exists())
    {
        data.thumbFile = _thumbFile;
        QVariant listItem;
        listItem.setValue(data);
        setData(Qt::UserRole, listItem);
    }
    QPixmap bgPixmap = QPixmap(!thumbFile.exists() ? "://images/icons/loading.png" : _thumbFile);
    QIcon thumb;
    //QSize maxThumbSize = SettingsHandler::getMaxThumbnailSize();
    //int newHeight = round((float)bgPixmap.height() / bgPixmap.width() * 1080);
    //QSize newSize = calculateSize(thumbSize);
    QPixmap scaled = bgPixmap.scaled(thumbSize, Qt::AspectRatioMode::KeepAspectRatio);
    QSize maxHeight = calculateMaxSize(thumbSize);

    if(scaled.height() > maxHeight.height())
    {
        scaled = bgPixmap.scaled(maxHeight, Qt::AspectRatioMode::KeepAspectRatio);
//        QRect rect(0,0,scaled.width(), newHeight);
//        scaled = scaled.copy(rect);
    }
    thumb.addPixmap(scaled);
    setIcon(thumb);
    setSizeHint(thumbSize);
    setTextAlignment(Qt::AlignmentFlag::AlignTop | Qt::AlignmentFlag::AlignHCenter);
}

LibraryListWidgetItem* LibraryListWidgetItem::clone() const
{
   return new LibraryListWidgetItem(*this);
}

QSize LibraryListWidgetItem::calculateMaxSize(QSize size)
{
    return {size.width(), (int)round(size.height()-size.height()/3.5)};
}

QSize LibraryListWidgetItem::calculateHintSize(QSize size)
{
    return {size.width(), size.height()-size.height()/7};
}
LibrarySortMode LibraryListWidgetItem::_sortMode = LibrarySortMode::NAME_ASC;
