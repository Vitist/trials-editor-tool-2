#include "trialseditortool.h"
#include "ui_trialseditortool.h"
#include "track.h"
#include "config.h"
#include <QFileDialog>
#include <QDirIterator>
#include <QStandardPaths>
#include <QDebug>

TrialsEditorTool::TrialsEditorTool(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TrialsEditorTool)
{
    ui->setupUi(this);

    //config.setConfig("4bf00161bc0597676a50b5322a159cd5", "deadbabe");
    //config.save();
    if(config.load()) {
        qDebug() << "Config loaded";
    }
    else {
        qDebug() << "Can't load config";
    }

    // Find SavedGames directory path
    const QString documentsDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if(!documentsDirPath.isEmpty()) {
        saveDir = QDir(documentsDirPath + "/TrialsFusion/SavedGames");
        scanDir(saveDir);
    }
    else {
        // TODO: Ask user to find correct folder
    }
}

TrialsEditorTool::~TrialsEditorTool()
{
    delete ui;
}

// Scan a directory for tracks
void TrialsEditorTool::scanDir(QDir dir)
{
    qDebug() << "Scanning: " << dir.path();

    // Tracks end with an "-index"
    QFileInfoList trackDirectories = dir.entryInfoList(QStringList() << "*-0000000*", QDir::Dirs | QDir::NoDotAndDotDot);

    // Editor track index is 0
    QString editorTrackIndex = "0000000000000";
    // Separate favorite tracks and editor tracks
    foreach(QFileInfo track, trackDirectories) {
        if(track.filePath().contains(editorTrackIndex)) {
            editorTracks.push_back(Track(track.filePath()));
        }
        else {
            favoriteTracks.push_back(Track(track.filePath()));
        }
    }

    // Add favorite tracks to the QListWidget
    foreach(Track track, favoriteTracks) {
        //qDebug() << "Adding favorite: " << track.getName();
        ui->availableTracksList->addItem(track.getName());
    }

    qDebug() << "\nScan complete\n";
}

void TrialsEditorTool::on_browseButton_clicked()
{
    qDebug() << "browseButton click";
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    QList<QUrl> folderNames;
    if (dialog.exec()) {
        folderNames = dialog.selectedUrls();
    }
    qDebug() << "selected folders count: " << folderNames.size();
    qDebug() << "selected folder name: " << folderNames.front().toString();
    // C:\Users\Teemu\Documents\TrialsFusion\SavedGames
}

void TrialsEditorTool::on_addTrackButton_clicked()
{
    // Get selected favorite tracks
    QList<QListWidgetItem*> selectedItems = ui->availableTracksList->selectedItems();
    foreach(QListWidgetItem* item, selectedItems) {
        qDebug() << "Adding track: " << item->text();
        // Find the selected track from favorites and add it to export
        foreach(Track track, favoriteTracks) {
            if (track.getName() == item->text()) {
                exportTracks.append(track);
                ui->exportTracksList->addItem(item->text());
                ui->exportTrackButton->setEnabled(true);
            }
        }
    }
}

void TrialsEditorTool::on_removeTrackButton_clicked()
{
    // Get selected export tracks
    QList<QListWidgetItem*> selectedItems = ui->exportTracksList->selectedItems();
    foreach(QListWidgetItem* item, selectedItems) {
        qDebug() << "Removing track: " << item->text();
        // Find the selected track from export and remove it
        foreach(Track track, exportTracks) {
            if(track.getName() == item->text()) {
                exportTracks.removeAll(track);
            }
        }
        // Remove track from export tracks
        delete item;
    }
    if(exportTracks.isEmpty()) {
        ui->exportTrackButton->setEnabled(false);
    }
}


void TrialsEditorTool::on_exportTrackButton_clicked()
{
    foreach(Track track, exportTracks) {
        qDebug() << "Exporting track: " + track.getName();
        track.exportToEditor(config.getConfig().value("userId"), config.getConfig().value("platform"), saveDir);
    }
    exportTracks.clear();
    ui->exportTracksList->clear();
    ui->exportTrackButton->setEnabled(false);
}
