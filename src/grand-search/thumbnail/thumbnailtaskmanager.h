// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILTASKMANAGER_H
#define THUMBNAILTASKMANAGER_H

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QPixmap>
#include <QQueue>
#include <QRunnable>
#include <QSet>
#include <QSize>
#include <QString>
#include <QThreadPool>

namespace GrandSearch {

/**
 * @brief 缩略图任务结构
 */
struct ThumbnailTask
{
    QString filePath;   // 文件路径
    QString mimetype;   // MIME 类型
    QSize size;   // 目标尺寸
    int priority;   // 优先级（数值越大优先级越高）
    bool canceled;   // 是否已取消

    ThumbnailTask()
        : priority(0), canceled(false)
    {
    }

    ThumbnailTask(const QString &path, const QString &mime, const QSize &s, int prio = 0)
        : filePath(path), mimetype(mime), size(s), priority(prio), canceled(false)
    {
    }

    bool operator<(const ThumbnailTask &other) const
    {
        return priority < other.priority;   // 优先级高的排在前面
    }
};

/**
 * @brief 缩略图任务管理器
 *
 * 使用线程池管理缩略图生成任务，支持：
 * - 任务优先级队列
 * - 任务去重
 * - 任务取消
 * - 完成通知
 */
class ThumbnailTaskManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     * @return ThumbnailTaskManager 实例指针
     */
    static ThumbnailTaskManager *instance();

    /**
     * @brief 提交缩略图生成任务
     * @param filePath 文件路径
     * @param mimetype MIME 类型
     * @param size 目标尺寸
     * @param priority 优先级（默认为 0）
     * @return 任务 ID（用于取消任务）
     */
    QString submit(const QString &filePath, const QString &mimetype,
                   const QSize &size, int priority = 0);

    /**
     * @brief 取消指定任务
     * @param taskId 任务 ID
     */
    void cancel(const QString &taskId);

    /**
     * @brief 取消指定文件的所有任务
     * @param filePath 文件路径
     */
    void cancelByFilePath(const QString &filePath);

    /**
     * @brief 取消所有任务
     */
    void cancelAll();

    /**
     * @brief 设置最大线程数
     * @param maxThreads 最大线程数
     */
    void setMaxThreadCount(int maxThreads);

    /**
     * @brief 获取当前活动任务数
     * @return 活动任务数
     */
    int activeTaskCount() const;

    /**
     * @brief 关闭任务管理器，释放资源
     *
     * 取消所有待处理任务并等待线程池完成。
     * 应在不再需要缩略图服务时主动调用。
     */
    void shutdown();

signals:
    /**
     * @brief 缩略图生成完成信号
     * @param filePath 文件路径
     * @param thumbnail 生成的缩略图
     */
    void thumbnailReady(const QString &filePath, const QPixmap &thumbnail);

    /**
     * @brief 缩略图生成失败信号
     * @param filePath 文件路径
     */
    void thumbnailFailed(const QString &filePath);

private:
    explicit ThumbnailTaskManager(QObject *parent = nullptr);
    ~ThumbnailTaskManager() override;

    // 禁止拷贝
    ThumbnailTaskManager(const ThumbnailTaskManager &) = delete;
    ThumbnailTaskManager &operator=(const ThumbnailTaskManager &) = delete;

    /**
     * @brief 生成任务 ID
     * @param filePath 文件路径
     * @param size 目标尺寸
     * @return 任务 ID
     */
    QString generateTaskId(const QString &filePath, const QSize &size);

private:
    static ThumbnailTaskManager *s_instance;

    QThreadPool *m_threadPool;   // 线程池
    QQueue<ThumbnailTask> m_taskQueue;   // 任务队列
    QSet<QString> m_pendingTasks;   // 待处理任务 ID 集合（用于去重）
    QHash<QString, ThumbnailTask> m_tasks;   // 任务 ID -> 任务映射
    QMutex m_mutex;   // 线程安全锁
};

/**
 * @brief 缩略图生成工作类（QRunnable）
 */
class ThumbnailWorker : public QRunnable
{
public:
    ThumbnailWorker(const ThumbnailTask &task, QObject *receiver);
    void run() override;

private:
    ThumbnailTask m_task;
    QObject *m_receiver;
};

}   // namespace GrandSearch

#endif   // THUMBNAILTASKMANAGER_H
