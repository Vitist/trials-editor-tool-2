#include "track.h"
#include <QFile>
#include <QDir>
#include <QDebug>

Track::Track(QString trackPath): path(trackPath)
{
    QFile file(trackPath + "\\displayname");
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray line = file.readLine();
        name = QString::fromUtf8(line);
        file.close();
    }
}

QString Track::getName()
{
    return name;
}

void Track::convertTrack(QString trackPath, QByteArray userId)
{
    QFile file(trackPath + QDir::separator() + "track.trk");
    if(file.open(QIODevice::ReadWrite)) {
        qDebug() << "Converting track " << name;
        QByteArray content = file.readAll();
        content.replace(9, 16, userId);
        file.seek(0);
        file.write(content);
    } else {
        qDebug() << "Couldn't open track file " << name;
    }
    file.close();
}

void Track::convertMetadata(QString trackPath, QByteArray userId)
{
    QFile file(trackPath + QDir::separator() + "metadata.mda");
    if(file.open(QIODevice::ReadWrite)) {
        qDebug() << "Converting metadata " << name;
        QByteArray content = file.readAll();
        content.replace(5, 16, userId);
        file.seek(0);
        file.write(content);
        file.close();
    } else {
        qDebug() << "Couldn't open metadata file " << name;
    }
}

bool Track::exportToEditor(QString userId, QDir saveDir)
{
    // Get track directory name
    QStringList pathDirs = path.split("/");
    QString trackDir = pathDirs.takeLast();
    // Track index, 0 for editor tracks
    QString editorTrackDirEnd = "-0000000000000";
    // Track id consisting of a timestamp in seconds and gamemode indicator
    QString trackId = trackDir.mid(32, 12);
    // Construct a path for a new editor track using an existing track in favorites
    QString exportTrackDir = userId + trackId + editorTrackDirEnd;
    QString exportPath = saveDir.path() + QDir::separator() + exportTrackDir;
    qDebug() << "Favorite path: " + path;
    qDebug() << "Export path: " + exportPath;
    // Create a directory for the converted track
    bool dirCreated = saveDir.mkdir(exportTrackDir);
    if(dirCreated) {
        qDebug() << "Directory created";
    } else {
        qDebug() << "Directory could not be created or already exists";
    }
    copyFiles(exportPath);

    // Convert hex strings to byte arrays
    QByteArray userIdBytes = QByteArray::fromHex(userId.toLatin1());

    // Convert track and metadata files to work in the users editor
    convertTrack(exportPath, userIdBytes);
    convertMetadata(exportPath, userIdBytes);

    // TODO: change to void?
    return true;
}

// Copy all files inside a directory into a new destination
void Track::copyFiles(QString destination)
{
    QDir dir(path);
    foreach (QString fileName, dir.entryList(QDir::Files)) {
        QFile::copy(path + QDir::separator() + fileName, destination + QDir::separator() + fileName);
    }
}

// Delete track files from SaveGames
bool Track::removeFromDisk()
{
    QDir dir(path);
    qDebug() << "Removing: " << path;
    return dir.removeRecursively();
}

bool Track::operator==(const Track &track) const
{
    return track.name == name && track.path == path;
}
