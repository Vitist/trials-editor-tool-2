#ifndef TRIALSEDITORTOOL_H
#define TRIALSEDITORTOOL_H

#include "track.h"
#include "config.h"
#include <memory>
#include <QMainWindow>
#include <QDir>
#include <QProgressBar>
#include <QScopedPointer>

namespace Ui {
class TrialsEditorTool;
}

class TrialsEditorTool : public QMainWindow
{
    Q_OBJECT

public:
    explicit TrialsEditorTool(QWidget *parent = nullptr);
    ~TrialsEditorTool();
    bool initialize(QString path);
    void scanFusionDownloads();
    void scanFusionEditor();
    void scanFusionTracks();
    void scanRisingDownloads();
    void scanRisingEditor();
    void scanDownloads();
    void scanEditor();
    void scanBrowseDir(QDir dir);
    void setupAvailableList();
    bool initWithRising(QDir dir);
    bool initWithFusion(QDir dir);

private slots:
    void on_browseButton_clicked();
    void on_addTrackButton_clicked();
    void on_removeTrackButton_clicked();
    void on_exportTrackButton_clicked();
    void on_favoritesButton_clicked();
    void on_selectDirLineEdit_editingFinished();
    void on_risingRadioButton_toggled(bool checked);
    void on_fusionRadioButton_toggled(bool checked);

    void on_fusionToRisingRadioButton_toggled(bool checked);

private:
    Ui::TrialsEditorTool *ui;
    QProgressBar *statusProgress;
    Config config;
    QDir risingSaveDir;
    QDir fusionSaveDir;
    QList<std::shared_ptr<Track>> editorTracks;
    QList<std::shared_ptr<Track>> availableTracks;
    QList<std::shared_ptr<Track>> exportTracks;
};

#endif // TRIALSEDITORTOOL_H
