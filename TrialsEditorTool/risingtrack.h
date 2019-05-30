#ifndef RISINGTRACK_H
#define RISINGTRACK_H

#include "track.h"

class RisingTrack : public Track
{
public:
    RisingTrack(QString trackPath);
    virtual ~RisingTrack();

    QString getName() const;
    QString getPath() const;
    QByteArray getThumbnail() const;
    bool exportToEditor(QString userId, QDir saveDir) const;
    bool removeFromDisk() const;

private:
    QString path;
    QString name;

    void convertMetadata(QString trackPath, QByteArray userId) const;
    void copyFiles(QString destination) const;
};

#endif // RISINGTRACK_H
