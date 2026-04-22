// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textthumbnailgenerator.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QImage>
#include <QPainter>
#include <QTextDocument>

namespace GrandSearch {

TextThumbnailGenerator::TextThumbnailGenerator()
    : m_maxLines(20), m_font("Monospace", 10)   // 默认使用等宽字体
{
    // 文本文件大小限制：1 MB
    m_maxFileSize = 1 * 1024 * 1024;
}

bool TextThumbnailGenerator::canHandle(const QString &mimetype) const
{
    // 支持文本类型和常见的文本格式
    if (mimetype.startsWith("text/")) {
        return true;
    }

    // 支持一些常见的文本格式
    static const QStringList textMimes = {
        "application/json",
        "application/xml",
        "application/javascript",
        "application/x-yaml",
        "application/x-sh",
        "application/x-shellscript",
        "application/x-python-code",
        "application/x-ruby-script",
        "application/x-perl",
        "application/x-php"
    };

    return textMimes.contains(mimetype);
}

QPixmap TextThumbnailGenerator::generate(const QString &filePath, const QSize &size)
{
    if (filePath.isEmpty() || size.isEmpty()) {
        return QPixmap();
    }

    // 检查文件大小
    if (!isFileSizeValid(filePath)) {
        qWarning() << "TextThumbnailGenerator: file too large" << filePath;
        return QPixmap();
    }

    // 读取文件内容
    QString content = readFileContent(filePath, m_maxLines);
    if (content.isEmpty()) {
        return QPixmap();
    }

    // 渲染文本
    QImage image = renderText(content, size);
    if (image.isNull()) {
        return QPixmap();
    }

    return QPixmap::fromImage(image);
}

QStringList TextThumbnailGenerator::supportedMimeTypes() const
{
    return {
        "text/plain",
        "text/html",
        "text/css",
        "text/javascript",
        "text/xml",
        "text/x-c",
        "text/x-c++",
        "text/x-java",
        "text/x-python",
        "text/x-ruby",
        "text/x-perl",
        "text/x-php",
        "text/x-shellscript",
        "text/markdown",
        "application/json",
        "application/xml",
        "application/javascript"
    };
}

void TextThumbnailGenerator::setMaxLines(int lines)
{
    m_maxLines = qMax(1, lines);
}

void TextThumbnailGenerator::setFont(const QFont &font)
{
    m_font = font;
}

QString TextThumbnailGenerator::readFileContent(const QString &filePath, int maxLines) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "TextThumbnailGenerator: cannot open file" << filePath;
        return QString();
    }

    QString content;
    int lineCount = 0;

    while (!file.atEnd() && lineCount < maxLines) {
        QByteArray line = file.readLine();
        content += QString::fromUtf8(line);
        lineCount++;
    }

    // 如果还有更多内容，添加省略号
    if (!file.atEnd()) {
        content += "...\n";
    }

    file.close();
    return content;
}

QImage TextThumbnailGenerator::renderText(const QString &text, const QSize &size) const
{
    // 创建图像
    QImage image(size, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    // 设置字体
    painter.setFont(m_font);

    // 创建文本文档
    QTextDocument doc;
    doc.setPlainText(text);
    doc.setDefaultFont(m_font);
    doc.setTextWidth(size.width() - 10);   // 留出边距

    // 绘制文本
    painter.translate(5, 5);   // 边距
    doc.drawContents(&painter, QRectF(0, 0, size.width() - 10, size.height() - 10));

    painter.end();

    return image;
}

}   // namespace GrandSearch
