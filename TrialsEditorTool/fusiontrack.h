#ifndef FUSIONTRACK_H
#define FUSIONTRACK_H

#include "track.h"

class FusionTrack : public Track
{
public:
    FusionTrack(QString trackPath);
    virtual ~FusionTrack();

    QString getName() const;
    QString getPath() const;
    bool exportToEditor(QString userId, QDir saveDir) const;
    bool removeFromDisk() const;

private:
    QString path;
    QString name;

    void convertTrack(QString trackPath, QByteArray userId) const;
    void convertMetadata(QString trackPath, QByteArray userId) const;
    void copyFiles(QString destination) const;
};

#endif // FUSIONTRACK_H
