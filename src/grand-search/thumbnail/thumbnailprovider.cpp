// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailprovider.h"
#include "thumbnail.h"
#include "thumbnailcache.h"
#include "thumbnailtaskmanager.h"

#include <algorithm>
#include <QDebug>
#include <QMutexLocker>

namespace GrandSearch {

ThumbnailProvider *ThumbnailProvider::s_instance = nullptr;

ThumbnailProvider::ThumbnailProvider(QObject *parent)
    : QObject(parent)
{
    // 连接任务管理器的信号
    connect(ThumbnailTaskManager::instance(), &ThumbnailTaskManager::thumbnailReady,
            this, &ThumbnailProvider::onThumbnailReady);
    connect(ThumbnailTaskManager::instance(), &ThumbnailTaskManager::thumbnailFailed,
            this, &ThumbnailProvider::onThumbnailFailed);
}

ThumbnailProvider::~ThumbnailProvider()
{
}

ThumbnailProvider *ThumbnailProvider::instance()
{
    if (!s_instance) {
        s_instance = new ThumbnailProvider();
    }
    return s_instance;
}

void ThumbnailProvider::registerGenerator(ThumbnailGenerator *generator)
{
    if (!generator) {
        return;
    }

    QMutexLocker locker(&m_mutex);

    // 检查是否已注册
    if (m_generators.contains(generator)) {
        return;
    }

    m_generators.append(generator);

    // 按优先级排序（优先级高的在前面）
    std::sort(m_generators.begin(), m_generators.end(),
              [](ThumbnailGenerator *a, ThumbnailGenerator *b) {
                  return a->priority() > b->priority();
              });

    qDebug() << "ThumbnailProvider: registered generator" << generator->name()
             << "with priority" << generator->priority();
}

void ThumbnailProvider::unregisterGenerator(ThumbnailGenerator *generator)
{
    if (!generator) {
        return;
    }

    QMutexLocker locker(&m_mutex);
    m_generators.removeOne(generator);

    qDebug() << "ThumbnailProvider: unregistered generator" << generator->name();
}

ThumbnailGenerator *ThumbnailProvider::findGenerator(const QString &mimetype) const
{
    QMutexLocker locker(&m_mutex);

    for (ThumbnailGenerator *generator : m_generators) {
        if (generator->canHandle(mimetype)) {
            return generator;
        }
    }

    return nullptr;
}

void ThumbnailProvider::requestThumbnail(const QString &filePath, const QString &mimetype,
                                         const ThumbnailSize &size, int priority)
{
    // 检查是否有生成器支持
    if (!isSupported(mimetype)) {
        emit thumbnailFailed(filePath);
        return;
    }

    // 先检查缓存
    QPixmap cached = ThumbnailCache::instance()->get(filePath, size);
    if (!cached.isNull()) {
        emit thumbnailReady(filePath, cached);
        return;
    }

    ThumbnailTaskManager::instance()->submit(filePath, mimetype, size, priority);
}

QPixmap ThumbnailProvider::getThumbnailSync(const QString &filePath, const QString &mimetype,
                                            const ThumbnailSize &size)
{
    // 先检查缓存
    QPixmap cached = ThumbnailCache::instance()->get(filePath, size);
    if (!cached.isNull()) {
        return cached;
    }

    // 查找生成器
    ThumbnailGenerator *generator = findGenerator(mimetype);
    if (!generator) {
        qWarning() << "ThumbnailProvider: no generator for mimetype" << mimetype;
        return QPixmap();
    }

    // 同步生成缩略图（将 ThumbnailSize 转换为 QSize）
    QSize qsize = ThumbnailCache::enumToSize(size);
    QPixmap thumbnail = generator->generate(filePath, qsize);

    if (!thumbnail.isNull()) {
        // 存入缓存
        ThumbnailCache::instance()->put(filePath, size, thumbnail);
    }

    return thumbnail;
}

void ThumbnailProvider::cancelRequest(const QString &filePath)
{
    ThumbnailTaskManager::instance()->cancelByFilePath(filePath);
}

bool ThumbnailProvider::isSupported(const QString &mimetype) const
{
    return findGenerator(mimetype) != nullptr;
}

QStringList ThumbnailProvider::supportedMimeTypes() const
{
    QMutexLocker locker(&m_mutex);

    QStringList result;
    for (ThumbnailGenerator *generator : m_generators) {
        result.append(generator->supportedMimeTypes());
    }
    return result;
}

void ThumbnailProvider::onThumbnailReady(const QString &filePath, const QPixmap &thumbnail)
{
    emit thumbnailReady(filePath, thumbnail);
}

void ThumbnailProvider::onThumbnailFailed(const QString &filePath)
{
    emit thumbnailFailed(filePath);
}

}   // namespace GrandSearch
