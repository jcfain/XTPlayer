#ifndef ADDPLAYLISTDIALOG_H
#define ADDPLAYLISTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include "lib/struct/LibraryListItem.h"

class PlaylistDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PlaylistDialog(QWidget *parent = nullptr);
    static QString getNewPlaylist(QWidget *parent, bool *ok = nullptr);
    static QString renamePlaylist(QWidget *parent, QString playlistName, bool *ok = nullptr);

private:
    QLabel* nameLabel;
    QLineEdit* nameEdit;

    static QString getPlaylistNAme(PlaylistDialog *dialog, bool *ok = nullptr);
};

#endif // ADDPLAYLISTDIALOG_H
