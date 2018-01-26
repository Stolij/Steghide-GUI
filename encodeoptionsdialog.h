#ifndef ENCODEOPTIONSDIALOG_H
#define ENCODEOPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class EncodeOptionsDialog;
}

class EncodeOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EncodeOptionsDialog(QWidget *parent = 0);
    ~EncodeOptionsDialog();

    QString EncodeOptionsDialog::getPassword();
    int EncodeOptionsDialog::getEncryptionAlgorithm();
    int EncodeOptionsDialog::getEncryptionMode();

private:
    Ui::EncodeOptionsDialog *ui;
};

#endif // ENCODEOPTIONSDIALOG_H
