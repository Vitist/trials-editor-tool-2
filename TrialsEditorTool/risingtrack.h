#ifndef RISINGTRACK_H
#define RISINGTRACK_H

#include "track.h"

class RisingTrack : public Track
{
public:
    RisingTrack();
    virtual ~RisingTrack();

    QString getName() const;
    bool exportToEditor(QString userId, QDir saveDir) const;
    bool removeFromDisk() const;

private:
    QString path;
    QString name;

    void convertMetadata(QString trackPath, QByteArray userId) const;
    void copyFiles(QString destination) const;
};

#endif // RISINGTRACK_H
