#include "trialseditortool.h"
#include "ui_trialseditortool.h"
#include "track.h"
#include "config.h"
#include "configdialog.h"
#include "trackoverwritedialog.h"
#include <QFileDialog>
#include <QDirIterator>
#include <QStandardPaths>
#include <QDebug>

TrialsEditorTool::TrialsEditorTool(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TrialsEditorTool)
{
    ui->setupUi(this);

    // Find SavedGames directory path
    const QString documentsDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if(!documentsDirPath.isEmpty()) {
        saveDir = QDir(documentsDirPath + "/TrialsFusion/SavedGames");
    } else {
        // TODO: Ask user to find correct folder
    }

    while(!config.initialize(saveDir)) {
        qDebug() << "Can't initialize config";
        ConfigDialog dialog;
        dialog.setModal(true);
        if(dialog.exec()) {
            //qDebug() << dialog.test();
        } else {
            //qDebug() << dialog.test();
        }
    }
    /*else {
        qDebug() << "Config initialized";
    }*/

    scanDir(saveDir);
    setupFavoriteList();
}

TrialsEditorTool::~TrialsEditorTool()
{
    delete ui;
}

// Scan a directory for tracks
void TrialsEditorTool::scanDir(QDir dir)
{
    qDebug() << "Scanning: " << dir.path();

    editorTracks.clear();
    favoriteTracks.clear();

    // Tracks end with an "-index"
    QFileInfoList trackDirectories = dir.entryInfoList(QStringList() << "*-0000000*", QDir::Dirs | QDir::NoDotAndDotDot);

    // Editor track index is 0
    QString editorTrackIndex = "0000000000000";
    // Separate favorite tracks and editor tracks
    foreach(QFileInfo track, trackDirectories) {
        if(track.filePath().contains(editorTrackIndex)) {
            editorTracks.append(Track(track.filePath()));
        } else {
            favoriteTracks.append(Track(track.filePath()));
        }
    }

    qDebug() << "\nScan complete\n";
}

void TrialsEditorTool::setupFavoriteList()
{
    // Add favorite tracks to the QListWidget
    foreach(Track track, favoriteTracks) {
        //qDebug() << "Adding favorite: " << track.getName();
        ui->availableTracksList->addItem(track.getName());
    }
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

        bool trackInExport = false;
        foreach(Track track, exportTracks) {
            if(track.getName() == item->text()) {
                trackInExport = true;
                qDebug() << "Track is already added to export";
            }
        }

        // Find the selected track from favorites and add it to export
        if(!trackInExport) {
            foreach(Track track, favoriteTracks) {
                if (track.getName() == item->text()) {
                    exportTracks.append(track);
                    ui->exportTracksList->addItem(item->text());
                    ui->exportTrackButton->setEnabled(true);
                }
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
    foreach(Track exportTrack, exportTracks) {
        qDebug() << "Exporting track: " + exportTrack.getName();
        bool allowExport = true;
        // Check if track is already available in the editor
        foreach(Track editorTrack, editorTracks) {
            if(editorTrack.getName() == exportTrack.getName()) {
                qDebug() << "Track is already added to editor";

                // Setup dialog for asking the user if they want to overwrite track
                TrackOverwriteDialog dialog;
                dialog.setModal(true);
                dialog.setTrackName(editorTrack.getName());

                // Ask the user if they want to overwrite track
                // Remove track if allowed
                if(dialog.exec()) {
                    bool removed = editorTrack.removeFromDisk();
                    if(removed) {
                        qDebug() << "Track removed";
                    }
                } else {
                    allowExport = false;
                }
            }
        }

        if(allowExport) {
            exportTrack.exportToEditor(config.getConfig().value("userId"), saveDir);
        }
    }
    scanDir(saveDir);
    exportTracks.clear();
    ui->exportTracksList->clear();
    ui->exportTrackButton->setEnabled(false);
}
