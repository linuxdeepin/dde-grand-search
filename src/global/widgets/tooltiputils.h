// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLTIPUTILS_H
#define TOOLTIPUTILS_H

#include <QPoint>
#include <QRect>
#include <QString>

class QWidget;

namespace ToolTipUtils {

inline constexpr int kDefaultLineCharCount = 32;

// 将纯文本按固定字符数换行（已有的换行会保留）；lineCharCount <= 0 时不换行
QString wrapText(const QString &text, int lineCharCount = kDefaultLineCharCount);

// 显示换行后的 tooltip；rect 有效时作为 tooltip 的锚定区域
void showText(const QString &text, const QPoint &globalPos,
              QWidget *widget = nullptr, const QRect &rect = QRect());

// 立即关闭当前 tooltip
void hide();

// 为控件设置换行后的 tooltip；text 为空则清除
void setToolTip(QWidget *widget, const QString &text, int lineCharCount = kDefaultLineCharCount);

}   // namespace ToolTipUtils

#endif   // TOOLTIPUTILS_H
