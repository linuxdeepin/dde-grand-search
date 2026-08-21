// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iconbutton.h"

#include <DStyle>
#include <DGuiApplicationHelper>
#include <DStyleOptionButton>

#include <QPainter>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

IconButton::IconButton(QWidget *parent)
    : DIconButton(parent)
{
}

void IconButton::paintEvent(QPaintEvent *event)
{
    DStyleOptionButton opt;
    initStyleOption(&opt);

    if (opt.state & (QStyle::State_Sunken | QStyle::State_MouseOver)) {
        DPalette dp = DGuiApplicationHelper::instance()->applicationPalette();
        QColor bgColor = dp.color(DPalette::ObviousBackground);
        if (opt.state & QStyle::State_Sunken)
            bgColor.setAlphaF(0.2);

        int radius = qMin(width(), height()) / 2;
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(bgColor);
        p.drawEllipse(QPointF(width() / 2.0, height() / 2.0), radius, radius);
    }

    DStylePainter p(this);
    p.drawControl(DStyle::CE_IconButton, opt);
}
