#include "trialseditortool.h"
#include "ui_trialseditortool.h"
#include <iostream>
#include <QFileDialog>
#include <QTextStream>
#include <QDirIterator>

TrialsEditorTool::TrialsEditorTool(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TrialsEditorTool)
{
    ui->setupUi(this);

    QDir saveGamesFolder("C:/Users/Teemu/Documents/TrialsFusion/SavedGames");
    scanDir(saveGamesFolder);
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

    QFileInfoList contentsInfo = dir.entryInfoList(QStringList() << "*-0000000*", QDir::Dirs | QDir::NoDotAndDotDot);

    QString excludeString = "0000000000000";
    for(int i = 0; i < contentsInfo.size(); i++) {
        if(contentsInfo.at(i).filePath().contains(excludeString)) {
            contentsInfo.removeAt(i);
            outputStream << i << "\n";
            --i; // indexes change when item is removed
        }
    }

    foreach(QFileInfo item, contentsInfo) {
        outputStream << item.filePath() << "\n";
        QFile file(item.filePath() + "\\displayname");
        file.open(QIODevice::ReadOnly);
        QByteArray line = file.readLine();
        ui->availableTracksList->addItem(QString::fromUtf8(line));
    }

    /*QDirIterator it(dir.path(), QStringList() << "displayname", QDir::NoFilter, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QFile file(it.next());
        file.open(QIODevice::ReadOnly);
        QByteArray line = file.readLine();
        //outputStream << file.fileName() << " " << QString::fromUtf8(line) << "\n";
        ui->availableTracksList->addItem(QString::fromUtf8(line));
    }*/

    outputStream << "\nScan complete";
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
    QDir saveGamesFolder("C:/Users/Teemu/Documents/TrialsFusion/SavedGames");
    scanDir(saveGamesFolder);
}
