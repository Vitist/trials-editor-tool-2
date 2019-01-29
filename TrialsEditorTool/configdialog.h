#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include "config.h"
#include <QDialog>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::ConfigDialog *ui;
};

#endif // CONFIGDIALOG_H
