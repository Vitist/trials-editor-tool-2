#ifndef TRIALSEDITORTOOL_H
#define TRIALSEDITORTOOL_H

#include "track.h"
#include "config.h"
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
    void on_addTrackButton_clicked();
    void on_removeTrackButton_clicked();
    void on_exportTrackButton_clicked();

private:
    Ui::TrialsEditorTool *ui;
    Config config;
    QDir saveDir;
    QList<Track> favoriteTracks;
    QList<Track> editorTracks;
    QList<Track> exportTracks;
};

#endif // TRIALSEDITORTOOL_H
