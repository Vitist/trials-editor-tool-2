#include "config.h"
#include <QFile>
#include <QDebug>
#include <QRegularExpression>
#include <QMap>

Config::Config(): userId("")
{
}

bool Config::initialize(QDir saveDir)
{
    bool success = true;
    if(!load()){
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
        } else {
            qDebug() << "No editor tracks found for config initialization";
            success = false;
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
