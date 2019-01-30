#include "trackoverwritedialog.h"
#include "ui_trackoverwritedialog.h"
#include <QDebug>

TrackOverwriteDialog::TrackOverwriteDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TrackOverwriteDialog)
{
    ui->setupUi(this);
}

TrackOverwriteDialog::~TrackOverwriteDialog()
{
    delete ui;
}

void TrackOverwriteDialog::setTrackName(QString name)
{
    ui->label->setText("Track \"" + name + "\" already exists in the editor. Do you want to overwrite? This will remove the old track permanently.");
}

void TrackOverwriteDialog::on_buttonBox_accepted()
{
    qDebug() << "Overwrite accepted";
}

void TrackOverwriteDialog::on_buttonBox_rejected()
{
    qDebug() << "Overwrite rejected";
}
