// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filestatisticsthread.h"

#include <QDebug>
#include <QQueue>
#include <QFileInfo>
#include <QDirIterator>
#include <QTimer>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

using namespace GrandSearch;

FileStatisticsThread::FileStatisticsThread(QObject *parent)
    : QThread (parent)
{
    qCDebug(logGrandSearch) << "FileStatisticsThread created";

    m_notifyDataTimer = new QTimer();
    connect(m_notifyDataTimer, &QTimer::timeout, this, [this] {
        emit dataNotify(m_totalSize);
    }, Qt::DirectConnection);

    qCDebug(logGrandSearch) << "File statistics notification timer initialized - Interval: 500ms";
}

FileStatisticsThread::~FileStatisticsThread()
{
    qCDebug(logGrandSearch) << "FileStatisticsThread destructor - Cleaning up timer";

    if (m_notifyDataTimer) {
        m_notifyDataTimer->stop();
        m_notifyDataTimer->deleteLater();
        m_notifyDataTimer = nullptr;
    }
}

void FileStatisticsThread::start(const QString &sourceFile)
{
    if (isRunning()) {
        qCWarning(logGrandSearch) << "File statistics thread is already running - Source:" << m_sourceFile;
        return;
    }

    if (sourceFile.isEmpty()) {
        qCWarning(logGrandSearch) << "Source file path is empty - Cannot start statistics";
        return;
    }

    m_sourceFile = sourceFile;
    qCDebug(logGrandSearch) << "Starting file statistics thread - Source:" << sourceFile;
    QThread::start();
}

void FileStatisticsThread::stop()
{
    qCDebug(logGrandSearch) << "Stopping file statistics thread";
    setState(StoppedState);
}

void FileStatisticsThread::run()
{
    qCInfo(logGrandSearch) << "File statistics thread started - Processing:" << m_sourceFile;

    setState(RunningState);
    m_totalSize = 0;
    emit dataNotify(0);

    QQueue<QString> dirQueue;
    processFile(m_sourceFile, dirQueue);

    while (!dirQueue.isEmpty()) {
        const QString &dir = dirQueue.dequeue();
        QDirIterator iterator(dir, QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);

        while (iterator.hasNext()) {
            auto file = iterator.next();
            processFile(file, dirQueue);

            if (!stateCheck()) {
                setState(StoppedState);
                return;
            }
        }
    }

    setState(StoppedState);
}

void FileStatisticsThread::setState(FileStatisticsThread::State state)
{
    if (state == m_state)
        return;

    qCDebug(logGrandSearch) << "File statistics thread state changed from" << m_state << "to" << state;
    m_state = state;
    if (state == RunningState) {
        QMetaObject::invokeMethod(m_notifyDataTimer, "start", Q_ARG(int, 500));
    } else {
        QMetaObject::invokeMethod(m_notifyDataTimer, "stop");
        emit dataNotify(m_totalSize);
    }
}

bool FileStatisticsThread::stateCheck()
{
    return m_state == RunningState;
}

void FileStatisticsThread::processFile(const QString &file, QQueue<QString> &dirQueue)
{
    QFileInfo fileInfo(file);
    if (fileInfo.isDir()) {
        dirQueue << file;
    } else {
        if (fileInfo.isSymLink() && fileInfo.symLinkTarget() == QStringLiteral("/proc/kcore")) {
            qCDebug(logGrandSearch) << "Skipping /proc/kcore symlink";
            return;
        }

        qint64 size = fileInfo.size();
        if (size > 0)
            m_totalSize += size;
    }
}
