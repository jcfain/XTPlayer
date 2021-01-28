#include "addplaylistdialog.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include "lib/handler/loghandler.h"

PlaylistDialog::PlaylistDialog(QWidget *parent) : QDialog(parent)
{
    nameLabel = new QLabel(this);
    nameLabel->setText("Name");
    nameEdit = new QLineEdit(this);
    nameEdit->setText("New playlist");
    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(nameLabel, 0, 0, 1, 1);
    layout->addWidget(nameEdit, 0, 1, 1, 1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox
            ( QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
              Qt::Horizontal, this );
    layout->addWidget(buttonBox, 1, 0, 1, 2);

    bool conn = connect(buttonBox, &QDialogButtonBox::accepted,
                   this, &PlaylistDialog::accept);
    Q_ASSERT(conn);
    conn = connect(buttonBox, &QDialogButtonBox::rejected,
                   this, &PlaylistDialog::reject);
    Q_ASSERT(conn);
    setLayout(layout);
}

QString PlaylistDialog::getNewPlaylist(QWidget *parent, bool *ok)
{
    PlaylistDialog *dialog = new PlaylistDialog(parent);
    return getPlaylistNAme(dialog, ok);
}


QString PlaylistDialog::renamePlaylist(QWidget *parent, QString playlistName, bool *ok)
{
    PlaylistDialog *dialog = new PlaylistDialog(parent);
    dialog->nameEdit->setText(playlistName);
    return getPlaylistNAme(dialog, ok);
}

QString PlaylistDialog::getPlaylistNAme(PlaylistDialog *dialog, bool *ok)
{
    const int ret = dialog->exec();
    if (ok)
        *ok = !!ret;
    if (ok)
    {
        bool isValid = true;
        if(dialog->nameEdit->text().isEmpty())
        {
            isValid = false;
            LogHandler::Dialog("Playlist name is required!", XLogLevel::Critical);
        }
        if (!isValid)
            *ok = false;
    }

    dialog->deleteLater();

    return ok ? dialog->nameEdit->text() : nullptr;
}
