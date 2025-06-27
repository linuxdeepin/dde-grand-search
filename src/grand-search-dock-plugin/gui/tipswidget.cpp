// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tipswidget.h"

#include <QPainter>
#include <QAccessible>
#include <QTextDocument>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDock)

using namespace GrandSearch;

TipsWidget::TipsWidget(QWidget *parent)
    : QFrame(parent)
    , m_type(SingleLine)
{
    qCDebug(logDock) << "TipsWidget initialized";
}

void TipsWidget::setText(const QString &text)
{
    qCDebug(logDock) << "Setting TipsWidget text - Original text:" << text;

    m_type = TipsWidget::SingleLine;
    // 如果传递的是富文本，获取富文本中的纯文本内容进行显示
    QTextDocument document;
    document.setHtml(text);
    // 同时去掉两边的空白信息，例如qBittorrent的提示
    m_text = document.toPlainText().simplified();

    qCDebug(logDock) << "TipsWidget text processed - Final text:" << m_text;

#if 0   //测试时可以使用下面的语句
    // FIXME:藏语字体绘制会有异常，设置高度时需要使用fontMetrics().boundingRect()去获取整体的边界矩形的高度，
    // 使用fontMetrics().height()去获取时，针对藏语这种字体，其高度和实际显示区域并不等同
    m_text = "བོད་སྐད་ཡིག་གཟུགས་ཚད་ལེན་ཚོད་ལྟའི་སྐོར་གྱི་རྗོད་ཚིག";
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    setFixedSize(fontMetrics().horizontalAdvance(m_text), fontMetrics().boundingRect(m_text).height());
#else
    setFixedSize(fontMetrics().width(m_text), fontMetrics().height());
#endif

    update();

#ifndef QT_NO_ACCESSIBILITY
    if (accessibleName().isEmpty()) {
        QAccessibleEvent event(this, QAccessible::NameChanged);
        QAccessible::updateAccessibility(&event);
        qCDebug(logDock) << "TipsWidget accessibility event triggered";
    }
#endif
}

void TipsWidget::setTextList(const QStringList &textList)
{
    qCDebug(logDock) << "Setting TipsWidget text list - Count:" << textList.size() << "Texts:" << textList;

    m_type = TipsWidget::MultiLine;
    m_textList = textList;

    int width = 0;
    int height = 0;
    for (QString text : m_textList) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        width = qMax(width, fontMetrics().horizontalAdvance(text));
        height += fontMetrics().boundingRect(text).height();
#else
        width = qMax(width, fontMetrics().width(text));
        height += fontMetrics().height();
#endif
    }

    setFixedSize(width, height);
    qCDebug(logDock) << "TipsWidget multi-line size calculated - Width:" << width << "Height:" << height;

    update();
}

/**
 * @brief TipsWidget::paintEvent 任务栏插件提示信息绘制
 * @param event
 */
void TipsWidget::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    QPainter painter(this);
    painter.setPen(QPen(palette().brightText(), 1));

    QTextOption option;
    option.setAlignment(Qt::AlignCenter);

    switch (m_type) {
    case SingleLine:
        qCDebug(logDock) << "Painting TipsWidget single line text:" << m_text;
        painter.drawText(rect(), m_text, option);
        break;
    case MultiLine:
        qCDebug(logDock) << "Painting TipsWidget multi-line text - Count:" << m_textList.size();
        int y= 0;
        if (m_textList.size() != 1)
            option.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        for (QString text : m_textList) {
            int lineHeight = fontMetrics().boundingRect(text).height();
            painter.drawText(QRect(0, y, rect().width(), lineHeight), text, option);
            y += lineHeight;
        }
        break;
    }
}

bool TipsWidget::event(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        qCDebug(logDock) << "TipsWidget font change event detected";
        switch (m_type) {
        case SingleLine:
            setText(m_text);
            qCDebug(logDock) << "TipsWidget single line text reset after font change";
            break;
        case MultiLine:
            setTextList(m_textList);
            qCDebug(logDock) << "TipsWidget multi-line text reset after font change";
            break;
        }
    }
    return QFrame::event(event);
}
