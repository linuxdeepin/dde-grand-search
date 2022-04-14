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
#include "checkboxwidget.h"
#include "checkboxitem.h"
#include "global/searchconfigdefine.h"

DWIDGET_USE_NAMESPACE

CheckBoxWidget::CheckBoxWidget(QWidget *parent)
    : DWidget(parent)
{
    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setSpacing(1);
    m_mainLayout->setMargin(0);

    setLayout(m_mainLayout);
}

CheckBoxWidget::~CheckBoxWidget()
{
}

int CheckBoxWidget::addCheckBox(const QString &text, bool isChecked)
{
    int index = m_checkBoxList.size();

    CheckBoxItem *c = new CheckBoxItem(text, this);
    c->setFixedSize(CHECKBOXITEMWIDTH, CHECKBOXITEMHEIGHT);
    addCheckBox(index, c, isChecked);

    return index;
}

void CheckBoxWidget::addCheckBox(int index, CheckBoxItem *item, bool isChecked)
{
    item->setChecked(isChecked);
    m_mainLayout->insertWidget(index, item);
    m_checkBoxList << item;

    for (int i = 0; i < m_checkBoxList.count(); ++i) {
        if (i == 0) {
            m_checkBoxList[i]->setTopRound(true);
        } else if (i == m_checkBoxList.count() - 1) {
            m_checkBoxList[i]->setBottomRound(true);
        } else {
            m_checkBoxList[i]->setTopRound(false);
            m_checkBoxList[i]->setBottomRound(false);
        }
    }

    connect(item, &CheckBoxItem::toggled, this, &CheckBoxWidget::onCheckBoxChecked);
}

bool CheckBoxWidget::setCheckBoxChecked(int index, bool isChecked)
{
    if (index < 0 || index >= m_checkBoxList.size())
        return false;

    m_checkBoxList[index]->setChecked(isChecked);
    return true;
}

bool CheckBoxWidget::setCheckBoxChecked(const QString &text, bool isChecked)
{
    int i = -1;
    for (const auto *c : m_checkBoxList) {
        ++i;
        if (c->text() == text)
            break;
    }

    return setCheckBoxChecked(i, isChecked);
}

void CheckBoxWidget::onCheckBoxChecked(bool checked)
{
    CheckBoxItem *c = qobject_cast<CheckBoxItem *>(sender());
    if (c) {
        int index = m_checkBoxList.indexOf(c);
        emit checkedChanged(index, checked);
    }
}
