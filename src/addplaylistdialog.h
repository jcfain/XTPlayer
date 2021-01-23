#ifndef ADDPLAYLISTDIALOG_H
#define ADDPLAYLISTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include "lib/struct/LibraryListItem.h"

class AddPlaylistDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddPlaylistDialog(QWidget *parent = nullptr);
    static QString getNewPlaylist(QWidget *parent, bool *ok = nullptr);

private:
    QLabel* nameLabel;
    QLineEdit* nameEdit;

signals:

};

#endif // ADDPLAYLISTDIALOG_H
