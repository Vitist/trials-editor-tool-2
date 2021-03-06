#include "config.h"
#include <QFile>
#include <QDebug>
#include <QRegularExpression>
#include <QMap>

Config::Config(): userId("")
{
}

bool Config::initialize(QDir saveDir, Game game)
{
    bool success = true;
    if(!load()){
        if(game == Fusion) {
            // Editor tracks end with -0000000000000
            QFileInfoList editorTrackDirectories = saveDir.entryInfoList(QStringList() << "*-0000000000000", QDir::Dirs | QDir::NoDotAndDotDot);

            if(!editorTrackDirectories.isEmpty()) {
                // User might have someone elses editor track in the SaveGames directory
                // Count each user id found from editor track directory names
                QMap<QString, int> userIdCount;
                foreach(QFileInfo track, editorTrackDirectories) {
                    QString id = track.fileName().left(32);
                    userIdCount[id]++;
                }

                // Set the user id to be the one that was repeated the most
                // User id will be wrong if there are more editor tracks from someone else than the user
                int count = 0;
                foreach(QString id, userIdCount.keys()) {
                    if(userIdCount[id] > count) {
                        userId = id;
                    }
                }

                qDebug() << userIdCount;
                qDebug() << "Config initialized with user id: " << userId;
                save();
            } else {
                qDebug() << "No editor tracks found for config initialization";
                success = false;
            }
        } else {
            // Extract uplay id from save directory name
            QString id = saveDir.dirName();
            id.remove("-");
            QStringList idParts;
            while(id.length() > 0) {
                idParts << id.left(4);
                id.remove(0, 4);
            }
            // Rearrange the id into the form used by the games
            idParts.swap(0,3);
            idParts.swap(1,2);
            idParts.swap(2,3);
            idParts.swap(4,7);
            idParts.swap(5,6);
            userId = idParts.join("");
            qDebug() << "Config initialized with user id: " << userId;
            save();
        }
    }
    return success;
}

QMap<QString, QString> Config::getConfig()
{
    QMap<QString, QString> config;
    config.insert("userId", userId);
    return config;
}

bool Config::load()
{
    // Check if config file has already been read
    if(userId.isEmpty()) {
        QFile file("config.txt");
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            // Couldn't open file
            return false;
        }

        // User id hex string matcher
        QRegularExpression hexMatcher("^[0-9A-F]{32}$", QRegularExpression::CaseInsensitiveOption);
        while(!file.atEnd()) {
            QString line = file.readLine();
            QStringList lineKeyAndValue = line.split("=");
            // Make sure the config file lines are valid
            if(lineKeyAndValue.size() == 2) {
                QRegularExpressionMatch match = hexMatcher.match(lineKeyAndValue.at(1));
                if(lineKeyAndValue.at(0) == "userId" && match.hasMatch()) {
                    userId = lineKeyAndValue.at(1);
                    qDebug() << "userId: " << userId;
                } else {
                    // Something wrong with config file
                    return false;
                }
            } else {
                // Something wrong with config file
                return false;
            }
        }
        file.close();
    }

    return true;
}

void Config::save()
{
    QFile file("config.txt");
    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        file.write("userId=" + userId.toUtf8());
    }
    file.close();
}
