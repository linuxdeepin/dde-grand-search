// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/widgets/highlightlabel.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QFontMetrics>
#include <QRegularExpression>
#include <QEvent>

namespace {

struct ElideResult
{
    QString text;
    QVector<int> origPositions;   // text[i] -> original char position, -1 for ellipsis char
};

static int charWidth(const QFontMetrics &fm, QChar ch)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return fm.width(ch);
#else
    return fm.horizontalAdvance(ch);
#endif
}

static int textWidth(const QFontMetrics &fm, const QString &text)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return fm.width(text);
#else
    return fm.horizontalAdvance(text);
#endif
}

ElideResult elideWithTracking(const QString &text, Qt::TextElideMode mode,
                              int maxWidth, const QFontMetrics &fm)
{
    ElideResult result;
    if (text.isEmpty())
        return result;

    const QString ellipsis = QStringLiteral("…");
    int ellipsisWidth = charWidth(fm, QChar(0x2026));

    if (textWidth(fm, text) <= maxWidth) {
        // No eliding needed
        result.text = text;
        result.origPositions.resize(text.length());
        for (int i = 0; i < text.length(); ++i)
            result.origPositions[i] = i;
        return result;
    }

    if (maxWidth <= ellipsisWidth) {
        // Not enough room even for ellipsis alone
        result.text = ellipsis;
        result.origPositions = { -1 };
        return result;
    }

    int availWidth = maxWidth - ellipsisWidth;

    switch (mode) {
    case Qt::ElideRight: {
        int w = 0;
        int keep = 0;
        for (int i = 0; i < text.length(); ++i) {
            int cw = charWidth(fm, text[i]);
            if (w + cw > availWidth)
                break;
            w += cw;
            ++keep;
        }
        result.text = text.left(keep) + ellipsis;
        result.origPositions.resize(result.text.length());
        for (int i = 0; i < keep; ++i)
            result.origPositions[i] = i;
        result.origPositions[keep] = -1;   // ellipsis
        break;
    }
    case Qt::ElideMiddle: {
        // Binary-search style: try different head/tail splits from largest to smallest
        int total = text.length();
        int bestKeep = 0;
        for (int tryChars = total - 1; tryChars >= 1; --tryChars) {
            int head = tryChars / 2;
            int tail = tryChars - head;
            if (head < 0 || tail < 0 || head + tail > total)
                continue;
            int w = textWidth(fm, text.left(head)) + textWidth(fm, text.right(tail));
            if (w <= availWidth) {
                bestKeep = tryChars;
                break;
            }
        }
        if (bestKeep == 0) {
            // Nothing fits besides ellipsis
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
        result.origPositions[head] = -1;   // ellipsis
        for (int i = 0; i < tail; ++i)
            result.origPositions[head + 1 + i] = total - tail + i;
        break;
    }
    case Qt::ElideLeft: {
        int w = 0;
        int keep = 0;
        for (int i = text.length() - 1; i >= 0; --i) {
            int cw = charWidth(fm, text[i]);
            if (w + cw > availWidth)
                break;
            w += cw;
            ++keep;
        }
        result.text = ellipsis + text.right(keep);
        result.origPositions.resize(result.text.length());
        result.origPositions[0] = -1;   // ellipsis
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

}   // anonymous namespace

HighlightLabel::HighlightLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{
    setTextFormat(Qt::PlainText);
    m_textOption.setWrapMode(QTextOption::WrapAnywhere);
}

void HighlightLabel::setPlainText(const QString &text)
{
    if (m_text == text)
        return;
    m_text = text;
    relayout();
}

void HighlightLabel::setKeywords(const QStringList &keywords)
{
    if (m_keywords == keywords)
        return;
    m_keywords = keywords;
    relayout();
}

void HighlightLabel::setElideMode(Qt::TextElideMode mode)
{
    if (m_elideMode == mode)
        return;
    m_elideMode = mode;
    relayout();
}

void HighlightLabel::setMaxLines(int lines)
{
    if (m_maxLines == lines)
        return;
    m_maxLines = lines;
    relayout();
}

Qt::TextElideMode HighlightLabel::elideMode() const
{
    return m_elideMode;
}

int HighlightLabel::maxLines() const
{
    return m_maxLines;
}

bool HighlightLabel::isElided() const
{
    return m_elided;
}

QSize HighlightLabel::sizeHint() const
{
    if (m_text.isEmpty())
        return QSize(0, 0);
    return m_layout.boundingRect().size().toSize();
}

QSize HighlightLabel::minimumSizeHint() const
{
    return sizeHint();
}

void HighlightLabel::relayout()
{
    doLayout();
    update();
}

void HighlightLabel::doLayout()
{
    m_textOption.setAlignment(alignment());
    m_layout.clearLayout();
    m_elided = false;

    if (m_text.isEmpty()) {
        m_layout.setText("");
        updateGeometry();
        return;
    }

    int layoutWidth = contentsRect().width();
    if (layoutWidth <= 0) {
        m_layout.setText("");
        updateGeometry();
        return;
    }

    ElideInfo info = computeElidedText(layoutWidth);
    m_elided = info.elided;
    buildFinalLayout(info.displayText, info.origPosMapping, layoutWidth);
}

HighlightLabel::ElideInfo HighlightLabel::computeElidedText(int layoutWidth) const
{
    ElideInfo info;
    info.displayText = m_text;

    if (m_maxLines <= 0)
        return info;

    QTextLayout probeLayout(m_text, font());
    probeLayout.setTextOption(m_textOption);

    probeLayout.beginLayout();
    for (int i = 0; i < m_maxLines; ++i) {
        QTextLine line = probeLayout.createLine();
        if (!line.isValid())
            break;
        line.setLineWidth(layoutWidth);

        if (i == m_maxLines - 1) {
            int consumed = line.textStart() + line.textLength();
            if (consumed < m_text.length()) {
                info.elided = true;
                QFontMetrics fm(font());
                QString remaining = m_text.mid(line.textStart());
                // For multi-line text, use left elision on the last line to show the end
                Qt::TextElideMode lastLineElideMode = (m_maxLines > 1 && m_elideMode == Qt::ElideMiddle) ? Qt::ElideLeft : m_elideMode;
                ElideResult elideResult = elideWithTracking(remaining, lastLineElideMode, layoutWidth, fm);
                info.displayText = m_text.left(line.textStart()) + elideResult.text;

                // Build full position mapping: identity for head, elideResult mapping for tail
                info.origPosMapping.resize(info.displayText.length());
                int headLen = line.textStart();
                for (int j = 0; j < headLen; ++j)
                    info.origPosMapping[j] = j;
                for (int j = 0; j < elideResult.origPositions.size(); ++j) {
                    int origPos = elideResult.origPositions[j];
                    info.origPosMapping[headLen + j] = (origPos == -1) ? -1 : (line.textStart() + origPos);
                }
            }
        }
    }
    probeLayout.endLayout();

    return info;
}

void HighlightLabel::buildFinalLayout(const QString &displayText, const QVector<int> &origPosMapping, int layoutWidth)
{
    QVector<QTextLayout::FormatRange> formats = buildFormatRanges(displayText, origPosMapping);

    m_layout.clearLayout();
    m_layout.setText(displayText);
    m_layout.setFont(font());
    m_layout.setTextOption(m_textOption);
    m_layout.setFormats(formats);

    m_layout.beginLayout();
    int lineIndex = 0;
    qreal height = 0;
    QFontMetrics fm(font());
    qreal lineHeight = fm.height();

    while (true) {
        QTextLine line = m_layout.createLine();
        if (!line.isValid())
            break;
        line.setLineWidth(layoutWidth);
        line.setPosition(QPointF(0, height));
        height += lineHeight;
        ++lineIndex;
        if (m_maxLines > 0 && lineIndex >= m_maxLines)
            break;
    }
    m_layout.endLayout();

    updateGeometry();
}

QVector<QTextLayout::FormatRange> HighlightLabel::buildFormatRanges(
        const QString &displayText, const QVector<int> &origPosMapping) const
{
    QVector<QTextLayout::FormatRange> ranges;

    if (m_keywords.isEmpty() || displayText.isEmpty())
        return ranges;

    QTextCharFormat fmt;
    fmt.setFont(font());
    fmt.setFontWeight(QFont::DemiBold);
    fmt.setForeground(palette().color(QPalette::BrightText));

    if (origPosMapping.isEmpty()) {
        // Non-elided case: find matches directly in displayText
        for (const QString &keyword : m_keywords) {
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
                range.format = fmt;
                ranges.append(range);
            }
        }
    } else {
        // Elided case: find matches in original text, map to display positions
        for (const QString &keyword : m_keywords) {
            if (keyword.isEmpty())
                continue;
            QRegularExpression re(QRegularExpression::escape(keyword),
                                  QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatchIterator it = re.globalMatch(m_text);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                int origStart = match.capturedStart();
                int origEnd = origStart + match.capturedLength();

                // Scan display positions to find contiguous visible sub-ranges
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
                            range.format = fmt;
                            ranges.append(range);
                            rangeStart = -1;
                        }
                    }
                }
                // Close any trailing range
                if (rangeStart != -1) {
                    QTextLayout::FormatRange range;
                    range.start = rangeStart;
                    range.length = displayText.length() - rangeStart;
                    range.format = fmt;
                    ranges.append(range);
                }
            }
        }
    }

    return ranges;
}

void HighlightLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (m_text.isEmpty())
        return;

    QPainter painter(this);

    QRectF layoutRect = m_layout.boundingRect();
    Qt::Alignment align = alignment();

    // Vertical alignment
    qreal y = 0;
    Qt::Alignment vAlign = align & Qt::AlignVertical_Mask;
    if (vAlign & Qt::AlignBottom)
        y = height() - layoutRect.height();
    else if (vAlign & Qt::AlignVCenter)
        y = (height() - layoutRect.height()) / 2.0;

    // Draw the entire layout at the calculated position
    m_layout.draw(&painter, QPointF(0, y));
}

void HighlightLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    relayout();
}

void HighlightLabel::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange
        || event->type() == QEvent::PaletteChange) {
        relayout();
    }
    QLabel::changeEvent(event);
}
