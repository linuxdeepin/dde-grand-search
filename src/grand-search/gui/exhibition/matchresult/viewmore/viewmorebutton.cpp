/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#include "viewmorebutton.h"

#include <DStyle>
#include <DGuiApplicationHelper>

#include <QPainter>
#include <QStyleOptionToolButton>

DWIDGET_USE_NAMESPACE

ViewMoreButton::ViewMoreButton(const QString &text, QWidget *parent)
    : QToolButton(parent)
{
    setText(text);
}

void ViewMoreButton::setSelected(bool selected)
{
    if (m_bIsSelected == selected)
        return;

    m_bIsSelected = selected;
    update();
}

bool ViewMoreButton::isSelected()
{
    return m_bIsSelected;
}

void ViewMoreButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::NoBrush);

    QColor backgroundColor(0, 0, 0, 0);
    QColor textColor = QColor(0, 0, 0, static_cast<int>(255 * 0.4));
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        textColor = QColor(255, 255, 255, static_cast<int>(255 * 0.4));
    // normal状态、hover状态 、press状态
    if (opt.state & (QStyle::State_Active | QStyle::State_MouseOver | QStyle::State_Sunken)) {
        auto state = DStyle::getState(&opt);
        switch (state) {
        case DStyle::SS_NormalState: {
            if (m_bIsSelected) {
                backgroundColor = QColor(0, 0, 0, static_cast<int>(255 * 0.05));
                if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
                    backgroundColor = QColor(255, 255, 255, static_cast<int>(255 * 0.05));
            }
            break;
        }
        case DStyle::SS_HoverState: {
            backgroundColor = QColor(0, 0, 0, static_cast<int>(255 * 0.05));
            if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
                backgroundColor = QColor(255, 255, 255, static_cast<int>(255 * 0.05));

            break;
        }
        case DStyle::SS_PressState: {
            backgroundColor = QColor(0, 0, 0, static_cast<int>(255 * 0.1));
            if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
                backgroundColor = QColor(255, 255, 255, static_cast<int>(255 * 0.1));

            textColor = QColor(0, 129, 255);
            break;
        }
        default:
            break;
        }
    }

    p.setBrush(backgroundColor);
    p.setFont(opt.font);
    p.drawRoundedRect(opt.rect, 13, 13);

    p.setPen(textColor);
    p.drawText(opt.rect, Qt::AlignCenter | Qt::TextShowMnemonic, opt.text);
}

