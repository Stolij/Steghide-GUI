#include "encodeoptionsdialog.h"
#include "ui_encodeoptionsdialog.h"

EncodeOptionsDialog::EncodeOptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EncodeOptionsDialog)
{
    ui->setupUi(this);
}

EncodeOptionsDialog::~EncodeOptionsDialog()
{
    delete ui;
}

QString EncodeOptionsDialog::getPassword()
{
    return ui->Input_Password->text();
}

int EncodeOptionsDialog::getEncryptionAlgorithm()
{
    return ui->EncryptionAlg_Combobox->currentIndex();
}
int EncodeOptionsDialog::getEncryptionMode()
{
    return ui->EncryptionModes_combobox->currentIndex();
}
