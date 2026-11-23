// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tooltiputils.h"

#include <QApplication>
#include <QStringList>
#include <QTextBoundaryFinder>
#include <QToolTip>
#include <QWidget>

namespace ToolTipUtils {

QString wrapText(const QString &text, int lineCharCount)
{
    if (text.isEmpty() || lineCharCount <= 0)
        return text;

    QString wrapped;
    const QStringList lines = text.split(QLatin1Char('\n'));
    for (int lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
        if (lineIndex > 0)
            wrapped.append(QLatin1Char('\n'));

        const QString &line = lines.at(lineIndex);
        QTextBoundaryFinder finder(QTextBoundaryFinder::Grapheme, line);
        int start = 0;
        while (start < line.length()) {
            int end = qMin(start + lineCharCount, line.length());
            if (end < line.length()) {
                // 按字素簇边界切分，避免把代理对（emoji 等）拆开
                finder.setPosition(end);
                if (!finder.isAtBoundary()) {
                    const int boundary = finder.toPreviousBoundary();
                    if (boundary > start) {
                        end = boundary;
                    } else {
                        // 单个字素簇比 lineCharCount 还长时，至少完整取一个字素簇
                        finder.setPosition(start);
                        const int next = finder.toNextBoundary();
                        if (next > start)
                            end = next;
                    }
                }
            }

            if (start > 0)
                wrapped.append(QLatin1Char('\n'));
            wrapped.append(line.mid(start, end - start));
            start = end;
        }
    }
    return wrapped;
}

void showText(const QString &text, const QPoint &globalPos, QWidget *widget, const QRect &rect)
{
    if (text.isEmpty())
        return;

    const QString wrapped = wrapText(text);
    if (rect.isValid())
        QToolTip::showText(globalPos, wrapped, widget, rect);
    else
        QToolTip::showText(globalPos, wrapped, widget);
}

void hide()
{
    // QToolTip::hideText() 不会立即销毁 QTipLabel（isVisible 仍为 true），
    // 这里沿用原列表 delegate 的做法，直接关闭 tooltip 顶层窗口。
    const QWidgetList widgets = QApplication::topLevelWidgets();
    for (QWidget *widget : widgets) {
        if (QStringLiteral("QTipLabel") == widget->metaObject()->className())
            widget->close();
    }
}

void setToolTip(QWidget *widget, const QString &text, int lineCharCount)
{
    if (!widget)
        return;
    widget->setToolTip(text.isEmpty() ? QString() : wrapText(text, lineCharCount));
}

}   // namespace ToolTipUtils
