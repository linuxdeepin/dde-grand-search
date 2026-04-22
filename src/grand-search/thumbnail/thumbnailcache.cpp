// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailcache.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QPixmap>
#include <QStandardPaths>
#include <QUrl>

namespace GrandSearch {

// PNG 元数据键名（与文管保持一致）
namespace Thumb {
constexpr const char *URL = "Thumb::URL";
constexpr const char *MTime = "Thumb::MTime";
constexpr const char *Path = "Thumb::Path";
}   // namespace Thumb

ThumbnailCache *ThumbnailCache::s_instance = nullptr;

ThumbnailCache::ThumbnailCache(QObject *parent)
    : QObject(parent), m_maxMemoryCacheItems(100)   // 默认缓存 100 个缩略图
{
    // 设置缓存目录：使用文管的标准路径 ~/.cache/thumbnails
    m_cacheDir = QDir::homePath() + "/.cache/thumbnails";

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

QString ThumbnailCache::cacheKey(const QString &filePath, const ThumbnailSize &size)
{
    // 使用文件 URL 的 MD5 作为缓存键（与文管保持一致）
    QString fileUrl = filePathToUrl(filePath);
    return QString::fromLatin1(QCryptographicHash::hash(fileUrl.toUtf8(), QCryptographicHash::Md5).toHex());
}

QString ThumbnailCache::cacheFilePath(const QString &filePath, const ThumbnailSize &size)
{
    QString key = cacheKey(filePath, size);
    QString dirPath = sizeToDirPath(size);
    return dirPath + "/" + key + ".png";
}

void ThumbnailCache::ensureCacheDirExists()
{
    // 确保所有尺寸的缓存目录都存在
    QDir dir(m_cacheDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // 创建子目录：small, normal, large, x-large
    QStringList subDirs = { "small", "normal", "large", "x-large" };
    for (const QString &subDir : subDirs) {
        QString path = m_cacheDir + "/" + subDir;
        QDir sub(path);
        if (!sub.exists()) {
            sub.mkpath(".");
        }
    }
}

QPixmap ThumbnailCache::get(const QString &filePath, const ThumbnailSize &size)
{
    QMutexLocker locker(&m_mutex);

    QString key = cacheKey(filePath, size);

    // 1. 先检查内存缓存
    if (m_memoryCache.contains(key)) {
        return *m_memoryCache.object(key);
    }

    // 2. 检查磁盘缓存
    QString diskPath = cacheFilePath(filePath, size);
    if (QFile::exists(diskPath)) {
        // 读取图片并验证元数据
        QImageReader reader(diskPath, "PNG");
        if (!reader.canRead()) {
            return QPixmap();
        }

        QImage image = reader.read();
        if (image.isNull()) {
            return QPixmap();
        }

        // 验证元数据中的 MTime
        bool cacheValid = !isCacheExpired(filePath, diskPath, image);

        if (cacheValid) {
            QPixmap pixmap = QPixmap::fromImage(image);
            if (!pixmap.isNull()) {
                // 存入内存缓存
                m_memoryCache.insert(key, new QPixmap(pixmap));
                return pixmap;
            }
        }
    }

    return QPixmap();
}

void ThumbnailCache::put(const QString &filePath, const ThumbnailSize &size, const QPixmap &thumbnail)
{
    if (thumbnail.isNull()) {
        return;
    }

    QMutexLocker locker(&m_mutex);

    QString key = cacheKey(filePath, size);

    // 1. 存入内存缓存
    m_memoryCache.insert(key, new QPixmap(thumbnail));

    // 2. 存入磁盘缓存（添加元数据）
    QString diskPath = cacheFilePath(filePath, size);

    // 获取文件修改时间
    QFileInfo fileInfo(filePath);
    qint64 mtime = fileInfo.lastModified().toSecsSinceEpoch();

    // 转换为 QImage 以便添加元数据
    QImage image = thumbnail.toImage();
    QString fileUrl = filePathToUrl(filePath);

    // 添加元数据（与文管保持一致）
    image.setText(Thumb::URL, fileUrl);
    image.setText(Thumb::MTime, QString::number(mtime));

    // 保存为 PNG
    image.save(diskPath, "PNG");
}

bool ThumbnailCache::exists(const QString &filePath, const ThumbnailSize &size)
{
    QMutexLocker locker(&m_mutex);
    QString key = cacheKey(filePath, size);

    // 检查内存缓存
    if (m_memoryCache.contains(key)) {
        return true;
    }

    // 检查磁盘缓存
    QString diskPath = cacheFilePath(filePath, size);
    if (QFile::exists(diskPath)) {
        // 读取并验证元数据
        QImageReader reader(diskPath, "PNG");
        if (reader.canRead()) {
            QImage image = reader.read();
            if (!image.isNull()) {
                return !isCacheExpired(filePath, diskPath, image);
            }
        }
    }

    return false;
}

bool ThumbnailCache::isCacheExpired(const QString &filePath, const QString &cachePath)
{
    // 兼容旧版本调用，读取图片后调用新版本
    QImageReader reader(cachePath, "PNG");
    if (!reader.canRead()) {
        return true;
    }

    QImage image = reader.read();
    return isCacheExpired(filePath, cachePath, image);
}

bool ThumbnailCache::isCacheExpired(const QString &filePath, const QString &cachePath, const QImage &cacheImage)
{
    QFileInfo sourceFile(filePath);

    if (!sourceFile.exists()) {
        return true;
    }

    // 从元数据中获取 MTime
    QString mtimeStr = cacheImage.text(Thumb::MTime);
    if (mtimeStr.isEmpty()) {
        // 没有元数据的缓存文件无效，需要重新生成
        return true;
    }

    // 使用元数据中的 MTime 进行验证
    qint64 cachedMTime = mtimeStr.toLongLong();
    qint64 sourceMTime = sourceFile.lastModified().toSecsSinceEpoch();

    // 如果源文件修改时间晚于缓存时间，则缓存过期
    return sourceMTime > cachedMTime;
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
        // 清除所有子目录中的缓存文件
        QStringList subDirs = { "small", "normal", "large", "x-large" };
        for (const QString &subDir : subDirs) {
            QString subPath = m_cacheDir + "/" + subDir;
            QDir subDirObj(subPath);
            if (subDirObj.exists()) {
                QStringList files = subDirObj.entryList(QDir::Files);
                for (const QString &file : files) {
                    subDirObj.remove(file);
                }
            }
        }
    }
}

void ThumbnailCache::setMaxMemoryCacheItems(int maxItems)
{
    QMutexLocker locker(&m_mutex);
    m_maxMemoryCacheItems = maxItems;
    m_memoryCache.setMaxCost(maxItems);
}

ThumbnailSize ThumbnailCache::sizeToEnum(const QSize &size)
{
    int maxDim = qMax(size.width(), size.height());

    if (maxDim <= 128) {
        return ThumbnailSize::Small;
    } else if (maxDim <= 256) {
        return ThumbnailSize::Normal;
    } else if (maxDim <= 512) {
        return ThumbnailSize::Large;
    } else {
        return ThumbnailSize::XLarge;
    }
}

QSize ThumbnailCache::enumToSize(ThumbnailSize size)
{
    switch (size) {
    case ThumbnailSize::Small:
        return QSize(128, 128);
    case ThumbnailSize::Normal:
        return QSize(256, 256);
    case ThumbnailSize::Large:
        return QSize(512, 512);
    case ThumbnailSize::XLarge:
        return QSize(1024, 1024);
    default:
        return QSize(256, 256);
    }
}

QString ThumbnailCache::sizeToDirPath(ThumbnailSize size) const
{
    switch (size) {
    case ThumbnailSize::Small:
        return m_cacheDir + "/small";
    case ThumbnailSize::Normal:
        return m_cacheDir + "/normal";
    case ThumbnailSize::Large:
        return m_cacheDir + "/large";
    case ThumbnailSize::XLarge:
        return m_cacheDir + "/x-large";
    default:
        return m_cacheDir + "/normal";
    }
}

QString ThumbnailCache::filePathToUrl(const QString &filePath) const
{
    // 将文件路径转换为 file:// URL 格式
    QUrl url = QUrl::fromLocalFile(filePath);
    return url.toString(QUrl::FullyEncoded);
}

}   // namespace GrandSearch
