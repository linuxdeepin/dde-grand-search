// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGETHUMBNAILGENERATOR_H
#define IMAGETHUMBNAILGENERATOR_H

#include "../thumbnailgenerator.h"

namespace GrandSearch {

/**
 * @brief 图片缩略图生成器
 *
 * 支持所有 Qt 支持的图片格式，包括：
 * - 常见格式：JPEG, PNG, GIF, BMP, TIFF, WebP 等
 * - 处理大图片：按需缩放，避免内存溢出
 * - 处理 EXIF 方向信息
 */
class ImageThumbnailGenerator : public ThumbnailGenerator
{
public:
    ImageThumbnailGenerator();
    ~ImageThumbnailGenerator() override = default;

    bool canHandle(const QString &mimetype) const override;
    QPixmap generate(const QString &filePath, const QSize &size) override;
    int priority() const override { return 10; }
    QString name() const override { return QStringLiteral("ImageThumbnailGenerator"); }
    QStringList supportedMimeTypes() const override;
};

}   // namespace GrandSearch

#endif   // IMAGETHUMBNAILGENERATOR_H
