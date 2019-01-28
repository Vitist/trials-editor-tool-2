#ifndef TRIALSEDITORTOOL_H
#define TRIALSEDITORTOOL_H

#include <QMainWindow>
#include <QDir>

namespace Ui {
class TrialsEditorTool;
}

class TrialsEditorTool : public QMainWindow
{
    Q_OBJECT

public:
    explicit TrialsEditorTool(QWidget *parent = nullptr);
    ~TrialsEditorTool();
    void scanDir(QDir dir);

private slots:
    void on_browseButton_clicked();

    void on_removeTrackButton_clicked();

    void on_addTrackButton_clicked();

private:
    Ui::TrialsEditorTool *ui;
    QFileInfoList favoriteTracks;
    QFileInfoList editorTracks;
};

#endif // TRIALSEDITORTOOL_H
