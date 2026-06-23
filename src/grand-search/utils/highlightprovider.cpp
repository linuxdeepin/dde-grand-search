// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "highlightprovider.h"

#include <dfm-search/contentretriever.h>
#include <dfm-search/dsearch_global.h>

#include <QMutexLocker>
#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

namespace GrandSearch {

static const QString kPendingToken = QStringLiteral("__pending__");

HighlightProvider::HighlightProvider(QObject *parent)
    : QObject(parent)
{
    // 在工作线程启动前设置回调，避免主线程/工作线程数据竞争
    m_fetchCallback = [](const QString &path, const QString &keyword, int searchType) -> QString {
        thread_local DFMSEARCH::ContentRetriever retriever;
        DFMSEARCH::HighlightOptions opt;
        opt.setMaxPreviewLength(200);
        opt.setPositioningMaxLength(80);
        return retriever.fetchHighlight(path, keyword,
                                        static_cast<DFMSEARCH::SearchType>(searchType),
                                        opt);
    };

    m_workerThread = new QThread(this);
    m_worker = new QObject();
    m_worker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    m_workerThread->start();
}

HighlightProvider::~HighlightProvider()
{
    m_workerThread->quit();
    m_workerThread->wait(5000);
}

HighlightProvider *HighlightProvider::instance()
{
    static HighlightProvider ins;
    return &ins;
}

void HighlightProvider::setFetchCallback(FetchHighlightCallback cb)
{
    m_fetchCallback = std::move(cb);
}

void HighlightProvider::requestHighlight(const QString &taskId, const QString &path,
                                         const QString &keyword, int searchType,
                                         bool highPriority)
{
    if (!m_fetchCallback) {
        qCWarning(logGrandSearch) << "HighlightProvider: fetchCallback not set";
        return;
    }

    // Step 1: 检查缓存
    {
        QMutexLocker lk(&m_cacheMutex);
        auto &taskCache = m_cache[taskId];
        auto it = taskCache.find(path);

        if (it != taskCache.end()) {
            const QString &cached = it.value();
            if (cached != kPendingToken) {
                if (cached.isEmpty())
                    return;   // 已获取但无内容，避免无意义的 UI 更新

                // 缓存命中，直接发射信号
                lk.unlock();
                Q_EMIT highlightReady(taskId, path, cached);
                return;
            }
            // 标记为 pending，请求正在进行中，不重复提交
            return;
        }

        // 标记为 pending
        taskCache[path] = kPendingToken;
    }

    // Step 2: 加入请求队列
    {
        QMutexLocker lk(&m_requestMutex);

        if (highPriority) {
            m_pendingRequests.prepend({ taskId, path, keyword, searchType });
        } else {
            m_pendingRequests.append({ taskId, path, keyword, searchType });
        }

        // 仅在 idle -> busy 时投递一次任务，避免重复唤醒 worker
        int expected = 0;
        if (m_processing.compare_exchange_strong(expected, 1, std::memory_order_acquire)) {
            QMetaObject::invokeMethod(
                    m_worker, [this]() {
                        processNextRequest();
                    },
                    Qt::QueuedConnection);
        }
    }
}

void HighlightProvider::cancelTask(const QString &taskId)
{
    qCDebug(logGrandSearch) << "HighlightProvider: canceling task for keyword:" << taskId;

    // 从队列中移除所有该 taskId 的请求
    {
        QMutexLocker lk(&m_requestMutex);
        m_pendingRequests.erase(
                std::remove_if(m_pendingRequests.begin(), m_pendingRequests.end(),
                               [&taskId](const HighlightRequest &req) {
                                   return req.taskId == taskId;
                               }),
                m_pendingRequests.end());
    }

    // 清除缓存
    {
        QMutexLocker lk(&m_cacheMutex);
        m_cache.remove(taskId);
    }
}

void HighlightProvider::processNextRequest()
{
    while (true) {
        HighlightRequest req;

        {
            QMutexLocker lk(&m_requestMutex);
            if (m_pendingRequests.isEmpty()) {
                m_processing.store(0, std::memory_order_release);
                // 双重检查：防止在 isEmpty 检查与 store 之间新增了请求
                if (!m_pendingRequests.isEmpty()) {
                    int expected = 0;
                    if (m_processing.compare_exchange_strong(expected, 1, std::memory_order_acquire)) {
                        continue;
                    }
                }
                return;
            }
            req = m_pendingRequests.takeFirst();
        }

        // 检查该 taskId 是否已被取消
        {
            QMutexLocker lk(&m_cacheMutex);
            if (!m_cache.contains(req.taskId)) {
                continue;   // 会话已被取消，跳过
            }
        }

        // 执行实际的 highlight 获取（同步阻塞 I/O，在工作线程中执行）
        QString content;
        if (m_fetchCallback) {
            content = m_fetchCallback(req.path, req.keyword, req.searchType);
            // 去除首尾的省略符号
            if (content.startsWith("…"))
                content = content.mid(1);
            if (content.endsWith("…"))
                content.chop(1);
        }

        // 存储到缓存（如果会话在 fetch 期间被取消，则跳过）
        {
            QMutexLocker lk(&m_cacheMutex);
            if (!m_cache.contains(req.taskId)) {
                continue;   // 会话在 fetch 期间被 cancel，跳过存储和通知
            }
            m_cache[req.taskId][req.path] = content;
        }

        if (content.isEmpty())
            continue;

        // 通知主线程（跨线程信号自动 QueuedConnection）
        Q_EMIT highlightReady(req.taskId, req.path, content);
    }
}

}   // namespace GrandSearch
