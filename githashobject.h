#pragma once
#include <QString>
#include <QByteArray>
class GitHashObject
{
public:
    GitHashObject();
    QByteArray createHash(const QByteArray& content);
};
