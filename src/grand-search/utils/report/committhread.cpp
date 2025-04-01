#include "committhread.h"

#include <QLibrary>
#include <QDebug>

using namespace GrandSearch;
using namespace report;

CommitLog::CommitLog(QObject *parent)
    : QObject(parent)
{
}

CommitLog::~CommitLog()
{
}

void CommitLog::commit(const QString &data)
{
    if (data.isEmpty())
        return;

    //qDebug() << data.toStdString().c_str();
    m_writeEventLog(data.toStdString());
}

bool CommitLog::init()
{
    QLibrary library("deepin-event-log");
    if (!library.load()) {
        qWarning() << "Load library failed";
        return false;
    }

    m_initEventLog = reinterpret_cast<InitEventLog>(library.resolve("Initialize"));
    m_writeEventLog = reinterpret_cast<WriteEventLog>(library.resolve("WriteEventLog"));

    if (!m_initEventLog || !m_writeEventLog) {
        qWarning() << "Library init failed";
        return false;
    }

    if (!m_initEventLog("uos-ai", false)) {
        qWarning() << "Initialize called failed";
        return false;
    }

    return true;
}
