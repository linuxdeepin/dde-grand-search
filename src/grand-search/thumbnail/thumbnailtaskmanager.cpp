// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailtaskmanager.h"
#include "thumbnailcache.h"
#include "thumbnailgenerator.h"
#include "thumbnailprovider.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QHash>
#include <QMetaObject>
#include <QSet>
#include <QThread>

namespace GrandSearch {

ThumbnailTaskManager *ThumbnailTaskManager::s_instance = nullptr;

ThumbnailTaskManager::ThumbnailTaskManager(QObject *parent)
    : QObject(parent), m_threadPool(new QThreadPool(this))
{
    // 设置线程池最大线程数为 CPU 核心数
    m_threadPool->setMaxThreadCount(QThread::idealThreadCount());
}

ThumbnailTaskManager::~ThumbnailTaskManager()
{
}

ThumbnailTaskManager *ThumbnailTaskManager::instance()
{
    if (!s_instance) {
        s_instance = new ThumbnailTaskManager();
    }
    return s_instance;
}

QString ThumbnailTaskManager::generateTaskId(const QString &filePath, const ThumbnailSize &size)
{
    const auto &sz = ThumbnailCache::enumToSize(size);
    QString key = QString("%1_%2x%3").arg(filePath).arg(sz.width()).arg(sz.height());
    return QString::fromLatin1(QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Md5).toHex());
}

QString ThumbnailTaskManager::submit(const QString &filePath, const QString &mimetype,
                                     const ThumbnailSize &size, int priority)
{
    QMutexLocker locker(&m_mutex);

    QString taskId = generateTaskId(filePath, size);

    // 检查是否已有相同任务
    if (m_pendingTasks.contains(taskId)) {
        return taskId;
    }

    // 检查缓存是否已存在
    if (ThumbnailCache::instance()->exists(filePath, size)) {
        QPixmap cached = ThumbnailCache::instance()->get(filePath, size);
        if (!cached.isNull()) {
            // 异步发送信号，避免死锁
            QMetaObject::invokeMethod(
                    this, [this, filePath, cached]() {
                        emit thumbnailReady(filePath, cached);
                    },
                    Qt::QueuedConnection);
        }
        return taskId;
    }

    // 创建任务
    ThumbnailTask task(filePath, mimetype, size, priority);
    m_tasks.insert(taskId, task);
    m_pendingTasks.insert(taskId);

    // 创建工作线程并提交
    ThumbnailWorker *worker = new ThumbnailWorker(task, this);
    worker->setAutoDelete(true);
    m_threadPool->start(worker, priority);

    return taskId;
}

void ThumbnailTaskManager::cancel(const QString &taskId)
{
    QMutexLocker locker(&m_mutex);

    if (m_tasks.contains(taskId)) {
        m_tasks[taskId].canceled = true;
        m_pendingTasks.remove(taskId);
    }
}

void ThumbnailTaskManager::cancelByFilePath(const QString &filePath)
{
    QMutexLocker locker(&m_mutex);

    for (auto it = m_tasks.begin(); it != m_tasks.end(); ++it) {
        if (it->filePath == filePath) {
            it->canceled = true;
            m_pendingTasks.remove(it.key());
        }
    }
}

void ThumbnailTaskManager::cancelAll()
{
    QMutexLocker locker(&m_mutex);

    for (auto it = m_tasks.begin(); it != m_tasks.end(); ++it) {
        it->canceled = true;
    }
    m_pendingTasks.clear();
}

void ThumbnailTaskManager::setMaxThreadCount(int maxThreads)
{
    m_threadPool->setMaxThreadCount(maxThreads);
}

int ThumbnailTaskManager::activeTaskCount() const
{
    return m_threadPool->activeThreadCount();
}

void ThumbnailTaskManager::shutdown()
{
    cancelAll();

    {
        QMutexLocker locker(&m_mutex);
        m_tasks.clear();
        m_taskQueue.clear();
    }

    // 等待线程池完成所有任务
    m_threadPool->waitForDone();
}

// ========== ThumbnailWorker 实现 ==========

ThumbnailWorker::ThumbnailWorker(const ThumbnailTask &task, QObject *receiver)
    : m_task(task), m_receiver(receiver)
{
}

void ThumbnailWorker::run()
{
    if (m_task.canceled) {
        return;
    }

    // 检查缓存
    QPixmap cached = ThumbnailCache::instance()->get(m_task.filePath, m_task.size);
    if (!cached.isNull()) {
        QMetaObject::invokeMethod(
                m_receiver, [this, cached]() {
                    ThumbnailTaskManager::instance()->thumbnailReady(m_task.filePath, cached);
                },
                Qt::QueuedConnection);
        return;
    }

    // 使用 ThumbnailProvider 同步生成缩略图
    QPixmap thumbnail = ThumbnailProvider::instance()->getThumbnailSync(
            m_task.filePath, m_task.mimetype, m_task.size);

    if (m_task.canceled) {
        return;   // 任务已取消，不发送结果
    }

    const QString &filePath = m_task.filePath;
    if (!thumbnail.isNull()) {
        QMetaObject::invokeMethod(
                m_receiver, [filePath, thumbnail]() {
                    ThumbnailTaskManager::instance()->thumbnailReady(filePath, thumbnail);
                },
                Qt::QueuedConnection);
    } else {
        QMetaObject::invokeMethod(
                m_receiver, [filePath]() {
                    ThumbnailTaskManager::instance()->thumbnailFailed(filePath);
                },
                Qt::QueuedConnection);
    }
}

}   // namespace GrandSearch
