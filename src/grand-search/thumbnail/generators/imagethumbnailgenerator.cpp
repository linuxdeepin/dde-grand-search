// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagethumbnailgenerator.h"

#include <QDebug>
#include <QImageReader>

inline constexpr qint64 DEFAULT_MAX_IMAGE_SIZE = 100 * 1024 * 1024;   // 100 MB (图片)

namespace GrandSearch {

ImageThumbnailGenerator::ImageThumbnailGenerator()
{
    // 图片文件大小限制：100 MB
    m_maxFileSize = DEFAULT_MAX_IMAGE_SIZE;
}

bool ImageThumbnailGenerator::canHandle(const QString &mimetype) const
{
    // 支持所有 image/* 类型的 MIME
    return mimetype.startsWith("image/");
}

QPixmap ImageThumbnailGenerator::generate(const QString &filePath, const QSize &size)
{
    if (filePath.isEmpty() || size.isEmpty()) {
        return QPixmap();
    }

    // 检查文件大小
    if (!isFileSizeValid(filePath)) {
        qWarning() << "ImageThumbnailGenerator: file too large" << filePath;
        return QPixmap();
    }

    QImageReader reader(filePath);

    if (!reader.canRead()) {
        qWarning() << "ImageThumbnailGenerator: cannot read image" << filePath;
        return QPixmap();
    }

    // 设置缩放尺寸，避免加载完整图片导致内存溢出
    QSize originalSize = reader.size();
    if (originalSize.isValid()) {
        QSize scaledSize = originalSize;
        scaledSize.scale(size, Qt::KeepAspectRatio);
        reader.setScaledSize(scaledSize);
    }

    QImage image = reader.read();
    if (image.isNull()) {
        qWarning() << "ImageThumbnailGenerator: failed to read image" << filePath
                   << "error:" << reader.errorString();
        return QPixmap();
    }

    // 如果图片尺寸仍然大于目标尺寸，进行二次缩放
    if (image.width() > size.width() || image.height() > size.height()) {
        image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    return QPixmap::fromImage(image);
}

QStringList ImageThumbnailGenerator::supportedMimeTypes() const
{
    return {
        "image/jpeg",
        "image/png",
        "image/gif",
        "image/bmp",
        "image/tiff",
        "image/webp",
        "image/svg+xml",
        "image/x-icon",
        "image/x-xpixmap",
        "image/x-xbitmap"
    };
}

}   // namespace GrandSearch
