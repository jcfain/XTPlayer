#include "addchanneldialog.h"

AddChannelDialog::AddChannelDialog(QWidget* parent) : QDialog(parent)
{
    friendlyNameLabel = new QLabel(this);
    friendlyNameLabel->setText("Friendly name");
    friendlyName = new QLineEdit(this);
    friendlyName->setText("New Channel");
    channelLabel = new QLabel(this);
    channelLabel->setText("Channel name");
    channelName = new QLineEdit(this);
    channelName->setText("L0");
    positiveModifier = new QRadioButton(this);
    positiveModifier->setText("+");
    negativeModifier = new QRadioButton(this);
    negativeModifier->setText("-");
    QLabel* typeLabel = new QLabel(this);
    typeLabel->setText("Type");
    type = new QComboBox(this);
    type->addItems(QStringList(AxisTypes.keys()));
    type->setCurrentText("None");
    QLabel* dimensionLabel = new QLabel(this);
    dimensionLabel->setText("Dimension");
    dimension = new QComboBox(this);
    dimension->addItems(QStringList(AxisDimensions.keys()));
    dimension->setCurrentText("None");
    connect(type, &QComboBox::currentTextChanged, this, &AddChannelDialog::onTypeChanged);

    modifierFrame = new QFrame(this);
    QGridLayout* modifierLayout = new QGridLayout(modifierFrame);
    modifierLayout->addWidget(positiveModifier, 0, 0, 1, 1);
    modifierLayout->addWidget(negativeModifier, 0, 1, 1, 1);
    modifierFrame->setLayout(modifierLayout);
    QGridLayout* layout = new QGridLayout(this);
    layout->addWidget(friendlyNameLabel, 0, 0, 1, 1);
    layout->addWidget(friendlyName, 0, 1, 1, 1);
    layout->addWidget(channelLabel, 1, 0, 1, 1);
    layout->addWidget(channelName, 1, 1, 1, 1);
    layout->addWidget(typeLabel, 3, 0, 1, 1);
    layout->addWidget(type, 3, 1, 1, 1);
    layout->addWidget(dimensionLabel, 4, 0, 1, 1);
    layout->addWidget(dimension, 4, 1, 1, 1);
    layout->addWidget(modifierFrame, 5, 0, 1, 2);
    modifierFrame->hide();

    QDialogButtonBox *buttonBox = new QDialogButtonBox
            ( QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
              Qt::Horizontal, this );
    layout->addWidget(buttonBox, 6, 0, 1, 2);

    bool conn = connect(buttonBox, &QDialogButtonBox::accepted,
                   this, &AddChannelDialog::accept);
    Q_ASSERT(conn);
    conn = connect(buttonBox, &QDialogButtonBox::rejected,
                   this, &AddChannelDialog::reject);
    Q_ASSERT(conn);
    setLayout(layout);
}

ChannelModel AddChannelDialog::getNewChannel(QWidget *parent, bool *ok)
{
    AddChannelDialog *dialog = new AddChannelDialog(parent);

    ChannelModel channelModel;

    const int ret = dialog->exec();
    if (ok)
        *ok = !!ret;

    if (*ok)
    {
        bool isValid = true;
        channelModel.Channel = dialog->channelName->text();
        channelModel.FriendlyName = dialog->friendlyName->text();
        QString modifier = "";
        if(AxisTypes[dialog->type->currentText()] == AxisType::HalfRange && dialog->positiveModifier->isChecked())
            modifier = "+";
        else if (AxisTypes[dialog->type->currentText()] == AxisType::HalfRange && dialog->negativeModifier->isChecked())
            modifier = "-";
        else if (AxisTypes[dialog->type->currentText()] == AxisType::HalfRange)
        {
            isValid = false;
            LogHandler::Dialog("Modifier (+/-) required for half range types!", XLogLevel::Critical);
        }
        channelModel.AxisName = dialog->channelName->text() + modifier;
        channelModel.Type = AxisTypes[dialog->type->currentText()];
        channelModel.Dimension = AxisDimensions[dialog->dimension->currentText()];
        channelModel.Min = 1;
        channelModel.Mid = 500;
        channelModel.Max = 999;
        channelModel.UserMin = 1;
        channelModel.UserMid = 500;
        channelModel.UserMax = 999;
        channelModel.TrackName = "";
        if(SettingsHandler::getAvailableAxis()->contains(channelModel.AxisName))
        {
            isValid = false;
            LogHandler::Dialog(channelModel.AxisName + " already exists!", XLogLevel::Critical);
        }
        if(channelModel.AxisName.isEmpty())
        {
            isValid = false;
            LogHandler::Dialog("Axis name is required!", XLogLevel::Critical);
        }
        if (!isValid)
            *ok = false;
    }

    dialog->deleteLater();

    return channelModel;
}

void AddChannelDialog::onTypeChanged(QString value)
{
    if(AxisTypes[value] == AxisType::HalfRange)
        modifierFrame->show();
    else
    {
        modifierFrame->hide();

        positiveModifier->setChecked(false);
        negativeModifier->setChecked(false);
    }
}
