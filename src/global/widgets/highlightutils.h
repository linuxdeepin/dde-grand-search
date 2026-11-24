// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HIGHLIGHTUTILS_H
#define HIGHLIGHTUTILS_H

#include <QFont>
#include <QFontMetricsF>
#include <QTextLayout>
#include <QString>
#include <QStringList>
#include <QVector>

class QTextCharFormat;

namespace HighlightUtils {

// 关键词高亮统一使用的字重
inline constexpr QFont::Weight kHighlightFontWeight = QFont::DemiBold;

struct MatchRange
{
    int start;
    int end;   // exclusive
};

struct ElideResult
{
    QString text;
    QVector<int> origPositions;   // text[i] -> original char position, -1 for ellipsis char
};

// Find all keyword match ranges in text (case-insensitive, merged, sorted)
QVector<MatchRange> findMatchRanges(const QString &text, const QStringList &keywords);

// Standard elide with position tracking
ElideResult elideWithTracking(const QString &text, Qt::TextElideMode mode,
                              int maxWidth, const QFontMetricsF &fm);

// Smart elide that keeps keyword matches visible
ElideResult smartElideWithTracking(const QString &text, int maxWidth,
                                   const QFontMetricsF &fm,
                                   const QVector<MatchRange> &matchRanges);

// Build QTextLayout format ranges for keyword highlighting
// If origPosMapping is empty, matches are found directly in displayText
// Otherwise, matches are found in originalText and mapped to display positions via origPosMapping
QVector<QTextLayout::FormatRange> buildFormatRanges(
        const QString &displayText,
        const QVector<int> &origPosMapping,
        const QString &originalText,
        const QStringList &keywords,
        const QTextCharFormat &format);

// Convenience: create a default highlight format (DemiBold + BrightText foreground)
QTextCharFormat defaultHighlightFormat(const QFont &baseFont, const QPalette &palette);

// 基于基础字体构造高亮字体（仅改变字重），用于按实际绘制字宽做省略
QFont highlightFont(const QFont &baseFont);
}   // namespace HighlightUtils

#endif   // HIGHLIGHTUTILS_H
