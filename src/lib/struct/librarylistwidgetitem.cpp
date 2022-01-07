#include "librarylistwidgetitem.h"
#include "../tool/imagefactory.h"

LibraryListWidgetItem::LibraryListWidgetItem(LibraryListItem27 &data, QListWidget* parent) :
    QListWidgetItem(data.nameNoExtension, parent)
{
    if (data.isMFS)
    {
        setForeground(QColorConstants::Green);
        setText("(MFS) " + data.nameNoExtension);
    }
    else
        setText(data.nameNoExtension);

    QString toolTip = data.toolTip;
    QFileInfo scriptInfo(data.script);
    QFileInfo zipScriptInfo(data.zipFile);
    if (data.type != LibraryListItemType::PlaylistInternal && !scriptInfo.exists() && !zipScriptInfo.exists())
    {
        toolTip = data.path + "\nNo script file of the same name found.\nRight click and Play with funscript.";
        setForeground(QColorConstants::Gray);
    }

    setToolTip(toolTip);

    QVariant listItem;
    listItem.setValue(data);
    setData(Qt::UserRole, listItem);
    int thumbSize = SettingsHandler::getThumbSize();
    _thumbSize = {thumbSize, thumbSize};

    QString thumbPath = data.thumbFile;
    QFileInfo thumbFile = QFileInfo(thumbPath);
    setThumbFile(thumbFile.exists() ? thumbPath : data.thumbFileLoading);
}

LibraryListWidgetItem::~LibraryListWidgetItem()
{

}

LibraryListItem27 LibraryListWidgetItem::getLibraryListItem()
{
    return data(Qt::UserRole).value<LibraryListItem27>();
}

LibraryListItemType LibraryListWidgetItem::getType()
{
    return getLibraryListItem().type;
}

bool LibraryListWidgetItem::operator< (const QListWidgetItem & other) const
{
    LibraryListItem27 otherData = other.data(Qt::UserRole).value<LibraryListItem27>();
    LibraryListItem27 thisData = data(Qt::UserRole).value<LibraryListItem27>();

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

void LibraryListWidgetItem::updateThumbSize(QSize thumbSize)
{
    _thumbSize = thumbSize;
    QFileInfo thumbFile = QFileInfo(_thumbFile);
    QString thumbFilePath = _thumbFile;
    if(!thumbFile.exists())
    {
        thumbFilePath = "://images/icons/loading.png";
    }
    QIcon thumb;
    thumb.addPixmap(ImageFactory::resize(thumbFilePath, thumbSize));
    setIcon(thumb);
    setSizeHint(thumbSize);
    setTextAlignment(Qt::AlignmentFlag::AlignTop | Qt::AlignmentFlag::AlignHCenter);
}

void LibraryListWidgetItem::setThumbFile(QString filePath, QString errorMessage)
{
    _thumbFile = filePath;
    auto data = getLibraryListItem();
    if(!errorMessage.isEmpty())
    {
        _thumbFile = data.thumbFileError;
        setToolTip(toolTip() + "\n"+ errorMessage);
    }
    else if(!_thumbFile.startsWith(":"))
    {
        data.thumbFile = _thumbFile;
        data.thumbFileExists = QFileInfo(_thumbFile).exists();
        QVariant listItem;
        listItem.setValue(data);
        setData(Qt::UserRole, listItem);
    }
    updateThumbSize(_thumbSize);
}

LibraryListWidgetItem* LibraryListWidgetItem::clone() const
{
   return new LibraryListWidgetItem(*this);
}

QSize LibraryListWidgetItem::calculateHintSize(QSize size)
{
    return {size.width(), size.height()-size.height()/7};
}
LibrarySortMode LibraryListWidgetItem::_sortMode = LibrarySortMode::NAME_ASC;
