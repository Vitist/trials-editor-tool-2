#include "trialseditortool.h"
#include "ui_trialseditortool.h"
#include "track.h"
#include <QFileDialog>
#include <QDirIterator>
#include <QStandardPaths>
#include <QDebug>

TrialsEditorTool::TrialsEditorTool(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TrialsEditorTool)
{
    ui->setupUi(this);

    const QString documentsDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if(!documentsDirPath.isEmpty()) {
        QDir saveGamesDir(documentsDirPath + "/TrialsFusion/SavedGames");
        scanDir(saveGamesDir);
    }
    else {
        // TODO: Ask user to find correct folder
    }
}

TrialsEditorTool::~TrialsEditorTool()
{
    delete ui;
}

void TrialsEditorTool::scanDir(QDir dir)
{
    qDebug() << "Scanning: " << dir.path();

    QFileInfoList trackDirectories = dir.entryInfoList(QStringList() << "*-0000000*", QDir::Dirs | QDir::NoDotAndDotDot);

    QString editorTrackIndex = "0000000000000";
    foreach(QFileInfo track, trackDirectories) {
        if(track.filePath().contains(editorTrackIndex)) {
            editorTracks.push_back(Track(track.filePath()));
        }
        else {
            favoriteTracks.push_back(Track(track.filePath()));
        }
    }

    /* Populate the two lists
     * TODO: create a funtion for this
     */
    foreach(Track track, favoriteTracks) {
        qDebug() << "Adding favorite: " << track.getName();
        ui->availableTracksList->addItem(track.getName());
    }

    foreach(Track track, editorTracks) {
        qDebug() << "Adding editor: " << track.getName();
        ui->editorTracksList->addItem(track.getName());
    }

    /*
    QString userId = "4bf00161bc0597676a50b5322a159cd5";
    QString platform = "deadbabe";

    Track track = favoriteTracks.first();
    track.exportToEditor(userId, platform, dir);
    */
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

void TrialsEditorTool::on_removeTrackButton_clicked()
{
    // TODO: remove track from editor list
    qDebug() << "Selected track: " << ui->editorTracksList->currentItem()->text();
}

void TrialsEditorTool::on_addTrackButton_clicked()
{
    // TODO: add track to editor list
    qDebug() << "Selected track: " << ui->availableTracksList->currentItem()->text();
}
