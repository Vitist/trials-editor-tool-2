#include "trialseditortool.h"
#include "ui_trialseditortool.h"
#include "fusiontrack.h"
#include "risingtrack.h"
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

    // Setup progress bar for track scanning and exporting
    statusProgress = new QProgressBar(this);
    statusProgress->setFixedHeight(10);
    statusProgress->setVisible(false);
    ui->statusBar->addPermanentWidget(statusProgress);
}

TrialsEditorTool::~TrialsEditorTool()
{
    delete ui;
    delete statusProgress;
}

bool TrialsEditorTool::initialize(QString path)
{
    // Find SavedGames directory path
    const QString documentsDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    // Check for Trials games in the Documents directory
    QDir risingSaveDir = QDir(documentsDirPath + "/Trials Rising/SavedGames");
    QDir fusionSaveDir = QDir(documentsDirPath + "/TrialsFusion/SavedGames");
    if (risingSaveDir.exists() && fusionSaveDir.exists()) {
        saveDir = risingSaveDir;
        ui->risingRadioButton->setEnabled(true);
        ui->risingRadioButton->setChecked(true);
        ui->fusionRadioButton->setEnabled(true);
        if(!config.load()) {
            if(!initWithRising(risingSaveDir)) {
                initWithFusion(fusionSaveDir);
            }
        }
    } else if(risingSaveDir.exists()) {
        saveDir = risingSaveDir;
        ui->risingRadioButton->setEnabled(true);
        ui->risingRadioButton->setChecked(true);
        if(!config.load()) {
            initWithRising(risingSaveDir);
        }
    } else if(fusionSaveDir.exists()) {
        saveDir = fusionSaveDir;
        ui->fusionRadioButton->setEnabled(true);
        ui->fusionRadioButton->setChecked(true);
        if(!config.load()) {
            initWithFusion(fusionSaveDir);
        }
    } else {
        // TODO: ask user to locate save dir
        qDebug() << "No Trials SavedGames dir";
        ui->statusBar->showMessage("No Trials SavedGames folder found");
    }

    // Check if a directory was given as a command line argument
    if(path.count() == 0) {
        scanDownloads();
    } else {
        scanBrowseDir(QDir(path));
        ui->selectDirLineEdit->setText(path);
    }
    setupAvailableList();
    // Return false to close the program if init fails
    return true;
}

void TrialsEditorTool::scanFusionDownloads()
{
    qDebug() << "\nScanning: " << saveDir.path();
    ui->statusBar->clearMessage();

    availableTracks.clear();

    // Tracks end with an "-index"
    QFileInfoList trackDirectories = saveDir.entryInfoList(QStringList() << "*-0000000*", QDir::Dirs | QDir::NoDotAndDotDot);
    // Editor track index is 0
    QString editorTrackIndex = "0000000000000";

    int dirProcessedCount = 0;
    statusProgress->setMaximum(trackDirectories.count());
    statusProgress->setVisible(true);
    foreach(QFileInfo track, trackDirectories) {
        if(!track.filePath().contains(editorTrackIndex)) {
            qDebug() << "Adding favorite: " << track.filePath();
            availableTracks.append(std::shared_ptr<Track>(new FusionTrack(track.filePath())));
        }
        statusProgress->setValue(++dirProcessedCount);
    }

    statusProgress->setVisible(false);
    if(availableTracks.count() == 1) {
        ui->statusBar->showMessage("Found " + QString::number(availableTracks.count()) + " track");
    } else {
        ui->statusBar->showMessage("Found " + QString::number(availableTracks.count()) + " tracks");
    }

    qDebug() << "Scan complete\n";
}

void TrialsEditorTool::scanFusionEditor()
{
    qDebug() << "\nScanning: " << saveDir.path();

    editorTracks.clear();

    // Editor tracks end with "-0000000000000"
    QFileInfoList trackDirectories = saveDir.entryInfoList(QStringList() << "*-0000000000000", QDir::Dirs | QDir::NoDotAndDotDot);

    foreach(QFileInfo track, trackDirectories) {
        qDebug() << "Adding editor: " << track.filePath();
        editorTracks.append(std::shared_ptr<Track>(new FusionTrack(track.filePath())));
    }

    qDebug() << "Scan complete\n";
}

void TrialsEditorTool::scanRisingDownloads()
{
    QDir downloadsDir = saveDir;
    qDebug() << "\nScanning: " << downloadsDir.path();
    ui->statusBar->clearMessage();

    availableTracks.clear();

    // Find the user directory inside SavedGames
    QFileInfoList userDirectories = downloadsDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    // Change directory to downloaded tracks directory inside the user directory
    if (!userDirectories.empty()) {
        downloadsDir.cd(userDirectories.first().fileName() + "/CacheStorage/usertracks");
        qDebug() << downloadsDir.path();
        QFileInfoList trackDirectories = downloadsDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        int dirProcessedCount = 0;
        statusProgress->setMaximum(trackDirectories.count());
        statusProgress->setVisible(true);
        foreach(QFileInfo track, trackDirectories) {
            qDebug() << "Adding favorite: " << track.fileName();
            availableTracks.append(std::shared_ptr<Track>(new RisingTrack(track.filePath())));
            statusProgress->setValue(++dirProcessedCount);
        }
    } else {
        qDebug() << "No Rising user directory found";
    }

    statusProgress->setVisible(false);
    if(availableTracks.count() == 1) {
        ui->statusBar->showMessage("Found " + QString::number(availableTracks.count()) + " track");
    } else {
        ui->statusBar->showMessage("Found " + QString::number(availableTracks.count()) + " tracks");
    }

    qDebug() << "Scan complete\n";
}

void TrialsEditorTool::scanRisingEditor()
{
    QDir editorDir = saveDir;
    qDebug() << "\nScanning: " << editorDir.path();

    editorTracks.clear();

    // Find the user directory inside SavedGames
    QFileInfoList userDirectories = editorDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    // Saved objects end with .o
    QString objectEnd = ".o";

    // Change directory to downloaded tracks directory inside the user directory
    if (!userDirectories.empty()) {
        editorDir.cd(userDirectories.first().fileName() + "/usertracks");
        qDebug() << editorDir.path();
        QFileInfoList trackDirectories = editorDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

        foreach(QFileInfo track, trackDirectories) {
            if(!track.filePath().contains(objectEnd)) {
                qDebug() << "Adding editor: " << track.fileName();
                editorTracks.append(std::shared_ptr<Track>(new RisingTrack(track.filePath())));
            }
        }
    } else {
        qDebug() << "No Rising user directory found";
    }

    qDebug() << "Scan complete\n";
}

void TrialsEditorTool::scanDownloads()
{
    if(ui->risingRadioButton->isChecked()) {
        scanRisingDownloads();
    } else {
        scanFusionDownloads();
    }
}

void TrialsEditorTool::scanEditor()
{
    if(ui->risingRadioButton->isChecked()) {
        scanRisingEditor();
    } else {
        scanFusionEditor();
    }
}

void TrialsEditorTool::scanBrowseDir(QDir dir)
{
    // Check if the user selected SavedGames directory
    if(dir.path() == saveDir.path()) {
        scanDownloads();
    } else {
        qDebug() << "\nScanning: " << dir.path();
        ui->statusBar->clearMessage();

        availableTracks.clear();

        if(dir.dirName().contains("-0000000")) {
            qDebug() << "Selected directory is a track";
            availableTracks.append(std::shared_ptr<Track>(new FusionTrack(dir.path())));
        } else {
            qDebug() << "Searching tracks from selected directory";
            // Tracks end with an "-index"
            QFileInfoList trackDirectories = dir.entryInfoList(QStringList() << "*-0000000*", QDir::Dirs | QDir::NoDotAndDotDot);

            int dirProcessedCount = 0;
            statusProgress->setMaximum(trackDirectories.count());
            statusProgress->setVisible(true);
            foreach(QFileInfo track, trackDirectories) {
                qDebug() << "Adding browse: " << track.filePath();
                availableTracks.append(std::shared_ptr<Track>(new FusionTrack(track.filePath())));
                statusProgress->setValue(++dirProcessedCount);
            }
        }

        statusProgress->setVisible(false);
        if(availableTracks.count() == 1) {
            ui->statusBar->showMessage("Found " + QString::number(availableTracks.count()) + " track");
        } else {
            ui->statusBar->showMessage("Found " + QString::number(availableTracks.count()) + " tracks");
        }

        qDebug() << "Scan complete\n";
    }
}

void TrialsEditorTool::setupAvailableList()
{
    ui->availableTracksList->clear();
    // Add favorite tracks to the QListWidget
    foreach(std::shared_ptr<Track> track, availableTracks) {
        //qDebug() << "Adding available: " << track.getName();
        ui->availableTracksList->addItem(track->getName());
    }
}

bool TrialsEditorTool::initWithRising(QDir dir)
{
    if(!config.initialize(dir, Config::Rising)) {
        qDebug() << "Config init with Rising failed";
        ui->statusBar->showMessage("Config initialization failed: no uplay id found.");
        return false;
    }
    return true;
}

bool TrialsEditorTool::initWithFusion(QDir dir)
{
    while(!config.initialize(dir, Config::Fusion)) {
        qDebug() << "Config init with Fusion failed";
        ConfigDialog dialog;
        dialog.setModal(true);
        if(!dialog.exec()) {
            ui->statusBar->showMessage("Config initialization failed: no uplay id found.");
            return false;
        }
    }
    return true;
}

void TrialsEditorTool::on_browseButton_clicked()
{
    // TODO:
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
        foreach(std::shared_ptr<Track> track, exportTracks) {
            if(track->getName() == item->text()) {
                trackInExport = true;
                qDebug() << "Track is already added to export";
            }
        }

        // Find the selected track from favorites and add it to export
        if(!trackInExport) {
            foreach(std::shared_ptr<Track> track, availableTracks) {
                if (track->getName() == item->text()) {
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
        foreach(std::shared_ptr<Track> track, exportTracks) {
            if(track->getName() == item->text()) {
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
    scanEditor();
    int exportProcessedCount = 0;
    int exportedCount = 0;
    statusProgress->setMaximum(exportTracks.count());
    statusProgress->setVisible(true);
    foreach(std::shared_ptr<Track> exportTrack, exportTracks) {
        qDebug() << "Exporting track: " + exportTrack->getName();
        bool allowExport = true;
        // Check if track is already available in the editor
        foreach(std::shared_ptr<Track> editorTrack, editorTracks) {
            if(editorTrack->getName() == exportTrack->getName()) {
                qDebug() << "Track is already added to editor";

                // Setup dialog for asking the user if they want to overwrite track
                TrackOverwriteDialog dialog;
                dialog.setModal(true);
                dialog.setTrackName(editorTrack->getName());

                // Ask the user if they want to overwrite track
                // Remove track if allowed
                if(dialog.exec()) {
                    bool removed = editorTrack->removeFromDisk();
                    if(removed) {
                        qDebug() << "Track removed";
                    }
                } else {
                    allowExport = false;
                }
            }
        }

        if(allowExport) {
            exportTrack->exportToEditor(config.getConfig().value("userId"), saveDir);
            ++exportedCount;
        }
        statusProgress->setValue(++exportProcessedCount);
    }

    if(exportedCount == 1) {
        ui->statusBar->showMessage(QString::number(exportedCount) + " track exported to editor");
    } else {
        ui->statusBar->showMessage(QString::number(exportedCount) + " tracks exported to editor");
    }

    exportTracks.clear();
    ui->exportTracksList->clear();
    ui->exportTrackButton->setEnabled(false);

    statusProgress->setVisible(false);
}

void TrialsEditorTool::on_favoritesButton_clicked()
{
    ui->selectDirLineEdit->setText(saveDir.path());
    scanDownloads();
    setupAvailableList();
}

void TrialsEditorTool::on_selectDirLineEdit_editingFinished()
{
    scanBrowseDir(ui->selectDirLineEdit->text());
    setupAvailableList();
}
