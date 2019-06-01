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
    // Setup FreeImage error message handling
    FreeImage_SetOutputMessage([](FREE_IMAGE_FORMAT fif, const char *message) {
        qDebug() << "Format: " << FreeImage_GetFormatFromFIF(fif);
        qDebug() << "Message: " << message;
    });

    // TODO: return QImage?
    QFile file(path + QDir::separator() + "thumbnail.thn");
    // TODO: Move library files to build root
    QByteArray imageData;
    if(file.open(QIODevice::ReadOnly)) {
        qDebug() << "Extracting thumbnail " << name;

        // Reader for raw data bytes
        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian);

        // 35 KB, thumbnails seem to be below 30 KB extra 5 KB added on top of that just in case
        const int maxImageSize = 35840;
        char *tempBuffer = new char[maxImageSize];

        // PIXEL FORMAT after first 8 bytes, 16 bytes long 24 C3 DD 6F 03 4E FE 4B B1 85 3D 77 76 8D C9 0F sometimes wrong
        const int pixelFormatSize = 16;
        const int pixelFormatOffset = 8;
        const unsigned char pixelFormat[pixelFormatSize] = {36, 195, 221, 111, 3, 78, 254, 75, 177, 133, 61, 119, 118, 141, 201, 15};

        // JPEG XR magic bytes
        const int magicBytesSize = 4;
        const unsigned char jxrMagicBytes[magicBytesSize] = {73, 73, 188, 01};

        // Start reading file. Skip Redlynx header, not sure if always the same length
        stream.skipRawData(43);
        int bytesRead = stream.readRawData(tempBuffer, maxImageSize);

        // Check if whole file was read
        if(bytesRead < maxImageSize && stream.atEnd()) {
            unsigned char *imageBuffer = reinterpret_cast<unsigned char *>(tempBuffer);
            // Check if there was an image in the file
            for(int i = 0; i < magicBytesSize; i++) {
                if(imageBuffer[i] != jxrMagicBytes[i]) {
                    qDebug() << imageBuffer[i] << jxrMagicBytes[i];
                    qDebug() << "No thumbnail image found";
                    delete[] tempBuffer;
                    file.close();
                    return QByteArray("NO_IMAGE");
                }
            }

            // Sometimes pixelformat section is broken, fix it
            for(int i = 0; i < pixelFormatSize; i++) {
                imageBuffer[i + pixelFormatOffset] = pixelFormat[i];
            }

            // Read image from memory
            FIMEMORY *srcStream = FreeImage_OpenMemory(imageBuffer, static_cast<unsigned long>(bytesRead));
            FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(srcStream);
            if(fif == FIF_UNKNOWN) {
                qDebug() << "Unknown image format";
                delete[] tempBuffer;
                file.close();
                return QByteArray("ERROR");
            }
            FIBITMAP *srcBitMap = FreeImage_LoadFromMemory(fif, srcStream);

            // Convert jxr bitmap to jpeg bitmap
            FIMEMORY *destStream = FreeImage_OpenMemory();
            FIBITMAP *srcBitMap24Bit = FreeImage_ConvertTo24Bits(srcBitMap);
            FreeImage_FlipVertical(srcBitMap24Bit);
            FreeImage_SaveToMemory(FIF_JPEG, srcBitMap24Bit, destStream);

            // Load JPEG data
            BYTE *mem_buffer = nullptr;
            DWORD size_in_bytes = 0;
            FreeImage_AcquireMemory(destStream, &mem_buffer, &size_in_bytes);

            // Free memory
            // TODO: figure out why closing destStream causes most images to not load
            FreeImage_Unload(srcBitMap);
            FreeImage_CloseMemory(srcStream);
            //FreeImage_CloseMemory(destStream);
            delete[] tempBuffer;

            file.close();

            // Load raw data into QByteArray
            return QByteArray::fromRawData(reinterpret_cast<char *>(mem_buffer), static_cast<long>(size_in_bytes));
        } else {
            qDebug() << "Error reading image";
            delete[] tempBuffer;
            file.close();
            return QByteArray("ERROR");
        }
    } else {
        qDebug() << "Couldn't open thumbnail file " << name;
        return QByteArray("NO_IMAGE");
    }
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
