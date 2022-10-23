#ifndef ADDCHANNELDIALOG_H
#define ADDCHANNELDIALOG_H

#include <QObject>
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QRadioButton>
#include <QComboBox>
#include <QGridLayout>
#include <QDialogButtonBox>
#include "lib/struct/ChannelModel33.h"
#include "lib/handler/settingshandler.h"
#include "lib/handler/dialoghandler.h"
#include "lib/lookup/TCodeVersion.h"


class AddChannelDialog : public QDialog
{
    Q_OBJECT
public:
    AddChannelDialog(QWidget* parent = nullptr);

    static ChannelModel33 getNewChannel(QWidget *parent, bool *ok = nullptr);

private:
    QLabel* friendlyNameLabel;
    QLineEdit* friendlyName;
    QLabel* channelLabel;
    QLineEdit* channelName;
    QLabel* trackNameLabel;
    QLineEdit* trackName;
    QComboBox* type;
    QComboBox* dimension;
    QRadioButton* positiveModifier;
    QRadioButton* negativeModifier;
    QFrame* modifierFrame;

    void onTypeChanged(QString value);
};

#endif // ADDCHANNELDIALOG_H
