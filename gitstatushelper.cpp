#include "gitstatushelper.h"
#include "gitindexfile.h"

GitStatusHelper::GitStatusHelper()
{

}

void GitStatusHelper::setDir(const QDir &dir)
{
    m_dir = dir;
}
