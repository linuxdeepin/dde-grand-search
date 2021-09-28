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
#include "detailitem.h"
#include "replicablelabel.h"

#include <DLineEdit>

#include <QPainter>
#include <QPainterPath>
#include <QRect>
#include <QVariant>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

DetailItem::DetailItem(QWidget *parent)
    : QWidget(parent)
{
    m_backgroundColor = QColor(0, 0, 0, int(255*0.05));

    setContentsMargins(20, 0, 10, 0);

    m_radius = 8;

    m_tagLabel = new DLabel(this);
    m_tagLabel->setAlignment(Qt::AlignLeft);

    m_contentLabel = new ReplicableLabel(this);
    m_contentLabel->setAlignment(Qt::AlignLeft);

    m_mainLayout = new QHBoxLayout(this);

    m_mainLayout->addWidget(m_tagLabel);
    m_mainLayout->addWidget(m_contentLabel);
}

DetailItem::~DetailItem()
{

}

void DetailItem::setDetailInfo(const DetailInfo &info)
{

    auto keyInfos = info.first;
    auto valueInfos = info.second;

    for (auto key : keyInfos.keys()) {
        if (DetailInfoProperty::Text == key) {
            auto text = keyInfos.value(key).toString();
            m_tagLabel->setText(text);
            m_tagLabel->adjustSize();       // 根据标签内容，调整合适大小
        } else if (DetailInfoProperty::ElideMode == key) {
            auto elideMode = keyInfos.value(key).value<Qt::TextElideMode>();
            m_tagLabel->setElideMode(elideMode);
        }
    }

    for (auto key : valueInfos.keys()) {
        if (DetailInfoProperty::Text == key) {
            auto text = valueInfos.value(key).toString();
            m_contentLabel->setText(text);
            m_contentLabel->setToolTip(text);
        } else if (DetailInfoProperty::ElideMode == key) {
            auto elideMode = valueInfos.value(key).value<Qt::TextElideMode>();
            m_contentLabel->setElideMode(elideMode);
        }
    }

    update();
}

void DetailItem::setTopRound(bool round)
{
    m_topRound = round;
}

bool DetailItem::topRound() const
{
    return m_topRound;
}

void DetailItem::setBottomRound(bool round)
{
    m_bottomRound = round;
}

bool DetailItem::bottomRound() const
{
    return m_bottomRound;
}

void DetailItem::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
}

QColor DetailItem::backgroundColor() const
{
    return m_backgroundColor;
}

void DetailItem::setRadius(int radius)
{
    m_radius = radius;
}

int DetailItem::radius() const
{
    return m_radius;
}

void DetailItem::setTagWidth(int width)
{
    m_tagLabel->setFixedWidth(width);
    update();
}

int DetailItem::tagWidth() const
{
    return m_tagLabel->width();
}

void DetailItem::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setBrush(m_backgroundColor);

    QRect roundRect(0, 0, m_radius * 2, m_radius * 2);
    QPainterPath path;
    int w = width();
    int h = height();

    if (m_topRound) {
        // 左上圆角
        path.moveTo(0, m_radius);
        roundRect.moveTo(0, 0);
        path.arcTo(roundRect, 180, -90);

        // 右上圆角
        path.lineTo(w - m_radius, 0);
        roundRect.moveTo(w - roundRect.width(), 0);
        path.arcTo(roundRect, 90, -90);
    } else {
        path.moveTo(0, 0);
        path.lineTo(w, 0);
    }

    if (m_bottomRound) {
        // 右下圆角
        path.lineTo(w, h - m_radius);
        roundRect.moveTo(w - roundRect.width(), h - roundRect.width());
        path.arcTo(roundRect, 0, -90);

        // 左下圆角
        path.lineTo(m_radius, h);
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
