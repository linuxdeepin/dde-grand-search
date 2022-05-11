/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhaohanxi<zhaohanxi@uniontech.com>
 *
 * Maintainer: zhaohanxi<zhaohanxi@uniontech.com>
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

#include "comboboxwidget.h"
#include "global/searchhelper.h"

#include <DPalette>
#include <DApplicationHelper>

#include <QPainter>

DWIDGET_USE_NAMESPACE

#define COMBOBOXW 250
#define COMBOBOXH 36

namespace GrandSearch {

ComboboxWidget::ComboboxWidget(const QString &title, QWidget *parent)
    : ComboboxWidget(parent, new QLabel(title, parent))
{

}

ComboboxWidget::ComboboxWidget(QWidget *parent, QWidget *leftWidget)
    : QWidget (parent)
    , m_leftWidget(leftWidget)
{
    if (!m_leftWidget)
        m_leftWidget = new QLabel(this);
    m_leftWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_comboBox = new DComboBox(this);
    m_comboBox->setFixedSize(COMBOBOXW, COMBOBOXH);

    QHBoxLayout *labelLayout = new QHBoxLayout;
    labelLayout->addWidget(m_leftWidget);

    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 6, 10, 6);
    m_mainLayout->addLayout(labelLayout, 0);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_comboBox, 0, Qt::AlignVCenter);
    setLayout(m_mainLayout);

    connect(m_comboBox, QOverload<int>::of(&DComboBox::currentIndexChanged), this, &ComboboxWidget::checkedChanged);
}

void ComboboxWidget::setChecked(const QString &text)
{
    m_comboBox->setCurrentText(text);
}

QString ComboboxWidget::checked() const
{
    return m_comboBox->currentText();
}

void ComboboxWidget::setTitle(const QString &title)
{
    QLabel *label = qobject_cast<QLabel *>(m_leftWidget);
    if (label) {
        label->setText(title);
        label->adjustSize();
    }
}

QString ComboboxWidget::title() const
{
    QLabel *label = qobject_cast<QLabel *>(m_leftWidget);
    if (label) {
        return label->text();
    }

    return QString();
}

void ComboboxWidget::setEnableBackground(const bool enable)
{
    if (enable == m_hasBack)
        return;
    m_hasBack = enable;
    update();
}

bool ComboboxWidget::enableBackground() const
{
    return m_hasBack;
}

DComboBox *ComboboxWidget::getComboBox()
{
    return m_comboBox;
}

void ComboboxWidget::paintEvent(QPaintEvent *event)
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

}
