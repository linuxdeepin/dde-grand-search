// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTTHUMBNAILGENERATOR_H
#define TEXTTHUMBNAILGENERATOR_H

#include "../thumbnailgenerator.h"

#include <QFont>

namespace GrandSearch {

/**
 * @brief 文本文件缩略图生成器
 *
 * 支持从文本文件中提取内容并渲染为缩略图：
 * - 读取文件前 N 行内容
 * - 使用 QTextDocument 进行文本布局和渲染
 * - 支持常见文本格式：纯文本、JSON、XML、Markdown 等
 */
class TextThumbnailGenerator : public ThumbnailGenerator
{
public:
    TextThumbnailGenerator();
    ~TextThumbnailGenerator() override = default;

    bool canHandle(const QString &mimetype) const override;
    QPixmap generate(const QString &filePath, const QSize &size) override;
    int priority() const override { return 3; }
    QString name() const override { return QStringLiteral("TextThumbnailGenerator"); }
    QStringList supportedMimeTypes() const override;

    /**
     * @brief 设置最大读取行数
     * @param lines 最大行数，默认 20 行
     */
    void setMaxLines(int lines);

    /**
     * @brief 设置字体
     * @param font 字体
     */
    void setFont(const QFont &font);

private:
    /**
     * @brief 读取文件内容
     * @param filePath 文件路径
     * @param maxLines 最大行数
     * @return 文件内容
     */
    QString readFileContent(const QString &filePath, int maxLines) const;

    /**
     * @brief 渲染文本为图片
     * @param text 文本内容
     * @param size 目标尺寸
     * @return 渲染后的图片
     */
    QImage renderText(const QString &text, const QSize &size) const;

private:
    int m_maxLines;   // 最大读取行数
    QFont m_font;   // 渲染字体
};

}   // namespace GrandSearch

#endif   // TEXTTHUMBNAILGENERATOR_H
