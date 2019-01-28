#include "trialseditortool.h"
#include "ui_trialseditortool.h"
#include <iostream>
#include <QFileDialog>
#include <QTextStream>
#include <QDirIterator>
#include <QStandardPaths>

TrialsEditorTool::TrialsEditorTool(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TrialsEditorTool)
{
    ui->setupUi(this);

    const QString documentsFolderPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if(!documentsFolderPath.isEmpty()) {
        QDir saveGamesFolder(documentsFolderPath + "/TrialsFusion/SavedGames");
        scanDir(saveGamesFolder);
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
    /*dir.setNameFilters(QStringList("displayname"));
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);*/

    QTextStream outputStream(stdout);

    outputStream << "Scanning: " << dir.path();

    QFileInfoList trackDirectories = dir.entryInfoList(QStringList() << "*-0000000*", QDir::Dirs | QDir::NoDotAndDotDot);

    QString editorTrackIndex = "0000000000000";
    foreach(QFileInfo track, trackDirectories) {
        if(track.filePath().contains(editorTrackIndex)) {
            editorTracks.push_back(track);
        }
        else {
            favoriteTracks.push_back(track);
        }
    }

    /* Populate the two lists
     * TODO: create a funtion for this
     */
    foreach(QFileInfo track, favoriteTracks) {
        outputStream << track.filePath() << "\n";
        QFile file(track.filePath() + "\\displayname");
        file.open(QIODevice::ReadOnly);
        QByteArray line = file.readLine();
        file.close();
        ui->availableTracksList->addItem(QString::fromUtf8(line));
    }

    foreach(QFileInfo track, editorTracks) {
        outputStream << track.filePath() << "\n";
        QFile file(track.filePath() + "\\displayname");
        file.open(QIODevice::ReadOnly);
        QByteArray line = file.readLine();
        file.close();
        ui->editorTracksList->addItem(QString::fromUtf8(line));
    }

    QString userId = "4bf00161bc0597676a50b5322a159cd5";
    QString platform = "deadbabe";

    QByteArray userIdBytes = QByteArray::fromHex(userId.toLatin1());
    QByteArray platformBytes = QByteArray::fromHex(platform.toLatin1());

    QFileInfo testTrack = favoriteTracks.first();
    QFile file(testTrack.filePath() + "\\track.trk");
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    data.replace(0, 4, platformBytes);
    QFile testFile("D:\\Teemu\\test\\test.trk");
    testFile.open(QIODevice::WriteOnly);
    testFile.write(data);
    testFile.close();
    //outputStream << data;

    /*QDirIterator it(dir.path(), QStringList() << "displayname", QDir::NoFilter, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QFile file(it.next());
        file.open(QIODevice::ReadOnly);
        QByteArray line = file.readLine();
        //outputStream << file.fileName() << " " << QString::fromUtf8(line) << "\n";
        ui->availableTracksList->addItem(QString::fromUtf8(line));
    }*/

    outputStream << "\nScan complete\n";
}

void TrialsEditorTool::on_browseButton_clicked()
{
    std::cout << "browseButton click" << std::endl;
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    QList<QUrl> folderNames;
    if (dialog.exec()) {
        folderNames = dialog.selectedUrls();
    }
    std::cout << "selected folders count: " << folderNames.size() << std::endl;
    QTextStream outputStream(stdout);
    outputStream << "selected folder name: " << folderNames.front().toString();
    // C:\Users\Teemu\Documents\TrialsFusion\SavedGames
}

void TrialsEditorTool::on_removeTrackButton_clicked()
{
    // TODO: remove track from editor list
    QTextStream outputStream(stdout);
    outputStream << "Selected track: " << ui->editorTracksList->currentItem()->text() << "\n";
}

void TrialsEditorTool::on_addTrackButton_clicked()
{
    // TODO: add track to editor list
    QTextStream outputStream(stdout);
    outputStream << "Selected track: " << ui->availableTracksList->currentItem()->text() << "\n";
}
