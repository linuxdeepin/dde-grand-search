// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "viewmorebutton.h"

#include <DStyle>
#include <DGuiApplicationHelper>
#include <DPlatformTheme>

#include <QPainter>
#include <QStyleOptionToolButton>

using namespace GrandSearch;
DWIDGET_USE_NAMESPACE

ViewMoreButton::ViewMoreButton(const QString &text, QWidget *parent)
    : QToolButton(parent)
{
    setText(text);
    connect(DGuiApplicationHelper::instance()->systemTheme(), &DPlatformTheme::activeColorChanged, this, [this]() { update(); });
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [this]() { update(); });
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

            // press 态文字跟随系统活动色
            textColor = DGuiApplicationHelper::instance()->systemTheme()->activeColor();
            break;
        }
        default:
            break;
        }
    }

    p.setBrush(backgroundColor);
    p.setFont(opt.font);
    p.drawRoundedRect(opt.rect, 4, 4);

    p.setPen(textColor);
    p.drawText(opt.rect, Qt::AlignCenter | Qt::TextShowMnemonic, opt.text);
}

