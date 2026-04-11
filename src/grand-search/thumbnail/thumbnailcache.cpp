// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailcache.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPixmap>
#include <QStandardPaths>

namespace GrandSearch {

ThumbnailCache *ThumbnailCache::s_instance = nullptr;

ThumbnailCache::ThumbnailCache(QObject *parent)
    : QObject(parent), m_maxMemoryCacheItems(100)   // 默认缓存 100 个缩略图
{
    // 设置缓存目录：遵循 XDG 标准
    QString cacheLocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    m_cacheDir = cacheLocation + "/thumbnails";

    ensureCacheDirExists();

    // 设置内存缓存最大条目数
    m_memoryCache.setMaxCost(m_maxMemoryCacheItems);
}

ThumbnailCache::~ThumbnailCache()
{
    clearMemoryCache();
}

ThumbnailCache *ThumbnailCache::instance()
{
    if (!s_instance) {
        s_instance = new ThumbnailCache();
    }
    return s_instance;
}

QString ThumbnailCache::cacheKey(const QString &filePath, const QSize &size)
{
    // 使用文件路径 + 尺寸生成唯一键
    QString key = QString("%1_%2x%3").arg(filePath).arg(size.width()).arg(size.height());
    return QString::fromLatin1(QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Md5).toHex());
}

QString ThumbnailCache::cacheFilePath(const QString &filePath, const QSize &size)
{
    QString key = cacheKey(filePath, size);
    return m_cacheDir + "/" + key + ".png";
}

void ThumbnailCache::ensureCacheDirExists()
{
    QDir dir(m_cacheDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
}

QPixmap ThumbnailCache::get(const QString &filePath, const QSize &size)
{
    QMutexLocker locker(&m_mutex);

    QString key = cacheKey(filePath, size);

    // 1. 先检查内存缓存
    if (m_memoryCache.contains(key)) {
        return *m_memoryCache.object(key);
    }

    // 2. 检查磁盘缓存
    QString diskPath = cacheFilePath(filePath, size);
    if (QFile::exists(diskPath) && !isCacheExpired(filePath, diskPath)) {
        QPixmap pixmap(diskPath);
        if (!pixmap.isNull()) {
            // 存入内存缓存
            m_memoryCache.insert(key, new QPixmap(pixmap));
            return pixmap;
        }
    }

    return QPixmap();
}

void ThumbnailCache::put(const QString &filePath, const QSize &size, const QPixmap &thumbnail)
{
    if (thumbnail.isNull()) {
        return;
    }

    QMutexLocker locker(&m_mutex);

    QString key = cacheKey(filePath, size);

    // 1. 存入内存缓存
    m_memoryCache.insert(key, new QPixmap(thumbnail));

    // 2. 存入磁盘缓存
    QString diskPath = cacheFilePath(filePath, size);
    thumbnail.save(diskPath, "PNG");
}

bool ThumbnailCache::exists(const QString &filePath, const QSize &size)
{
    QMutexLocker locker(&m_mutex);

    QString key = cacheKey(filePath, size);

    // 检查内存缓存
    if (m_memoryCache.contains(key)) {
        return true;
    }

    // 检查磁盘缓存
    QString diskPath = cacheFilePath(filePath, size);
    return QFile::exists(diskPath) && !isCacheExpired(filePath, diskPath);
}

bool ThumbnailCache::isCacheExpired(const QString &filePath, const QString &cachePath)
{
    QFileInfo sourceFile(filePath);
    QFileInfo cacheFile(cachePath);

    if (!sourceFile.exists() || !cacheFile.exists()) {
        return true;
    }

    // 如果源文件修改时间晚于缓存文件，则缓存过期
    return sourceFile.lastModified() > cacheFile.lastModified();
}

void ThumbnailCache::clearMemoryCache()
{
    QMutexLocker locker(&m_mutex);
    m_memoryCache.clear();
}

void ThumbnailCache::clearDiskCache()
{
    QMutexLocker locker(&m_mutex);

    QDir dir(m_cacheDir);
    if (dir.exists()) {
        QStringList files = dir.entryList(QDir::Files);
        for (const QString &file : files) {
            dir.remove(file);
        }
    }
}

void ThumbnailCache::setMaxMemoryCacheItems(int maxItems)
{
    QMutexLocker locker(&m_mutex);
    m_maxMemoryCacheItems = maxItems;
    m_memoryCache.setMaxCost(maxItems);
}

}   // namespace GrandSearch
