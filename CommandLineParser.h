#pragma once

#include <QString>
#include <QRegularExpression>
#include <QCryptographicHash>

#include <QObject>
#include <QDir>
#include "gitstatushelper.h"
#include "gitindexfile.h"


class CommandLineParser : public QObject
{
    Q_OBJECT
public:
    explicit CommandLineParser(QObject *parent = nullptr);
    std::string process(const std::string& s, bool tabPressed = false);

    void printWorkingDirectory();
    void setNthTabSelectionIndex(int index);
private:
    void mkdir(const QStringList& args);
    std::string cd(const QStringList& args, bool tabPressed = false);
    void ls_files(const QStringList& args);
    void ls(const QStringList& args);
    void init(const QStringList& args);
    void add(const QStringList& args);
    void remove(const QStringList& args);
    void diff(const QStringList& args);
    void touch(const QStringList& args);
    void commit(const QStringList& args);
    void push(const QStringList& args);
    void status(const QStringList& args);
    void help();

private:
    void print(const char *data);
    void println(const char *data);
private:
    QDir pwd;
    const std::string emptystring;
    int m_nthTabSelectionIndex = 0;
    QCryptographicHash m_sha1;

    GitStatusHelper m_statusHelper;
    GitIndexFile m_indexFile;
};
