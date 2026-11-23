// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "highlightlabel.h"
#include "highlightutils.h"
#include "tooltiputils.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QFontMetrics>
#include <QEvent>
#include <QtMath>

namespace {
// QTextLayout 的实际排版宽度可能略大于字体度量，按实际排版结果回退宽度
constexpr int kMaxLayoutRetries = 8;
constexpr int kMaxOverhangRetries = 3;
}   // namespace

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

void HighlightLabel::setElideMode(ElideMode mode)
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

HighlightLabel::ElideMode HighlightLabel::elideMode() const
{
    return m_elideMode;
}

int HighlightLabel::maxLines() const
{
    return m_maxLines;
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
        ToolTipUtils::setToolTip(this, QString());
        updateGeometry();
        return;
    }

    int layoutWidth = contentsRect().width();
    if (layoutWidth <= 0) {
        m_layout.setText("");
        ToolTipUtils::setToolTip(this, QString());
        updateGeometry();
        return;
    }

    // 省略宽度按字体度量计算，但最终由 QTextLayout 排版；在高亮加粗、
    // 字体回退、字形右侧溢出等情况下，两者仍可能有细微差异，导致末尾
    // 字符被挤到未创建的行。按实际排版结果收窄末行省略宽度，直到全部
    // 显示字符都能排入现有行。
    ElideInfo info;
    int extraReserved = 0;
    // 每次最多回退半个行高，通常 1-2 次即可收敛
    const int retryStep = qMax(2, qCeil(QFontMetricsF(font()).height() / 2));
    const int maxRetries = (m_elideMode == ElideMode::None) ? 1 : kMaxLayoutRetries;
    for (int retry = 0; retry < maxRetries; ++retry) {
        info = computeElidedText(layoutWidth, extraReserved);
        buildFinalLayout(info.displayText, info.origPosMapping, layoutWidth);

        int covered = 0;
        for (int i = 0; i < m_layout.lineCount(); ++i) {
            const QTextLine line = m_layout.lineAt(i);
            covered = qMax(covered, line.textStart() + line.textLength());
        }
        if (covered >= info.displayText.length())
            break;
        extraReserved += retryStep;
    }
    m_elided = info.elided;
    ToolTipUtils::setToolTip(this, m_elided ? m_text : QString());
}

HighlightLabel::ElideInfo HighlightLabel::computeElidedText(int layoutWidth, int extraReserved) const
{
    ElideInfo info;
    info.displayText = m_text;

    if (m_maxLines <= 0)
        return info;

    QTextLayout probeLayout(m_text, font());
    probeLayout.setTextOption(m_textOption);
    // 让探测排版也带上关键词高亮格式，避免按常规字宽判断换行、
    // 实际绘制时因加粗变宽而把末尾字符挤到未创建的行。
    probeLayout.setFormats(buildFormatRanges(m_text));

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
                QString remaining = m_text.mid(line.textStart());

                // 关键词命中时，末行可见字符会以 DemiBold 绘制，字宽比常规字体大。
                // 末行省略按加粗后的字宽计算，保证最终排版不会丢掉末尾字符。
                // 注意：关键词可能匹配到原始文本更早的位置（例如关键词就是完整
                // 文件名），不能用 remaining 单独匹配，否则会漏判。
                QFont elideFont = font();
                const auto allRanges = HighlightUtils::findMatchRanges(m_text, m_keywords);
                for (const auto &range : allRanges) {
                    if (range.end > line.textStart()) {
                        elideFont = HighlightUtils::highlightFont(elideFont);
                        break;
                    }
                }
                QFontMetrics fm(elideFont);

                auto elideLastLine = [&](int width) {
                    HighlightUtils::ElideResult result;
                    if (m_elideMode == ElideMode::Smart) {
                        ElideInfo smartInfo = computeSmartElidedText(remaining, width, fm);
                        result.text = smartInfo.displayText;
                        result.origPositions = smartInfo.origPosMapping;
                        info.elided = smartInfo.elided;
                    } else {
                        auto qtMode = static_cast<Qt::TextElideMode>(m_elideMode);
                        Qt::TextElideMode lastLineElideMode =
                                (m_maxLines > 1 && qtMode == Qt::ElideMiddle) ? Qt::ElideLeft : qtMode;
                        result = HighlightUtils::elideWithTracking(remaining, lastLineElideMode, width, fm);
                    }
                    return result;
                };

                int elideWidth = qMax(1, layoutWidth - extraReserved);
                HighlightUtils::ElideResult elideResult = elideLastLine(elideWidth);

                // QTextLayout 的 naturalTextWidth 会包含最后一个字形超出 advance 的
                // 右侧墨迹，而 QFontMetricsF 只按 advance 计算。若最后一个字形有
                // 右侧溢出，按原宽度省略会被 QTextLayout 挤到下一行而丢弃
                // （maxLines 限制下不可见），因此预留该宽度后重新省略。
                QFontMetricsF fmf(elideFont);
                int reservedWidth = 0;
                for (int retry = 0; retry < kMaxOverhangRetries && !elideResult.text.isEmpty(); ++retry) {
                    const qreal overhang = qMax<qreal>(0.0, -fmf.rightBearing(elideResult.text.back()));
                    const int needed = overhang > 0 ? qCeil(overhang) + 1 : 0;
                    if (needed <= reservedWidth)
                        break;
                    reservedWidth = needed;
                    elideWidth = qMax(1, layoutWidth - extraReserved - reservedWidth);
                    elideResult = elideLastLine(elideWidth);
                }

                info.displayText = m_text.left(line.textStart()) + elideResult.text;

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

HighlightLabel::ElideInfo HighlightLabel::computeSmartElidedText(
        const QString &text, int maxWidth, const QFontMetrics &fm) const
{
    ElideInfo info;
    info.displayText = text;

    if (text.isEmpty() || m_keywords.isEmpty())
        return info;

    if (HighlightUtils::elideWithTracking(text, Qt::ElideRight, maxWidth, fm).text == text)
        return info;

    auto matchRanges = HighlightUtils::findMatchRanges(text, m_keywords);
    if (matchRanges.isEmpty()) {
        HighlightUtils::ElideResult fallback = HighlightUtils::elideWithTracking(text, Qt::ElideRight, maxWidth, fm);
        info.displayText = fallback.text;
        info.origPosMapping = fallback.origPositions;
        info.elided = true;
        return info;
    }

    HighlightUtils::ElideResult windowResult = HighlightUtils::smartElideWithTracking(text, maxWidth, fm, matchRanges);
    if (windowResult.text.isEmpty()) {
        HighlightUtils::ElideResult fallback = HighlightUtils::elideWithTracking(text, Qt::ElideRight, maxWidth, fm);
        info.displayText = fallback.text;
        info.origPosMapping = fallback.origPositions;
        info.elided = true;
        return info;
    }

    info.displayText = windowResult.text;
    info.origPosMapping = windowResult.origPositions;
    info.elided = true;
    return info;
}

QVector<QTextLayout::FormatRange> HighlightLabel::buildFormatRanges(
        const QString &displayText, const QVector<int> &origPosMapping) const
{
    QTextCharFormat fmt = HighlightUtils::defaultHighlightFormat(font(), palette());
    return HighlightUtils::buildFormatRanges(displayText, origPosMapping, m_text, m_keywords, fmt);
}

void HighlightLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (m_text.isEmpty())
        return;

    QPainter painter(this);

    QRectF layoutRect = m_layout.boundingRect();
    Qt::Alignment align = alignment();

    qreal y = 0;
    Qt::Alignment vAlign = align & Qt::AlignVertical_Mask;
    if (vAlign & Qt::AlignBottom)
        y = height() - layoutRect.height();
    else if (vAlign & Qt::AlignVCenter)
        y = (height() - layoutRect.height()) / 2.0;

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
