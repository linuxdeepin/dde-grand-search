// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILGENERATOR_H
#define THUMBNAILGENERATOR_H

#include <QPixmap>
#include <QSize>
#include <QString>

inline constexpr qint64 DEFAULT_MAX_FILE_SIZE = 50 * 1024 * 1024;   // 50 MB

namespace GrandSearch {

/**
 * @brief 缩略图生成器基类
 *
 * 所有缩略图生成器都需要继承此类并实现相关接口。
 * 通过 mimetype 匹配机制，支持不同文件类型的缩略图生成。
 */
class ThumbnailGenerator
{
public:
    virtual ~ThumbnailGenerator() = default;

    /**
     * @brief 检查是否可以处理指定的 mimetype
     * @param mimetype 文件的 MIME 类型
     * @return 如果可以处理返回 true，否则返回 false
     */
    virtual bool canHandle(const QString &mimetype) const = 0;

    /**
     * @brief 生成缩略图
     * @param filePath 文件路径
     * @param size 目标缩略图尺寸
     * @return 生成的缩略图，如果生成失败返回空 QPixmap
     */
    virtual QPixmap generate(const QString &filePath, const QSize &size) = 0;

    /**
     * @brief 获取生成器优先级
     * @return 优先级值，数值越大优先级越高
     *
     * 当多个生成器可以处理同一 mimetype 时，优先使用优先级高的生成器。
     */
    virtual int priority() const { return 0; }

    /**
     * @brief 获取生成器名称（用于调试和日志）
     * @return 生成器名称
     */
    virtual QString name() const = 0;

    /**
     * @brief 获取支持的 mimetype 列表
     * @return 支持的 mimetype 列表
     *
     * 用于快速匹配，避免对每个 mimetype 都调用 canHandle。
     */
    virtual QStringList supportedMimeTypes() const = 0;

    /**
     * @brief 获取最大文件大小限制
     * @return 最大文件大小（字节），超过此大小的文件不生成缩略图
     */
    qint64 maxFileSize() const { return m_maxFileSize; }

    /**
     * @brief 设置最大文件大小限制
     * @param size 最大文件大小（字节）
     */
    void setMaxFileSize(qint64 size) { m_maxFileSize = size; }

    /**
     * @brief 检查文件大小是否在限制范围内
     * @param filePath 文件路径
     * @return 如果文件大小在限制范围内返回 true，否则返回 false
     */
    bool isFileSizeValid(const QString &filePath) const;

protected:
    qint64 m_maxFileSize = DEFAULT_MAX_FILE_SIZE;   // 最大文件大小限制
};

}   // namespace GrandSearch

#endif   // THUMBNAILGENERATOR_H
