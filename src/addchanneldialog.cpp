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
    trackNameLabel = new QLabel(this);
    trackNameLabel->setText("Track name");
    trackName = new QLineEdit(this);
    trackName->setText("");
    positiveModifier = new QRadioButton(this);
    positiveModifier->setText("+");
    negativeModifier = new QRadioButton(this);
    negativeModifier->setText("-");
    QLabel* typeLabel = new QLabel(this);
    typeLabel->setText("Type");
    type = new QComboBox(this);
    type->addItems(QStringList(ChannelTypes.keys()));
    type->setCurrentText("None");
    QLabel* dimensionLabel = new QLabel(this);
    dimensionLabel->setText("Dimension");
    dimension = new QComboBox(this);
    dimension->addItems(QStringList(ChannelDimensions.keys()));
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
    layout->addWidget(trackNameLabel, 2, 0, 1, 1);
    layout->addWidget(trackName, 2, 1, 1, 1);
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

ChannelModel33 AddChannelDialog::getNewChannel(QWidget *parent, bool *ok)
{
    AddChannelDialog *dialog = new AddChannelDialog(parent);

    ChannelModel33 channelModel;

    const int ret = dialog->exec();
    if (ok)
        *ok = !!ret;

    if (*ok)
    {
        bool isValid = true;
        channelModel.Channel = dialog->channelName->text();
        channelModel.FriendlyName = dialog->friendlyName->text();
        QString modifier = "";
        if(ChannelTypes[dialog->type->currentText()] == ChannelType::HalfOscillate && dialog->positiveModifier->isChecked())
            modifier = "+";
        else if (ChannelTypes[dialog->type->currentText()] == ChannelType::HalfOscillate && dialog->negativeModifier->isChecked())
            modifier = "-";
        else if (ChannelTypes[dialog->type->currentText()] == ChannelType::HalfOscillate)
        {
            isValid = false;
            DialogHandler::MessageBox(parent, "Modifier (+/-) required for half range types!", XLogLevel::Critical);
        }
        channelModel.ChannelName = dialog->channelName->text() + modifier;
        channelModel.Type = ChannelTypes[dialog->type->currentText()];
        channelModel.Dimension = ChannelDimensions[dialog->dimension->currentText()];
        channelModel.Min = 0;
        channelModel.Mid = channelModel.Type == ChannelType::Ramp ? 0 : TCodeChannelLookup::getSelectedTCodeVersion() == TCodeVersion::v2 ? 500 : 5000;
        channelModel.Max = TCodeChannelLookup::getSelectedTCodeVersion() == TCodeVersion::v2 ? 999: 9999;
        channelModel.UserMin = 0;
        channelModel.UserMid = channelModel.Type == ChannelType::Ramp ? 0 : TCodeChannelLookup::getSelectedTCodeVersion() == TCodeVersion::v2 ? 500 : 5000;
        channelModel.UserMax = TCodeChannelLookup::getSelectedTCodeVersion() == TCodeVersion::v2 ? 999 : 9999;
        channelModel.DamperEnabled = false;
        channelModel.DamperValue = 0.2f;
        channelModel.FunscriptInverted = false;
        channelModel.GamepadInverted = false;
        channelModel.MultiplierEnabled = false;
        channelModel.LinkToRelatedMFS = false;
        channelModel.RelatedChannel = TCodeChannelLookup::Stroke();
        channelModel.trackName = dialog->trackName->text();
        if(TCodeChannelLookup::hasChannel(channelModel.ChannelName))
        {
            isValid = false;
            DialogHandler::MessageBox(parent, channelModel.ChannelName + " already exists!", XLogLevel::Critical);
        }
        if(channelModel.ChannelName.isEmpty())
        {
            isValid = false;
            DialogHandler::MessageBox(parent, "Axis name is required!", XLogLevel::Critical);
        }
        if (!isValid)
            *ok = false;
    }

    dialog->deleteLater();

    return channelModel;
}

void AddChannelDialog::onTypeChanged(QString value)
{
    if(ChannelTypes[value] == ChannelType::HalfOscillate)
        modifierFrame->show();
    else
    {
        modifierFrame->hide();

        positiveModifier->setChecked(false);
        negativeModifier->setChecked(false);
    }
}
