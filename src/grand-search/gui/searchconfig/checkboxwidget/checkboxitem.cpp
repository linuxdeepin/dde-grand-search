/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liqianga@uniontech.com>
 *
 * Maintainer: liuzhangjian<liqianga@uniontech.com>
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
#include "checkboxitem.h"

#include <DPaletteHelper>

#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>

#define RADIUS 8

DWIDGET_USE_NAMESPACE

CheckBoxItem::CheckBoxItem(const QString &text, QWidget *parent)
    : DWidget(parent)
{
    m_checkBox = new DCheckBox(text, parent);
    auto mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_checkBox);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 0, 10, 0);
    setLayout(mainLayout);

    connect(m_checkBox, &DCheckBox::toggled, this, &CheckBoxItem::toggled);
}

CheckBoxItem::~CheckBoxItem()
{
}

void CheckBoxItem::setChecked(bool checked)
{
    m_checkBox->setChecked(checked);
}

void CheckBoxItem::setTopRound(bool round)
{
    m_topRound = round;
}

void CheckBoxItem::setBottomRound(bool round)
{
    m_bottomRound = round;
}

QString CheckBoxItem::text() const
{
    return m_checkBox->text();
}

void CheckBoxItem::paintEvent(QPaintEvent *event)
{
    const DPalette &dp = DPaletteHelper::instance()->palette(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setBrush(dp.brush(DPalette::ItemBackground));

    QRect roundRect(0, 0, RADIUS * 2, RADIUS * 2);
    QPainterPath path;
    int w = width();
    int h = height();

    if (m_topRound) {
        // 左上圆角
        path.moveTo(0, RADIUS);
        roundRect.moveTo(0, 0);
        path.arcTo(roundRect, 180, -90);

        // 右上圆角
        path.lineTo(w - RADIUS, 0);
        roundRect.moveTo(w - roundRect.width(), 0);
        path.arcTo(roundRect, 90, -90);
    } else {
        path.moveTo(0, 0);
        path.lineTo(w, 0);
    }

    if (m_bottomRound) {
        // 右下圆角
        path.lineTo(w, h - RADIUS);
        roundRect.moveTo(w - roundRect.width(), h - roundRect.width());
        path.arcTo(roundRect, 0, -90);

        // 左下圆角
        path.lineTo(RADIUS, h);
        roundRect.moveTo(0, h - roundRect.height());
        path.arcTo(roundRect, 270, -90);
    } else {
        path.lineTo(w, h);
        path.lineTo(0, h);
    }

    path.closeSubpath();
    p.drawPath(path);

    return QWidget::paintEvent(event);
}
