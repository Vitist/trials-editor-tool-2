#include "trialseditortool.h"
#include "ui_trialseditortool.h"
#include "fusiontrack.h"
#include "risingtrack.h"
#include "fusiontorisingtrack.h"
#include "config.h"
#include "configdialog.h"
#include "trackoverwritedialog.h"
#include <QFileDialog>
#include <QDirIterator>
#include <QStandardPaths>
#include <QDebug>
#include <FreeImage.h>

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

/*
 * Initialize the program by setting up config, finding Trials save directories and
 * checking if a track was given by drag & drop
 */
bool TrialsEditorTool::initialize(QString path)
{
    qDebug() << "Initializing";
    // FreeImage test
    qDebug() << FreeImage_GetCopyrightMessage();
    qDebug() << FreeImage_GetVersion();

    // Find SavedGames directory path
    const QString documentsDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    // Check for Trials game saves in the Documents directory
    fusionSaveDir = QDir(documentsDirPath + "/TrialsFusion/SavedGames");
    risingSaveDir = QDir(documentsDirPath + "/Trials Rising/SavedGames");
    // Find directories inside SavedGames
    QFileInfoList userDirectories = risingSaveDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    // Check if a user directory exists
    foreach(QFileInfo dir, userDirectories) {
        // User directory is always 36 charcters long
        if(dir.baseName().count() == 36) {
            // Change directory to the user directory
            risingSaveDir.cd(dir.fileName());
        }
    }
    // Set available games and initialize config
    if (risingSaveDir.exists() && fusionSaveDir.exists()) {
        ui->selectDirLineEdit->setText(risingSaveDir.path());
        ui->risingRadioButton->setEnabled(true);
        ui->risingRadioButton->setChecked(true);
        ui->fusionRadioButton->setEnabled(true);
        ui->fusionToRisingRadioButton->setEnabled(true);
        if(!config.load()) {
            if(!initWithRising(risingSaveDir)) {
                initWithFusion(fusionSaveDir);
            }
        }
    } else if(risingSaveDir.exists()) {
        ui->selectDirLineEdit->setText(risingSaveDir.path());
        ui->risingRadioButton->setEnabled(true);
        ui->risingRadioButton->setChecked(true);
        if(!config.load()) {
            initWithRising(risingSaveDir);
        }
    } else if(fusionSaveDir.exists()) {
        ui->selectDirLineEdit->setText(fusionSaveDir.path());
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
    if(path.size() == 0) {
        scanDownloads();
    } else {
        scanBrowseDir(QDir(path));
        ui->selectDirLineEdit->setText(path);
    }
    setupAvailableList();
    // Return false to close the program if init fails
    return true;
}

/*
 * Scan Fusion save directory for downloaded tracks
 */
void TrialsEditorTool::scanFusionDownloads()
{
    qDebug() << "\nScanning Fusion: " << fusionSaveDir.path();
    ui->statusBar->showMessage("Scanning Fusion tracks...");

    // Tracks end with an "-index"
    QFileInfoList trackDirectories = fusionSaveDir.entryInfoList(QStringList() << "*-0000000*", QDir::Dirs | QDir::NoDotAndDotDot);
    // Editor track index is 0
    QString editorTrackIndex = "0000000000000";

    // Setup progress bar
    int dirProcessedCount = 0;
    statusProgress->setMaximum(trackDirectories.count());
    statusProgress->setVisible(true);

    // Find all downloaded tracks
    foreach(QFileInfo track, trackDirectories) {
        if(!track.filePath().contains(editorTrackIndex)) {
            availableTracks.append(std::shared_ptr<Track>(new FusionTrack(track.filePath())));
        }
        statusProgress->setValue(++dirProcessedCount);
    }
    statusProgress->setVisible(false);
    qDebug() << "Fusion scan complete\n";
}

/*
 * Scan Fusion save directory for tracks created by the user
 */
void TrialsEditorTool::scanFusionEditor()
{
    qDebug() << "\nScanning: " << fusionSaveDir.path();
    editorTracks.clear();

    // Editor tracks end with "-0000000000000"
    QFileInfoList trackDirectories = fusionSaveDir.entryInfoList(QStringList() << "*-0000000000000", QDir::Dirs | QDir::NoDotAndDotDot);

    foreach(QFileInfo track, trackDirectories) {
        editorTracks.append(std::shared_ptr<Track>(new FusionTrack(track.filePath())));
    }

    qDebug() << "Scan complete\n";
}

/*
 * Scan Fusion save directory for all tracks
 */
void TrialsEditorTool::scanFusionTracks()
{
    qDebug() << "\nScanning Fusion: " << fusionSaveDir.path();
    ui->statusBar->showMessage("Scanning Fusion tracks...");

    // Tracks end with an "-index"
    QFileInfoList trackDirectories = fusionSaveDir.entryInfoList(QStringList() << "*-0000000*", QDir::Dirs | QDir::NoDotAndDotDot);

    // Setup progress bar
    int dirProcessedCount = 0;
    statusProgress->setMaximum(trackDirectories.count());
    statusProgress->setVisible(true);

    // Find all downloaded tracks
    foreach(QFileInfo track, trackDirectories) {
        availableTracks.append(std::shared_ptr<Track>(new FusionToRisingTrack(track.filePath())));
        statusProgress->setValue(++dirProcessedCount);
    }
    statusProgress->setVisible(false);
    qDebug() << "Fusion scan complete\n";
}

/*
 * Scan Rising save directory for downloaded tracks
 */
void TrialsEditorTool::scanRisingDownloads()
{
    QDir downloadsDir = risingSaveDir;
    qDebug() << "\nScanning Rising: " << downloadsDir.path();
    ui->statusBar->showMessage("Scanning Rising tracks");

    // Change directory to downloaded tracks directory inside the user directory
    downloadsDir.cd("CacheStorage/usertracks");
    // List all directories inside the download directory
    QFileInfoList trackDirectories = downloadsDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    // Setup progress bar
    int dirProcessedCount = 0;
    statusProgress->setMaximum(trackDirectories.count());
    statusProgress->setVisible(true);

    foreach(QFileInfo track, trackDirectories) {
        availableTracks.append(std::shared_ptr<Track>(new RisingTrack(track.filePath())));
        statusProgress->setValue(++dirProcessedCount);
    }

    statusProgress->setVisible(false);

    qDebug() << "Rising scan complete\n";
}

/*
 * Scan Rising save directory for tracks created by the user
 */
void TrialsEditorTool::scanRisingEditor()
{
    QDir editorDir = risingSaveDir;
    // Change directory to downloaded tracks directory inside the user directory
    editorDir.cd("usertracks");
    qDebug() << "\nScanning: " << editorDir.path();

    editorTracks.clear();

    // Saved objects end with .o
    QString objectEnd = ".o";

    // List all directories inside usertracks which contains tracks and favorite objects created by the user
    QFileInfoList trackDirectories = editorDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    //Find all tracks created by the user
    foreach(QFileInfo track, trackDirectories) {
        if(!track.filePath().contains(objectEnd)) {
            editorTracks.append(std::shared_ptr<Track>(new RisingTrack(track.filePath())));
        }
    }

    qDebug() << "Scan complete\n";
}

/*
 * Scan downloaded tracks from both games if the save directories exist
 */
void TrialsEditorTool::scanDownloads()
{
    availableTracks.clear();
    // Radio buttons are disabled during initialization if save directories
    // couldn't be found
    if(ui->risingRadioButton->isEnabled()) {
        scanRisingDownloads();
    }
    if(ui->fusionRadioButton->isEnabled()) {
        scanFusionDownloads();
    }
    if(ui->fusionToRisingRadioButton->isEnabled()) {
        scanFusionTracks();
    }
}

/*
 * Scan tracks created by the user from the currently selected game
 */
void TrialsEditorTool::scanEditor()
{
    if(ui->risingRadioButton->isChecked() || ui->fusionToRisingRadioButton->isChecked()) {
        scanRisingEditor();
    } else {
        scanFusionEditor();
    }
}

/*
 * Scan a directory that isn't a Trials save directory
 */
void TrialsEditorTool::scanBrowseDir(QDir dir)
{
    // Check if the user selected SavedGames directory
    if(dir.path() == fusionSaveDir.path() && dir.path() == risingSaveDir.path()) {
        scanDownloads();
    } else if(!dir.path().contains(".o")) {
        qDebug() << "\nScanning: " << dir.path();
        ui->statusBar->showMessage("Scanning folder...");

        availableTracks.clear();

        // List all the contents inside the directory
        QFileInfoList dirContents = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

        // Setup progress bar
        int dirProcessedCount = 0;
        statusProgress->setMaximum(dirContents.count());
        statusProgress->setVisible(true);

        // Setup counters for selecting the correct game
        int risingTrackCount = 0;
        int fusionTrackCount = 0;

        bool metadataFound = false;
        bool trackFound = false;
        bool displaynameFound = false;

        // Iterate through the selected directory contents
        foreach(QFileInfo dirContent, dirContents) {
            qDebug() << "dir " << dirContent.fileName();
            if(dirContent.fileName() == "metadata.mda") {
                metadataFound = true;
            } else if(dirContent.fileName() == "track.trk") {
                trackFound = true;
            } else if(dirContent.fileName() == "displayname") {
                displaynameFound = true;
            }
            statusProgress->setValue(++dirProcessedCount);
        }

        if(metadataFound && trackFound) {
            // Directory is a track directory
            if(displaynameFound) {
                // Fusion track
                availableTracks.append(std::shared_ptr<Track>(new FusionTrack(dir.path())));
                availableTracks.append(std::shared_ptr<Track>(new FusionToRisingTrack(dir.path())));
                ++fusionTrackCount;
            } else {
                // Rising track
                availableTracks.append(std::shared_ptr<Track>(new RisingTrack(dir.path())));
                ++risingTrackCount;
            }
        } else {
            // Setup progress bar
            dirProcessedCount = 0;
            // Directory isn't a track directory, check sub directories
            foreach(QFileInfo dirContent, dirContents) {
                // Only check files inside subdirectories
                QDir subDir(dirContent.filePath());
                QFileInfoList subDirContents = subDir.entryInfoList(QDir::Files);

                metadataFound = false;
                trackFound = false;
                displaynameFound = false;

                // Check if the subdirectory contains track files
                foreach(QFileInfo subDirContent, subDirContents) {
                    qDebug() << "sub dir " << subDirContent.fileName();
                    if(subDirContent.fileName() == "metadata.mda") {
                        metadataFound = true;
                    } else if(subDirContent.fileName() == "track.trk") {
                        trackFound = true;
                    } else if(subDirContent.fileName() == "displayname") {
                        displaynameFound = true;
                    }
                }

                if(metadataFound && trackFound) {
                    // Directory is a track directory
                    if(displaynameFound) {
                        // Fusion track
                        availableTracks.append(std::shared_ptr<Track>(new FusionTrack(subDir.path())));
                        availableTracks.append(std::shared_ptr<Track>(new FusionToRisingTrack(subDir.path())));
                        ++fusionTrackCount;
                    } else {
                        // Rising track
                        availableTracks.append(std::shared_ptr<Track>(new RisingTrack(subDir.path())));
                        ++risingTrackCount;
                    }
                }
                statusProgress->setValue(++dirProcessedCount);
            }
        }

        statusProgress->setVisible(false);

        // Select the correct game, Rising by default
        if(risingTrackCount == 0 && fusionTrackCount > 0) {
            ui->fusionRadioButton->setChecked(true);
        }

        qDebug() << "Scan complete\n";
    } else {
        availableTracks.clear();
        setupAvailableList();
    }
}

/*
 * Setup a list of tracks that can be exported to the editor
 */
void TrialsEditorTool::setupAvailableList()
{
    ui->availableTracksList->clear();
    // Add available tracks to the QListWidget
    foreach(std::shared_ptr<Track> track, availableTracks) {
        // Only show tracks for the currently selected game
        if(ui->risingRadioButton->isChecked()) {
            track = std::dynamic_pointer_cast<RisingTrack>(track);
        } else if(ui->fusionRadioButton->isChecked()) {
            track = std::dynamic_pointer_cast<FusionTrack>(track);
        } else if(ui->fusionToRisingRadioButton->isChecked()) {
            track = std::dynamic_pointer_cast<FusionToRisingTrack>(track);
        }

        // Tracks for the wrong game couldn't be cast from base type to derived type
        if(track != nullptr) {
            if(track->getPath().contains("-0000000000000")) {
                ui->availableTracksList->addItem(track->getName() + " (editor)");
            } else {
                //ui->availableTracksList->addItem(track->getName());
                QByteArray temp = track->getThumbnail();
                QImage trackThumbnail = QImage::fromData(temp);
                /*qDebug() << temp.left(20);
                qDebug() << trackThumbnail.size();*/
                QListWidgetItem *newItem = new QListWidgetItem(QIcon(QPixmap::fromImage(trackThumbnail.rgbSwapped())), track->getName());
                newItem->setBackgroundColor(QColor(51, 51, 51));
                newItem->setTextColor(Qt::white);
                newItem->setFlags(newItem->flags() | Qt::ItemIsUserCheckable);
                newItem->setCheckState(Qt::Unchecked);
                ui->availableTracksList->addItem(newItem);
            }
        }
    }
    ui->exportTracksList->hide();

    // Show how many tracks are in the list
    if(ui->availableTracksList->count() == 1) {
        ui->statusBar->showMessage("Found " + QString::number(ui->availableTracksList->count()) + " track");
    } else {
        ui->statusBar->showMessage("Found " + QString::number(ui->availableTracksList->count()) + " tracks");
    }
}

/*
 * Initialize config using Rising save directory
 */
bool TrialsEditorTool::initWithRising(QDir dir)
{
    if(!config.initialize(dir, Config::Rising)) {
        qDebug() << "Config init with Rising failed";
        ui->statusBar->showMessage("Config initialization failed: no uplay id found.");
        return false;
    }
    return true;
}

/*
 * Initialize config using Fusion save directory
 */
bool TrialsEditorTool::initWithFusion(QDir dir)
{ 
    // Initialization with Fusion uses a track created by the user
    // If track can't be found ask the user to create one in a dialog
    while(!config.initialize(dir, Config::Fusion)) {
        qDebug() << "Config init with Fusion failed";

        // Setup the dialog
        ConfigDialog dialog;
        dialog.setModal(true);

        // Keep asking the user to create a track until a track is found or user presses cancel
        if(!dialog.exec()) {
            ui->statusBar->showMessage("Config initialization failed: no uplay id found.");
            return false;
        }
    }
    return true;
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

        // Clear export track list
        exportTracks.clear();
        ui->exportTracksList->clear();
        ui->exportTrackButton->setEnabled(false);
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
            QString selectedTrackName = item->text();
            // Check if the track is editor track or downloaded track to avoid issues when
            // an editor track and a downloaded track have the same name
            if(selectedTrackName.contains(" (editor)")) {
                selectedTrackName.chop(9);
                if(track->getPath().contains("-0000000000000") && track->getName() == selectedTrackName) {
                    trackInExport = true;
                    qDebug() << "Track is already added to export";
                }
            } else if(!track->getPath().contains("-0000000000000") && track->getName() == selectedTrackName) {
                trackInExport = true;
                qDebug() << "Track is already added to export";
            }
        }

        // Find the selected track from favorites and add it to export
        if(!trackInExport) {
            foreach(std::shared_ptr<Track> track, availableTracks) {
                // Only select tracks for the currently selected game
                if(ui->risingRadioButton->isChecked()) {
                    track = std::dynamic_pointer_cast<RisingTrack>(track);
                } else if(ui->fusionRadioButton->isChecked()) {
                    track = std::dynamic_pointer_cast<FusionTrack>(track);
                } else if(ui->fusionToRisingRadioButton->isChecked()) {
                    track = std::dynamic_pointer_cast<FusionToRisingTrack>(track);
                }

                // Tracks for the wrong game couldn't be cast from base type to derived type
                if(track != nullptr) {
                    QString selectedTrackName = item->text();
                    // Check if the track is editor track or downloaded track to avoid issues when
                    // an editor track and a downloaded track have the same name
                    if(selectedTrackName.contains(" (editor)")) {
                        selectedTrackName.chop(9);
                        if(track->getPath().contains("-0000000000000") && track->getName() == selectedTrackName) {
                            exportTracks.append(track);
                            ui->exportTracksList->addItem(item->text());
                            ui->exportTrackButton->setEnabled(true);
                        }
                    } else if(!track->getPath().contains("-0000000000000") && track->getName() == selectedTrackName) {
                        exportTracks.append(track);
                        ui->exportTracksList->addItem(item->text());
                        ui->exportTrackButton->setEnabled(true);
                    }
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
            QString selectedTrackName = item->text();
            // Check if the track is editor track or downloaded track to avoid issues when
            // an editor track and a downloaded track have the same name
            if(selectedTrackName.contains(" (editor)")) {
                selectedTrackName.chop(9);
                if(track->getPath().contains("-0000000000000") && track->getName() == selectedTrackName) {
                    exportTracks.removeAll(track);
                    qDebug() << "Removed track: " << item->text();
                }
            } else if(!track->getPath().contains("-0000000000000") && track->getName() == selectedTrackName) {
                exportTracks.removeAll(track);
                qDebug() << "Removed track: " << item->text();
            }
        }
        // Remove track from export tracks list
        delete item;
    }
    if(exportTracks.isEmpty()) {
        ui->exportTrackButton->setEnabled(false);
    }
}

void TrialsEditorTool::on_exportTrackButton_clicked()
{
    // Get tracks that currently are in the editor
    scanEditor();

    // Get the currently selected game directory
    QDir saveDir;
    if(ui->risingRadioButton->isChecked() || ui->fusionToRisingRadioButton->isChecked()) {
        saveDir = risingSaveDir;
    } else if(ui->fusionRadioButton->isChecked()){
        saveDir = fusionSaveDir;
    }

    // Setup progress bar
    int exportProcessedCount = 0;
    int exportedCount = 0;
    statusProgress->setMaximum(exportTracks.count());
    statusProgress->setVisible(true);

    // Iterate through all the tracks that will be exported
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

        // Export track if it doesn't exist or overwrite was approved
        if(allowExport) {
            exportTrack->exportToEditor(config.getConfig().value("userId"), saveDir);
            ++exportedCount;
        }
        statusProgress->setValue(++exportProcessedCount);
    }

    // Show how many tracks were exported
    if(exportedCount == 1) {
        ui->statusBar->showMessage(QString::number(exportedCount) + " track exported to editor");
    } else {
        ui->statusBar->showMessage(QString::number(exportedCount) + " tracks exported to editor");
    }

    // Clear export tracks after they've been exported
    exportTracks.clear();
    ui->exportTracksList->clear();
    ui->exportTrackButton->setEnabled(false);

    statusProgress->setVisible(false);
}

void TrialsEditorTool::on_favoritesButton_clicked()
{
    // Set the current directory path in UI to the selected game
    if(ui->risingRadioButton->isChecked()) {
        ui->selectDirLineEdit->setText(risingSaveDir.path());
    } else if(ui->fusionRadioButton->isChecked()  || ui->fusionToRisingRadioButton->isChecked()){
        ui->selectDirLineEdit->setText(fusionSaveDir.path());
    }

    scanDownloads();
    setupAvailableList();
}

void TrialsEditorTool::on_selectDirLineEdit_editingFinished()
{
    scanBrowseDir(ui->selectDirLineEdit->text());
    setupAvailableList();
}

void TrialsEditorTool::on_risingRadioButton_toggled(bool checked)
{
    if(checked) {
        // Change current directory path in UI
        if(ui->selectDirLineEdit->text() == fusionSaveDir.path()) {
            ui->selectDirLineEdit->setText(risingSaveDir.path());
        }
        setupAvailableList();

        // Clear export track list
        exportTracks.clear();
        ui->exportTracksList->clear();
        ui->exportTrackButton->setEnabled(false);
    }
}

void TrialsEditorTool::on_fusionRadioButton_toggled(bool checked)
{
    if(checked) {
        // Change current directory path in UI
        if(ui->selectDirLineEdit->text() == risingSaveDir.path()) {
            ui->selectDirLineEdit->setText(fusionSaveDir.path());
        }
        setupAvailableList();

        // Clear export track list
        exportTracks.clear();
        ui->exportTracksList->clear();
        ui->exportTrackButton->setEnabled(false);
    }
}

void TrialsEditorTool::on_fusionToRisingRadioButton_toggled(bool checked)
{
    if(checked) {
        // Change current directory path in UI
        if(ui->selectDirLineEdit->text() == risingSaveDir.path()) {
            ui->selectDirLineEdit->setText(fusionSaveDir.path());
        }
        setupAvailableList();

        // Clear export track list
        exportTracks.clear();
        ui->exportTracksList->clear();
        ui->exportTrackButton->setEnabled(false);
    }
}
