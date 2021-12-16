/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "filestatisticsthread.h"

#include <QDebug>
#include <QQueue>
#include <QFileInfo>
#include <QDirIterator>
#include <QTimer>

FileStatisticsThread::FileStatisticsThread(QObject *parent)
    : QThread (parent)
{
    m_notifyDataTimer = new QTimer();
    connect(m_notifyDataTimer, &QTimer::timeout, this, [this] {
        emit dataNotify(m_totalSize);
    }, Qt::DirectConnection);
}

FileStatisticsThread::~FileStatisticsThread()
{
    if (m_notifyDataTimer) {
        m_notifyDataTimer->stop();
        m_notifyDataTimer->deleteLater();
        m_notifyDataTimer = nullptr;
    }
}

void FileStatisticsThread::start(const QString &sourceFile)
{
    if (isRunning()) {
        qWarning() << "current thread is running.";
        return;
    }

    if (sourceFile.isEmpty())
        return;

    m_sourceFile = sourceFile;
    QThread::start();
}

void FileStatisticsThread::stop()
{
    setState(StoppedState);
}

void FileStatisticsThread::run()
{
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
        if (fileInfo.isSymLink() && fileInfo.symLinkTarget() == QStringLiteral("/proc/kcore"))
            return;

        qint64 size = fileInfo.size();
        if (size > 0)
            m_totalSize += size;
    }
}
