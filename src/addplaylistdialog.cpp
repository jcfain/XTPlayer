#include "addplaylistdialog.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include "lib/handler/loghandler.h"

AddPlaylistDialog::AddPlaylistDialog(QWidget *parent) : QDialog(parent)
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
                   this, &AddPlaylistDialog::accept);
    Q_ASSERT(conn);
    conn = connect(buttonBox, &QDialogButtonBox::rejected,
                   this, &AddPlaylistDialog::reject);
    Q_ASSERT(conn);
    setLayout(layout);
}

QString AddPlaylistDialog::getNewPlaylist(QWidget *parent, bool *ok)
{
    AddPlaylistDialog *dialog = new AddPlaylistDialog(parent);
    const int ret = dialog->exec();
    if (ok)
        *ok = !!ret;
    if (*ok)
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

    return dialog->nameEdit->text();
}
