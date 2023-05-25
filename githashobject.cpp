#include "githashobject.h"

#include <QDebug>
#include <QCryptographicHash>
#include <iostream>
using namespace std;

GitHashObject::GitHashObject()
{
    // TEST

}

QByteArray GitHashObject::createHash(const QByteArray &content)
{
    static QCryptographicHash hash = QCryptographicHash(QCryptographicHash::Sha1);
    auto header = QString("blob %0").arg(content.size()).toUtf8();
    hash.addData(header.data(), header.size() + 1);
    hash.addData(content);
    auto checksum = hash.result();
    return checksum;
}
