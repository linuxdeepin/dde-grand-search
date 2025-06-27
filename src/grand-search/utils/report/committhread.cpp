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
    qCDebug(logGrandSearch) << "CommitLog created for event logging";
}

CommitLog::~CommitLog()
{
    qCDebug(logGrandSearch) << "CommitLog destroyed";
}

void CommitLog::commit(const QString &data)
{
    if (data.isEmpty()) {
        qCDebug(logGrandSearch) << "Empty data provided for event commit - Ignoring";
        return;
    }

    m_writeEventLog(data.toStdString());
    qCDebug(logGrandSearch) << "Event log data committed successfully";
}

bool CommitLog::init()
{
    qCDebug(logGrandSearch) << "Initializing event log system";

    QLibrary library("deepin-event-log");
    if (!library.load()) {
        qCWarning(logGrandSearch) << "Failed to load deepin-event-log library - Error:" << library.errorString();
        return false;
    }
    qCDebug(logGrandSearch) << "deepin-event-log library loaded successfully";

    m_initEventLog = reinterpret_cast<InitEventLog>(library.resolve("Initialize"));
    m_writeEventLog = reinterpret_cast<WriteEventLog>(library.resolve("WriteEventLog"));

    if (!m_initEventLog || !m_writeEventLog) {
        qCWarning(logGrandSearch) << "Failed to resolve library functions - Initialize:" << (m_initEventLog ? "OK" : "Failed")
                                  << "WriteEventLog:" << (m_writeEventLog ? "OK" : "Failed");
        return false;
    }
    qCDebug(logGrandSearch) << "Event log library functions resolved successfully";

    if (!m_initEventLog("uos-ai", false)) {
        qCWarning(logGrandSearch) << "Failed to initialize event log system - Application:uos-ai";
        return false;
    }

    qCInfo(logGrandSearch) << "Event log system initialized successfully - Application:uos-ai";
    return true;
}
