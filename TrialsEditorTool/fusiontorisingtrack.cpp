#include "fusiontorisingtrack.h"
#include <QFile>
#include <QDir>
#include <QDebug>

FusionToRisingTrack::FusionToRisingTrack(QString trackPath): path(trackPath)
{
    QFile file(trackPath + "\\displayname");
    if(file.open(QIODevice::ReadOnly)) {
        QByteArray line = file.readLine();
        name = QString::fromUtf8(line);
        file.close();
    }
}

FusionToRisingTrack::~FusionToRisingTrack()
{

}

QString FusionToRisingTrack::getName() const
{
    return name;
}

QString FusionToRisingTrack::getPath() const
{
    return path;
}

QByteArray FusionToRisingTrack::getThumbnail() const
{
    // No thumbnail files available in Fusion
    return QByteArray("NO_THUMBNAIL");
}

bool FusionToRisingTrack::exportToEditor(QString userId, QDir saveDir) const
{
    // Get track directory name
    QStringList pathDirs = path.split("/");
    QString trackDir = pathDirs.takeLast();
    // Track save timestamp, used in Rising as track directory name
    QString exportTrackDir = trackDir.mid(32, 8);
    // Switch to Rising editor directory
    saveDir.cd("usertracks");
    QString exportPath = saveDir.path() + QDir::separator() + exportTrackDir;
    qDebug() << "Track path: " + path;
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

    // Convert metadata file to work in the users editor
    convertMetadata(exportPath, userIdBytes);

    // TODO: change to void?
    return true;
}

// Delete track files from SaveGames
// Not used at this point
bool FusionToRisingTrack::removeFromDisk() const
{
    /*QDir dir(path);
    qDebug() << "Removing: " << path;
    return dir.removeRecursively();*/
    return false;
}

// Convert metadata file
void FusionToRisingTrack::convertMetadata(QString trackPath, QByteArray userId) const
{
    QFile file(trackPath + QDir::separator() + "metadata.mda");
    file.resize(29);
    if(file.open(QIODevice::ReadWrite)) {
        qDebug() << "Converting metadata " << name;
        QByteArray content = file.readAll();

        // Game id?
        content.replace(4, 1, QByteArray::fromHex("16"));
        // User id
        content.replace(5, 16, userId);
        // All rising editor track metadata files include these parts
        content.append(QByteArray::fromHex("01"));
        content.append(QByteArray::fromHex("00").repeated(99));
        // Start of track description, creator name and track name
        content.append(QByteArray::fromHex("4E20"));
        content.append(QByteArray::fromHex("00").repeated(7));
        QByteArray creatorName = "FusionPorted";
        int8_t creatorNameSize = static_cast<int8_t>(creatorName.size());
        content.append(creatorNameSize);
        content.append(creatorName);
        content.append(QByteArray::fromHex("010000"));
        int8_t trackNameSize = static_cast<int8_t>(name.size());
        content.append(trackNameSize);
        content.append(name);
        // Creator name again?
        content.append(creatorNameSize);
        content.append(creatorName);
        content.append(QByteArray::fromHex("00").repeated(10));
        // User id again?
        content.append(userId);

        file.seek(0);
        file.write(content);
        file.close();
    } else {
        qDebug() << "Couldn't open metadata file " << name;
    }
}

// Copy all files inside a directory into a new destination
void FusionToRisingTrack::copyFiles(QString destination) const
{
    QDir dir(path);
    foreach (QString fileName, dir.entryList(QStringList() << "track*" << "metadata*", QDir::Files)) {
        QFile::copy(path + QDir::separator() + fileName, destination + QDir::separator() + fileName);
    }
}
