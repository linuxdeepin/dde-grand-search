// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILCACHE_H
#define THUMBNAILCACHE_H

#include <QCache>
#include <QMutex>
#include <QObject>
#include <QPixmap>
#include <QSize>
#include <QString>

namespace GrandSearch {

/**
 * @brief 缩略图缓存管理器
 *
 * 实现两级缓存机制：
 * 1. 内存缓存：使用 QCache 存储最近使用的缩略图
 * 2. 磁盘缓存：遵循 XDG 标准，存储在用户缓存目录
 *
 * 缓存文件命名使用文件路径的 MD5 哈希 + 尺寸后缀。
 */
class ThumbnailCache : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     * @return ThumbnailCache 实例指针
     */
    static ThumbnailCache *instance();

    /**
     * @brief 从缓存获取缩略图
     * @param filePath 文件路径
     * @param size 缩略图尺寸
     * @return 缓存的缩略图，如果不存在返回空 QPixmap
     */
    QPixmap get(const QString &filePath, const QSize &size);

    /**
     * @brief 将缩略图存入缓存
     * @param filePath 文件路径
     * @param size 缩略图尺寸
     * @param thumbnail 缩略图
     */
    void put(const QString &filePath, const QSize &size, const QPixmap &thumbnail);

    /**
     * @brief 检查缓存是否存在
     * @param filePath 文件路径
     * @param size 缩略图尺寸
     * @return 如果缓存存在返回 true，否则返回 false
     */
    bool exists(const QString &filePath, const QSize &size);

    /**
     * @brief 清除内存缓存
     */
    void clearMemoryCache();

    /**
     * @brief 清除磁盘缓存
     */
    void clearDiskCache();

    /**
     * @brief 设置内存缓存最大条目数
     * @param maxItems 最大条目数
     */
    void setMaxMemoryCacheItems(int maxItems);

    /**
     * @brief 获取缓存文件路径
     * @param filePath 原始文件路径
     * @param size 缩略图尺寸
     * @return 缓存文件的完整路径
     */
    QString cacheFilePath(const QString &filePath, const QSize &size);

    /**
     * @brief 检查缓存是否过期（基于文件修改时间）
     * @param filePath 原始文件路径
     * @param cachePath 缓存文件路径
     * @return 如果缓存过期返回 true，否则返回 false
     */
    bool isCacheExpired(const QString &filePath, const QString &cachePath);

private:
    explicit ThumbnailCache(QObject *parent = nullptr);
    ~ThumbnailCache() override;

    // 禁止拷贝
    ThumbnailCache(const ThumbnailCache &) = delete;
    ThumbnailCache &operator=(const ThumbnailCache &) = delete;

    /**
     * @brief 生成缓存键
     * @param filePath 文件路径
     * @param size 缩略图尺寸
     * @return 缓存键
     */
    QString cacheKey(const QString &filePath, const QSize &size);

    /**
     * @brief 确保缓存目录存在
     */
    void ensureCacheDirExists();

private:
    static ThumbnailCache *s_instance;

    QCache<QString, QPixmap> m_memoryCache;   // 内存缓存
    QMutex m_mutex;   // 线程安全锁
    QString m_cacheDir;   // 磁盘缓存目录
    int m_maxMemoryCacheItems;   // 内存缓存最大条目数
};

}   // namespace GrandSearch

#endif   // THUMBNAILCACHE_H
