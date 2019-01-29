#include "configdialog.h"
#include "ui_configdialog.h"
#include <QDebug>

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::on_buttonBox_accepted()
{
    qDebug() << "Accept";
}

void ConfigDialog::on_buttonBox_rejected()
{
    qDebug() << "Reject";
}
