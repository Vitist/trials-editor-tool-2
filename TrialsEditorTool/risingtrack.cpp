#include "risingtrack.h"
#include <QFile>
#include <QDir>
#include <QDebug>

RisingTrack::RisingTrack(QString trackPath): path(trackPath)
{
    // Find track name here
   QFile file(trackPath + QDir::separator() + "metadata.mda");
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray content = file.readAll();
        QByteArray trackInfoStart = QByteArray::fromHex("4E20");
        // Track description
        int readNextIndex = content.indexOf(trackInfoStart) + 2;
        int8_t dataLength = content.at(readNextIndex);
        // Track creator
        readNextIndex += dataLength + 7;
        dataLength = content.at(readNextIndex);
        // Track name
        readNextIndex += dataLength + 4;
        dataLength = content.at(readNextIndex);
        name = QString::fromUtf8(content.mid(readNextIndex + 1, dataLength));
        file.close();
    }
}

RisingTrack::~RisingTrack()
{

}


QString RisingTrack::getName() const
{
    return name;
}

bool RisingTrack::exportToEditor(QString userId, QDir saveDir) const
{

    QDir downloadsDir = saveDir;
    QDir editorDir = saveDir;

    // Find the user directory inside SavedGames
    QFileInfoList userDirectories = downloadsDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    if (!userDirectories.empty()) {
        downloadsDir.cd(userDirectories.first().fileName() + "/CacheStorage/usertracks");
        editorDir.cd(userDirectories.first().fileName() + "/usertracks");
        qDebug() << downloadsDir.path();
        qDebug() << editorDir.path();

        // Get track directory name
        QStringList pathDirs = path.split("/");
        QString trackDir = pathDirs.takeLast();
        QString exportPath = editorDir.path() + QDir::separator() + trackDir;
        qDebug() << "Download path: " + path;
        qDebug() << "Export path: " + exportPath;
        // Create a directory for the converted track
        bool dirCreated = editorDir.mkdir(trackDir);
        if(dirCreated) {
            qDebug() << "Directory created";
        } else {
            qDebug() << "Directory could not be created or already exists";
        }
        copyFiles(exportPath);

        // Convert hex strings to byte arrays
        QByteArray userIdBytes = QByteArray::fromHex(userId.toLatin1());

        // Convert track and metadata files to work in the users editor
        convertMetadata(exportPath, userIdBytes);

        return true;
    } else {
        qDebug() << "No Rising user directory found";
        return false;
    }
}

// Delete track files from SaveGames
bool RisingTrack::removeFromDisk() const
{
    QDir dir(path);
    qDebug() << "Removing: " << path;
    return dir.removeRecursively();
}

void RisingTrack::convertMetadata(QString trackPath, QByteArray userId) const
{
    QFile file(trackPath + QDir::separator() + "metadata.mda");
    if(file.open(QIODevice::ReadWrite)) {
        qDebug() << "Converting metadata " << name;
        QByteArray content = file.readAll();
        content.replace(5, 16, userId);
        QByteArray zeros = QByteArray::fromHex("00");
        content.replace(30, 8, zeros.repeated(8));
        file.seek(0);
        file.write(content);
        file.close();
    } else {
        qDebug() << "Couldn't open metadata file " << name;
    }
}

// Copy all files inside a directory into a new destination
void RisingTrack::copyFiles(QString destination) const
{
    QDir dir(path);
    foreach (QString fileName, dir.entryList(QDir::Files)) {
        QFile::copy(path + QDir::separator() + fileName, destination + QDir::separator() + fileName);
    }
}
