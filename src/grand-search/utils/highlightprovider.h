// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HIGHLIGHTPROVIDER_H
#define HIGHLIGHTPROVIDER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QHash>
#include <QList>
#include <functional>
#include <atomic>

namespace GrandSearch {

/**
 * @brief 延迟高亮内容加载提供者
 *
 * 搜索结果展示时不同步获取高亮内容，而是通过此类在工作线程中异步获取。
 * 使用方式：
 * 1. 通过 setFetchCallback() 注入实际的 fetchHighlight 实现
 * 2. 通过 requestHighlight() 请求高亮内容
 * 3. 通过 highlightReady 信号获取异步结果
 * 4. 通过 cancelTask() 在搜索关键词改变时清理过期任务
 *
 * 参考自 dde-file-manager 的 dfmbase::HighlightProvider
 */
class HighlightProvider : public QObject
{
    Q_OBJECT

public:
    using FetchHighlightCallback = std::function<QString(const QString &path,
                                                         const QString &keyword,
                                                         int searchType)>;

    static HighlightProvider *instance();

    /**
     * @brief 注入实际的 fetchHighlight 实现
     * 回调函数将在工作线程中执行（同步阻塞 I/O）
     */
    void setFetchCallback(FetchHighlightCallback cb);

    /**
     * @brief 请求单个文件的高亮内容
     * @param taskId  搜索关键词（作为任务隔离标识，关键词改变时通过 cancelTask 清理）
     * @param path    文件路径
     * @param keyword 搜索关键词
     * @param searchType 搜索类型（对应 DFMSEARCH::SearchType 枚举值）
     * @param highPriority true 表示可见区域项，插入队列头部优先处理
     */
    void requestHighlight(const QString &taskId, const QString &path,
                          const QString &keyword, int searchType,
                          bool highPriority = false);

    /**
     * @brief 取消指定搜索会话的所有待处理请求并清除缓存
     * 在搜索关键词改变时调用，丢弃所有已过期的高亮任务
     */
    void cancelTask(const QString &taskId);

Q_SIGNALS:
    /**
     * @brief 高亮内容获取完成信号
     * @param taskId  搜索关键词
     * @param path    文件路径
     * @param content 高亮内容（可能为空表示无匹配内容）
     */
    void highlightReady(const QString &taskId, const QString &path, const QString &content);

private Q_SLOTS:
    void processNextRequest();

private:
    struct HighlightRequest
    {
        QString taskId;
        QString path;
        QString keyword;
        int searchType;
    };

    explicit HighlightProvider(QObject *parent = nullptr);
    ~HighlightProvider() override;

    FetchHighlightCallback m_fetchCallback;
    QThread *m_workerThread = nullptr;
    QObject *m_worker = nullptr;

    mutable QMutex m_requestMutex;
    QList<HighlightRequest> m_pendingRequests;

    std::atomic<int> m_processing { 0 };

    // 缓存：QHash<taskId(keyword), QHash<path, content>>
    // 不存在 = 未请求
    // "__pending__" = 请求中（去重哨兵）
    // 空 QString = 已获取但无高亮内容
    // 非空 = 已获取且有高亮内容
    QHash<QString, QHash<QString, QString>> m_cache;
    QMutex m_cacheMutex;
};

}   // namespace GrandSearch

#endif   // HIGHLIGHTPROVIDER_H
