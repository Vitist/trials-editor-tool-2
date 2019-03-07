#ifndef TRACK_H
#define TRACK_H

#include <QString>
#include <QDir>

class Track
{
public:
    Track();
    virtual ~Track();
    virtual QString getName() const = 0;
    virtual bool exportToEditor(QString userId, QDir saveDir) const = 0;
    virtual bool removeFromDisk() const = 0;
};

#endif // TRACK_H
