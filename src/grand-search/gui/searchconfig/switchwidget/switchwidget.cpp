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
#include "switchwidget.h"

#include <DPalette>
#include <DApplicationHelper>

#include <QPainter>

#define ICONLABELSIZE   36

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

SwitchWidget::SwitchWidget(const QString &title, QWidget *parent)
    : SwitchWidget(parent, new QLabel(title, parent))
{

}

SwitchWidget::SwitchWidget(QWidget *parent, QWidget *leftWidget)
    : QWidget (parent)
    , m_leftWidget(leftWidget)
{

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(ICONLABELSIZE, ICONLABELSIZE);

    if (!m_leftWidget)
        m_leftWidget = new QLabel(this);
    m_leftWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_switchBtn = new DSwitchButton(this);

    QHBoxLayout *labelLayout = new QHBoxLayout;
    labelLayout->addWidget(m_leftWidget);

    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 0, 10, 0);
    m_mainLayout->addWidget(m_iconLabel);
    m_mainLayout->addLayout(labelLayout, 0);
    m_mainLayout->addWidget(m_switchBtn, 0, Qt::AlignVCenter);
    setLayout(m_mainLayout);

    connect(m_switchBtn, &DSwitchButton::toggled, this, &SwitchWidget::checkedChanged);
}

void SwitchWidget::setChecked(const bool checked)
{
    m_switchBtn->blockSignals(true);
    m_switchBtn->setChecked(checked);
    m_switchBtn->blockSignals(false);
}

bool SwitchWidget::checked() const
{
    return m_switchBtn->isChecked();
}

void SwitchWidget::setTitle(const QString &title)
{
    QLabel *label = qobject_cast<QLabel *>(m_leftWidget);
    if (label) {
        label->setText(title);
        label->setWordWrap(true);
        label->adjustSize();
    }
}

QString SwitchWidget::title() const
{
    QLabel *label = qobject_cast<QLabel *>(m_leftWidget);
    if (label) {
        return label->text();
    }

    return QString();
}

void SwitchWidget::setIcon(const QIcon &icon, const QSize &size)
{
    m_iconLabel->setPixmap(icon.pixmap(size));

    update();
}

void SwitchWidget::setEnableBackground(const bool enable)
{
    if (enable == m_hasBack)
        return;
    m_hasBack = enable;
    update();
}

bool SwitchWidget::enableBackground() const
{
    return m_hasBack;
}

void SwitchWidget::paintEvent(QPaintEvent *event)
{
    if (m_hasBack) {
        const DPalette &dp = DApplicationHelper::instance()->palette(this);
        QPainter p(this);
        p.setPen(Qt::NoPen);
        p.setBrush(dp.brush(DPalette::ItemBackground));
        p.drawRoundedRect(rect(), 8, 8);
    }
    return QWidget::paintEvent(event);
}
