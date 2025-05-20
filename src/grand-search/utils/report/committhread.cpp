#include "committhread.h"

#include <QLibrary>
#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

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

    // qCDebug(logGrandSearch) << "Committing event log data - Size:" << data.length();
    m_writeEventLog(data.toStdString());
}

bool CommitLog::init()
{
    QLibrary library("deepin-event-log");
    if (!library.load()) {
        qCWarning(logGrandSearch) << "Failed to load deepin-event-log library - Error:" << library.errorString();
        return false;
    }

    m_initEventLog = reinterpret_cast<InitEventLog>(library.resolve("Initialize"));
    m_writeEventLog = reinterpret_cast<WriteEventLog>(library.resolve("WriteEventLog"));

    if (!m_initEventLog || !m_writeEventLog) {
        qCWarning(logGrandSearch) << "Failed to resolve library functions - Initialize:" << (m_initEventLog ? "OK" : "Failed")
                                  << "WriteEventLog:" << (m_writeEventLog ? "OK" : "Failed");
        return false;
    }

    if (!m_initEventLog("uos-ai", false)) {
        qCWarning(logGrandSearch) << "Failed to initialize event log system - Application:uos-ai";
        return false;
    }

    return true;
}
