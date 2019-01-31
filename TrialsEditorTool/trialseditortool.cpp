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
        ui->selectDirLineEdit->setText(saveDir.path());
    } else {
        // TODO: Ask user to find correct folder
    }

    while(!config.initialize(saveDir)) {
        qDebug() << "Can't initialize config";
        ConfigDialog dialog;
        dialog.setModal(true);
        dialog.exec();
    }

    scanSaveGamesFavorite();
    setupAvailableList();
}

TrialsEditorTool::~TrialsEditorTool()
{
    delete ui;
}

void TrialsEditorTool::scanSaveGamesFavorite()
{
    qDebug() << "\nScanning: " << saveDir.path();

    availableTracks.clear();

    // Tracks end with an "-index"
    QFileInfoList trackDirectories = saveDir.entryInfoList(QStringList() << "*-0000000*", QDir::Dirs | QDir::NoDotAndDotDot);

    // Editor track index is 0
    QString editorTrackIndex = "0000000000000";
    // Separate favorite tracks and editor tracks
    foreach(QFileInfo track, trackDirectories) {
        if(!track.filePath().contains(editorTrackIndex)) {
            qDebug() << "Adding favorite: " << track.filePath();
            availableTracks.append(Track(track.filePath()));
        }
    }

    qDebug() << "Scan complete\n";
}

void TrialsEditorTool::scanSaveGamesEditor()
{
    qDebug() << "\nScanning: " << saveDir.path();

    editorTracks.clear();

    // Editor tracks end with "-0000000000000"
    QFileInfoList trackDirectories = saveDir.entryInfoList(QStringList() << "*-0000000000000", QDir::Dirs | QDir::NoDotAndDotDot);

    foreach(QFileInfo track, trackDirectories) {
        qDebug() << "Adding editor: " << track.filePath();
        editorTracks.append(Track(track.filePath()));
    }

    qDebug() << "Scan complete\n";
}

void TrialsEditorTool::scanBrowseDir(QDir dir)
{
    // Check if the user selected SavedGames directory
    if(dir.path() == saveDir.path()) {
        scanSaveGamesFavorite();
    } else {
        qDebug() << "\nScanning: " << dir.path();

        availableTracks.clear();

        if(dir.dirName().contains("-0000000")) {
            qDebug() << "Selected directory is a track";
            availableTracks.append(Track(dir.path()));
        } else {
            qDebug() << "Searching tracks from selected directory";
            // Tracks end with an "-index"
            QFileInfoList trackDirectories = dir.entryInfoList(QStringList() << "*-0000000*", QDir::Dirs | QDir::NoDotAndDotDot);

            // Separate favorite tracks and editor tracks
            foreach(QFileInfo track, trackDirectories) {
                qDebug() << "Adding browse: " << track.filePath();
                availableTracks.append(Track(track.filePath()));
            }
        }

        qDebug() << "Scan complete\n";
    }
}

void TrialsEditorTool::setupAvailableList()
{
    ui->availableTracksList->clear();
    // Add favorite tracks to the QListWidget
    foreach(Track track, availableTracks) {
        qDebug() << "Adding available: " << track.getName();
        ui->availableTracksList->addItem(track.getName());
    }
}

void TrialsEditorTool::on_browseButton_clicked()
{
    // Create a dialog for selecting a directory
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);

    // Open the dialog
    if (dialog.exec()) {
        // Get selected directory paths
        QList<QUrl> dirPaths = dialog.selectedUrls();
        qDebug() << "Selected folders count: " << dirPaths.size();
        qDebug() << "Selected folder name: " << dirPaths.first().toLocalFile();

        // Only one directory can be selected at a time
        QDir browseDir(dirPaths.first().toLocalFile());
        ui->selectDirLineEdit->setText(browseDir.path());
        scanBrowseDir(browseDir);
        setupAvailableList();
    }
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
            foreach(Track track, availableTracks) {
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
    scanSaveGamesEditor();
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
    exportTracks.clear();
    ui->exportTracksList->clear();
    ui->exportTrackButton->setEnabled(false);
}

void TrialsEditorTool::on_favoritesButton_clicked()
{
    ui->selectDirLineEdit->setText(saveDir.path());
    scanSaveGamesFavorite();
    setupAvailableList();
}
