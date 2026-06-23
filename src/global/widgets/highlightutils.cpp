// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "highlightutils.h"

#include <QPainter>
#include <QFontMetricsF>
#include <QRegularExpression>
#include <QPalette>
#include <QFont>
#include <algorithm>

namespace HighlightUtils {

static double charWidth(const QFontMetricsF &fm, QChar ch)
{
    return fm.horizontalAdvance(ch);
}

static double textWidth(const QFontMetricsF &fm, const QString &text)
{
    return fm.horizontalAdvance(text);
}

QVector<MatchRange> findMatchRanges(const QString &text, const QStringList &keywords)
{
    QVector<MatchRange> ranges;
    for (const QString &kw : keywords) {
        if (kw.isEmpty())
            continue;
        QRegularExpression re(QRegularExpression::escape(kw),
                              QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatchIterator it = re.globalMatch(text);
        while (it.hasNext()) {
            auto m = it.next();
            ranges.append({ static_cast<int>(m.capturedStart()),
                            static_cast<int>(m.capturedStart() + m.capturedLength()) });
        }
    }
    std::sort(ranges.begin(), ranges.end(),
              [](const MatchRange &a, const MatchRange &b) { return a.start < b.start; });
    QVector<MatchRange> merged;
    for (const auto &r : ranges) {
        if (!merged.isEmpty() && r.start <= merged.last().end)
            merged.last().end = std::max(merged.last().end, r.end);
        else
            merged.append(r);
    }
    return merged;
}

static ElideResult buildWindowElide(const QString &text, int keepStart, int keepEnd,
                                    int maxWidth, const QFontMetricsF &fm)
{
    ElideResult result;
    const QString ellipsis = QStringLiteral("…");
    double ellipsisWidth = charWidth(fm, QChar(0x2026));

    bool needLeft = (keepStart > 0);
    bool needRight = (keepEnd < text.length());
    int numEllipsis = (needLeft ? 1 : 0) + (needRight ? 1 : 0);
    double budget = maxWidth - numEllipsis * ellipsisWidth;
    if (budget <= 0) {
        result.text = ellipsis;
        result.origPositions = { -1 };
        return result;
    }

    int keepLen = keepEnd - keepStart;
    int trimLeft = 0, trimRight = 0;
    double w = textWidth(fm, text.mid(keepStart, keepLen));
    while (w > budget && keepLen > 0) {
        double lNext = charWidth(fm, text[keepStart + trimLeft]);
        double rNext = charWidth(fm, text[keepEnd - 1 - trimRight]);
        if (trimRight >= keepLen - trimLeft - 1) {
            w -= lNext;
            ++trimLeft;
        } else if (lNext >= rNext) {
            w -= lNext;
            ++trimLeft;
        } else {
            w -= rNext;
            ++trimRight;
        }
        keepLen = (keepEnd - trimRight) - (keepStart + trimLeft);
    }
    if (keepLen <= 0) {
        result.text = ellipsis;
        result.origPositions = { -1 };
        return result;
    }

    int actualStart = keepStart + trimLeft;
    int actualEnd = keepEnd - trimRight;
    QString kept = text.mid(actualStart, actualEnd - actualStart);

    result.text = (needLeft ? ellipsis : QString()) + kept + (needRight ? ellipsis : QString());
    int pos = 0;
    result.origPositions.resize(result.text.length());
    if (needLeft) {
        result.origPositions[pos++] = -1;
    }
    for (int i = actualStart; i < actualEnd; ++i)
        result.origPositions[pos++] = i;
    if (needRight)
        result.origPositions[pos] = -1;
    return result;
}

ElideResult smartElideWithTracking(const QString &text, int maxWidth,
                                   const QFontMetricsF &fm,
                                   const QVector<MatchRange> &matchRanges)
{
    const QString ellipsis = QStringLiteral("…");
    double ellipsisWidth = charWidth(fm, QChar(0x2026));

    if (textWidth(fm, text) <= maxWidth) {
        ElideResult result;
        result.text = text;
        result.origPositions.resize(text.length());
        for (int i = 0; i < text.length(); ++i)
            result.origPositions[i] = i;
        return result;
    }

    if (maxWidth <= ellipsisWidth) {
        ElideResult result;
        result.text = ellipsis;
        result.origPositions = { -1 };
        return result;
    }

    if (matchRanges.isEmpty())
        return {};

    // 以第一个匹配的关键词为锚点，并尽量让它显示在可见窗口的中间。
    // 初始窗口只包含第一个匹配，随后向两侧平衡扩展以填充可用宽度，
    // 避免总是先填满某一侧而把关键词挤到边缘。
    const MatchRange &first = matchRanges.constFirst();
    int keepStart = first.start;
    int keepEnd = first.end;

    double keptWidth = textWidth(fm, text.mid(keepStart, keepEnd - keepStart));
    double leftWidth = 0;    // 第一个匹配左侧已扩展的像素宽度
    double rightWidth = 0;   // 第一个匹配右侧已扩展的像素宽度

    while (true) {
        bool needLeft = (keepStart > 0);
        bool needRight = (keepEnd < text.length());
        int numEllipsis = (needLeft ? 1 : 0) + (needRight ? 1 : 0);
        double budget = maxWidth - numEllipsis * ellipsisWidth;

        if (keptWidth > budget)
            break;
        if (!needLeft && !needRight)
            break;

        // 向已累积像素宽度更窄的一侧扩展，使第一个匹配尽量居中。
        bool tryLeft;
        if (needLeft && needRight)
            tryLeft = (leftWidth <= rightWidth);
        else
            tryLeft = needLeft;

        // 向指定一侧扩展一个字符；放得下则更新窗口与累计像素宽度。
        auto tryExpand = [&](bool left) -> bool {
            if (left && keepStart > 0) {
                double cw = charWidth(fm, text[keepStart - 1]);
                int newNeedLeft = (keepStart - 1 > 0);
                int newNumEllipsis = (newNeedLeft ? 1 : 0) + (needRight ? 1 : 0);
                double newBudget = maxWidth - newNumEllipsis * ellipsisWidth;
                if (keptWidth + cw <= newBudget) {
                    --keepStart;
                    keptWidth += cw;
                    leftWidth += cw;
                    return true;
                }
            } else if (!left && keepEnd < text.length()) {
                double cw = charWidth(fm, text[keepEnd]);
                int newNeedRight = (keepEnd + 1 < text.length());
                int newNumEllipsis = (needLeft ? 1 : 0) + (newNeedRight ? 1 : 0);
                double newBudget = maxWidth - newNumEllipsis * ellipsisWidth;
                if (keptWidth + cw <= newBudget) {
                    ++keepEnd;
                    keptWidth += cw;
                    rightWidth += cw;
                    return true;
                }
            }
            return false;
        };

        // 首选侧放不下时回退到另一侧；两侧都放不下则结束扩展。
        bool expanded = tryExpand(tryLeft);
        if (!expanded)
            expanded = tryExpand(!tryLeft);
        if (!expanded)
            break;
    }

    return buildWindowElide(text, keepStart, keepEnd, maxWidth, fm);
}

ElideResult elideWithTracking(const QString &text, Qt::TextElideMode mode,
                              int maxWidth, const QFontMetricsF &fm)
{
    ElideResult result;
    if (text.isEmpty())
        return result;

    const QString ellipsis = QStringLiteral("…");
    double ellipsisWidth = charWidth(fm, QChar(0x2026));

    if (textWidth(fm, text) <= maxWidth) {
        result.text = text;
        result.origPositions.resize(text.length());
        for (int i = 0; i < text.length(); ++i)
            result.origPositions[i] = i;
        return result;
    }

    if (maxWidth <= ellipsisWidth) {
        result.text = ellipsis;
        result.origPositions = { -1 };
        return result;
    }

    double availWidth = maxWidth - ellipsisWidth;

    switch (mode) {
    case Qt::ElideRight: {
        double w = 0;
        int keep = 0;
        for (int i = 0; i < text.length(); ++i) {
            double cw = charWidth(fm, text[i]);
            if (w + cw > availWidth)
                break;
            w += cw;
            ++keep;
        }
        result.text = text.left(keep) + ellipsis;
        result.origPositions.resize(result.text.length());
        for (int i = 0; i < keep; ++i)
            result.origPositions[i] = i;
        result.origPositions[keep] = -1;
        break;
    }
    case Qt::ElideMiddle: {
        int total = text.length();
        int bestKeep = 0;
        for (int tryChars = total - 1; tryChars >= 1; --tryChars) {
            int head = tryChars / 2;
            int tail = tryChars - head;
            if (head < 0 || tail < 0 || head + tail > total)
                continue;
            double w = textWidth(fm, text.left(head)) + textWidth(fm, text.right(tail));
            if (w <= availWidth) {
                bestKeep = tryChars;
                break;
            }
        }
        if (bestKeep == 0) {
            result.text = ellipsis;
            result.origPositions = { -1 };
            break;
        }
        int head = bestKeep / 2;
        int tail = bestKeep - head;
        result.text = text.left(head) + ellipsis + text.right(tail);
        result.origPositions.resize(result.text.length());
        for (int i = 0; i < head; ++i)
            result.origPositions[i] = i;
        result.origPositions[head] = -1;
        for (int i = 0; i < tail; ++i)
            result.origPositions[head + 1 + i] = total - tail + i;
        break;
    }
    case Qt::ElideLeft: {
        double w = 0;
        int keep = 0;
        for (int i = text.length() - 1; i >= 0; --i) {
            double cw = charWidth(fm, text[i]);
            if (w + cw > availWidth)
                break;
            w += cw;
            ++keep;
        }
        result.text = ellipsis + text.right(keep);
        result.origPositions.resize(result.text.length());
        result.origPositions[0] = -1;
        for (int i = 0; i < keep; ++i)
            result.origPositions[1 + i] = text.length() - keep + i;
        break;
    }
    case Qt::ElideNone:
    default:
        result.text = text;
        result.origPositions.resize(text.length());
        for (int i = 0; i < text.length(); ++i)
            result.origPositions[i] = i;
        break;
    }

    return result;
}

QVector<QTextLayout::FormatRange> buildFormatRanges(
        const QString &displayText,
        const QVector<int> &origPosMapping,
        const QString &originalText,
        const QStringList &keywords,
        const QTextCharFormat &format)
{
    QVector<QTextLayout::FormatRange> ranges;

    if (keywords.isEmpty() || displayText.isEmpty())
        return ranges;

    if (origPosMapping.isEmpty()) {
        for (const QString &keyword : keywords) {
            if (keyword.isEmpty())
                continue;
            QRegularExpression re(QRegularExpression::escape(keyword),
                                  QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatchIterator it = re.globalMatch(displayText);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                QTextLayout::FormatRange range;
                range.start = match.capturedStart();
                range.length = match.capturedLength();
                range.format = format;
                ranges.append(range);
            }
        }
    } else {
        for (const QString &keyword : keywords) {
            if (keyword.isEmpty())
                continue;
            QRegularExpression re(QRegularExpression::escape(keyword),
                                  QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatchIterator it = re.globalMatch(originalText);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                int origStart = match.capturedStart();
                int origEnd = origStart + match.capturedLength();

                int rangeStart = -1;
                for (int i = 0; i < displayText.length(); ++i) {
                    int origPos = origPosMapping.value(i, -1);
                    bool inMatch = (origPos >= origStart && origPos < origEnd);
                    if (inMatch) {
                        if (rangeStart == -1)
                            rangeStart = i;
                    } else {
                        if (rangeStart != -1) {
                            QTextLayout::FormatRange range;
                            range.start = rangeStart;
                            range.length = i - rangeStart;
                            range.format = format;
                            ranges.append(range);
                            rangeStart = -1;
                        }
                    }
                }
                if (rangeStart != -1) {
                    QTextLayout::FormatRange range;
                    range.start = rangeStart;
                    range.length = displayText.length() - rangeStart;
                    range.format = format;
                    ranges.append(range);
                }
            }
        }
    }

    return ranges;
}

QTextCharFormat defaultHighlightFormat(const QFont &baseFont, const QPalette &palette)
{
    QTextCharFormat fmt;
    fmt.setFont(baseFont);
    fmt.setFontWeight(QFont::DemiBold);
    fmt.setForeground(palette.color(QPalette::BrightText));
    return fmt;
}

}   // namespace HighlightUtils
