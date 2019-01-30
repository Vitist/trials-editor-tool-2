#ifndef TRACKOVERWRITEDIALOG_H
#define TRACKOVERWRITEDIALOG_H

#include <QDialog>

namespace Ui {
class TrackOverwriteDialog;
}

class TrackOverwriteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TrackOverwriteDialog(QWidget *parent = nullptr);
    ~TrackOverwriteDialog();
    void setTrackName(QString name);

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::TrackOverwriteDialog *ui;
};

#endif // TRACKOVERWRITEDIALOG_H
