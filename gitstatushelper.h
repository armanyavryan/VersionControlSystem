#pragma once
#include <QObject>
#include <QDir>
class GitStatusHelper
{
public:
    GitStatusHelper();
    void setDir(const QDir& dir);
    bool hasCommit();
    QString branchName();
    QStringList untrackedFiles();
    QDir m_dir;
};
