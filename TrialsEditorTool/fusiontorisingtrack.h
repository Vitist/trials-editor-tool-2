#ifndef FUSIONTORISINGTRACK_H
#define FUSIONTORISINGTRACK_H

#include "track.h"

class FusionToRisingTrack : public Track
{
public:
    FusionToRisingTrack(QString trackPath);
    virtual ~FusionToRisingTrack();

    QString getName() const;
    QString getPath() const;
    bool exportToEditor(QString userId, QDir saveDir) const;
    bool removeFromDisk() const;

private:
    QString path;
    QString name;

    void convertMetadata(QString trackPath, QByteArray userId) const;
    void copyFiles(QString destination) const;
};

#endif // FUSIONTORISINGTRACK_H
