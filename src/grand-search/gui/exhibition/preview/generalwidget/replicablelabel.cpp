/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "replicablelabel.h"
#include "global/accessibility/acintelfunctions.h"

#include <DGuiApplicationHelper>
#include <DStyle>

#include <QMouseEvent>
#include <QResizeEvent>

using namespace GrandSearch;
DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

const static int LineEditHorizontalMargin = 2;

ReplicableLabel::ReplicableLabel(QWidget *parent)
    : QLineEdit(parent)
{
    this->setStyleSheet("QLineEdit{background:transparent;border-width:0;border-style:outset;}");

    setReadOnly(true);
    setContextMenuPolicy(Qt::NoContextMenu);
}

void ReplicableLabel::setText(const QString &text)
{
    m_fullText = text;
}

void ReplicableLabel::setElideMode(Qt::TextElideMode elideMode)
{
    m_elideMode = elideMode;
}

Qt::TextElideMode ReplicableLabel::elideMode() const
{
    return m_elideMode;
}

bool ReplicableLabel::event(QEvent *event)
{
    if (event->type() == QEvent::FocusOut) {
        m_pressed = false;

        QLineEdit::setText(m_elideText);

        update();
    }

    return QLineEdit::event(event);
}

void ReplicableLabel::mousePressEvent(QMouseEvent *event)
{
    m_pressed = true;
    setFocus();

    return QLineEdit::mousePressEvent(event);
}

void ReplicableLabel::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;

    return QLineEdit::mouseReleaseEvent(event);
}

void ReplicableLabel::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pressed) {
        m_pressed = false;

        QLineEdit::setText(m_fullText);

        QPoint pos = event->pos();
        int point = cursorPositionAt(pos);
        setCursorPosition(point);
    }

    return QLineEdit::mouseMoveEvent(event);
}

void ReplicableLabel::resizeEvent(QResizeEvent *event)
{
    QFontMetrics fontMetrics(this->font());

    // 截断文本的显示宽度，需要扣除输入框控件两边的绘制间距
    int drawTextWidth = event->size().width() - LineEditHorizontalMargin * 2;
    m_elideText = fontMetrics.elidedText(m_fullText, m_elideMode, drawTextWidth);

    QLineEdit::setText(m_elideText);

    AC_SET_ACCESSIBLE_NAME(this, m_elideText);

    return QLineEdit::resizeEvent(event);
}
