#include "risingtrack.h"
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QDataStream>
#include <FreeImage.h>

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

QString RisingTrack::getPath() const
{
    return path;
}

QByteArray RisingTrack::getThumbnail() const
{
    FreeImage_SetOutputMessage([](FREE_IMAGE_FORMAT fif, const char *message) {
        qDebug() << "Format: " << FreeImage_GetFormatFromFIF(fif);
        qDebug() << "Message: " << message;
    });
    // TODO: return QImage?
    QFile file(path + QDir::separator() + "thumbnail.thn");
    FREE_IMAGE_FORMAT fiftest = FreeImage_GetFileType((path + "/thumbnail.jxr").toStdString().c_str(), 0);
    if(fiftest == FIF_UNKNOWN) {
        qDebug() << "Test unknown";
    }

    QByteArray imageData;
    if(file.open(QIODevice::ReadOnly)) {
        qDebug() << "Extracting thumbnail " << name;

        // START OF NEW FILE READ
        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian);

        // TODO: Fix this shit
        // Scrap the vector, create larger array that will always be able to fit the entire image 30kb?
        // Take the data length from bytes read instead
        std::vector<unsigned char> rawData;
        // Skip Redlynx header, not sure if always the same length
        stream.skipRawData(43);
        while(!stream.atEnd()) {
            char *buffer = new char[1024];
            int bytesRead = stream.readRawData(buffer, 1024);
            unsigned char *uBuffer = reinterpret_cast<unsigned char *>(buffer);
            for(int i = 0; i < bytesRead; i++) {
                rawData.push_back(uBuffer[i]);
            }
            delete[] buffer;
        }

        // This will be useless, do reinterpret_cast to unsigned char here instead
        unsigned char *data = new unsigned char[rawData.size()];
        for(unsigned int i = 0; i < rawData.size(); i++) {
            data[i] = rawData.at(i);
            // qDebug() << rawData.at(i);
        }
        qDebug() << rawData.size();
        // END OF NEW FILE READ

        // Check if bytes read is too small to fit an image instead
        QByteArray content = file.readAll();
        if (!content.contains("WMPHOTO")) {
            qDebug() << "Thumbnail file contains an image";
            // Remove extra Redlynx header
            int imageDataStart = content.indexOf(QByteArray::fromHex("4949BC01"));
            content.remove(0, imageDataStart);

            // Read jxr image data into FreeImage bitmap
            /*BYTE *jxrData = reinterpret_cast<BYTE *>(content.data());
            qDebug() << "Content: " << content.right(10).toHex();
            qDebug() << content.size();
            qDebug() << static_cast<DWORD>(content.size() + 1);

            for (int i = 0; i < 27521; i++) {
                qDebug() << *jxrData;
                ++jxrData;
            }*/
            //FIMEMORY *srcStream = FreeImage_OpenMemory(jxrData, static_cast<DWORD>(content.size() + 1));
            FIMEMORY *srcStream = FreeImage_OpenMemory(data, rawData.size());
            FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(srcStream);
            if(fif == FIF_UNKNOWN) {
                qDebug() << "Unknown image format";
                imageData = QByteArray("ERROR");
            } else {
                FIBITMAP *srcBitMap = FreeImage_LoadFromMemory(fif, srcStream);
                //FIBITMAP *srcBitMap = FreeImage_Load(/*FIF_JXR*/fiftest, (path + "/thumbnail.jxr").toStdString().c_str());

                // Convert jxr bitmap to jpeg bitmap
                FIMEMORY *destStream = FreeImage_OpenMemory();
                srcBitMap = FreeImage_ConvertTo24Bits(srcBitMap);
                FreeImage_FlipVertical(srcBitMap);
                FreeImage_SaveToMemory(FIF_JPEG, srcBitMap, destStream);

                // Load JPEG data
                BYTE *mem_buffer = nullptr;
                DWORD size_in_bytes = 0;
                FreeImage_AcquireMemory(destStream, &mem_buffer, &size_in_bytes);

                // Free memory
                FreeImage_Unload(srcBitMap);
                FreeImage_CloseMemory(srcStream);
                FreeImage_CloseMemory(destStream);

                // Load raw data into QByteArray
                imageData = QByteArray::fromRawData(reinterpret_cast<char *>(mem_buffer), static_cast<long>(size_in_bytes));
            }
        } else {
            qDebug() << "Thumbnail file doesn't contain an image";
            imageData = QByteArray("EDITOR");
        }
        delete[] data;
        file.close();
    } else {
        qDebug() << "Couldn't open thumbnail file " << name;
        imageData = QByteArray("NO_THUMBNAIL");
    }
    return imageData;
}

bool RisingTrack::exportToEditor(QString userId, QDir saveDir) const
{

    QDir downloadsDir = saveDir;
    QDir editorDir = saveDir;

    downloadsDir.cd("CacheStorage/usertracks");
    editorDir.cd("usertracks");
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
