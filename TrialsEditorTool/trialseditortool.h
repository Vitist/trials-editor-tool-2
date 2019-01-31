#ifndef TRIALSEDITORTOOL_H
#define TRIALSEDITORTOOL_H

#include "track.h"
#include "config.h"
#include <QMainWindow>
#include <QDir>
#include <QProgressBar>

namespace Ui {
class TrialsEditorTool;
}

class TrialsEditorTool : public QMainWindow
{
    Q_OBJECT

public:
    explicit TrialsEditorTool(QWidget *parent = nullptr);
    ~TrialsEditorTool();
    bool initialize();
    void scanSaveGamesFavorite();
    void scanSaveGamesEditor();
    void scanBrowseDir(QDir dir);
    void setupAvailableList();

private slots:
    void on_browseButton_clicked();
    void on_addTrackButton_clicked();
    void on_removeTrackButton_clicked();
    void on_exportTrackButton_clicked();

    void on_favoritesButton_clicked();

private:
    Ui::TrialsEditorTool *ui;
    QProgressBar *statusProgress;
    Config config;
    QDir saveDir;
    QList<Track> editorTracks;
    QList<Track> availableTracks;
    QList<Track> exportTracks;
};

#endif // TRIALSEDITORTOOL_H
