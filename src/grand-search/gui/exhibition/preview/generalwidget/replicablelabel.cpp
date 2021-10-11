/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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

#include <DGuiApplicationHelper>
#include <DStyle>

#include <QMouseEvent>
#include <QResizeEvent>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

ReplicableLabel::ReplicableLabel(QWidget *parent)
    : QLineEdit(parent)
{
    QPalette palette;
    QColor colorButton(0, 0, 0, 0);
    palette.setColor(QPalette::Button, colorButton);
    this->setPalette(palette);
    DStyle::setFocusRectVisible(this, false);

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

    m_elideText = fontMetrics.elidedText(m_fullText, m_elideMode, event->size().width());

    QLineEdit::setText(m_elideText);

    return QLineEdit::resizeEvent(event);
}
