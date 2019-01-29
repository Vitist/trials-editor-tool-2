#ifndef TRACK_H
#define TRACK_H

#include <QString>
#include <QDir>

class Track
{
public:
    Track(QString trackPath);
    QString getName();
    void convertTrack(QString trackPath, QByteArray userId);
    void convertMetadata(QString trackPath, QByteArray userId);
    bool exportToEditor(QString userId, QDir saveDir);
    void copyFiles(QString destination);

    bool operator==(const Track &track) const;
private:
    QString path;
    QString name;
};

#endif // TRACK_H
