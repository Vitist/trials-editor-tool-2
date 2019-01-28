#ifndef TRACK_H
#define TRACK_H

#include <QString>
#include <QDir>

class Track
{
public:
    Track(QFileInfo dir);
    QByteArray readHexFile();
    void writeHexFile();
    void convertTrack();
    void convertMetadata();
    bool exportToEditor();
private:
    QString path;
    QString name;
    QByteArray track;
    QByteArray metadata;
};

#endif // TRACK_H
