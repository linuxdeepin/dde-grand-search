// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "checkboxwidget.h"
#include "checkboxitem.h"
#include "global/searchconfigdefine.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

using namespace GrandSearch;
DWIDGET_USE_NAMESPACE

CheckBoxWidget::CheckBoxWidget(QWidget *parent)
    : DWidget(parent)
{
    qCDebug(logGrandSearch) << "Creating CheckBoxWidget";
    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setSpacing(1);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(m_mainLayout);
    qCDebug(logGrandSearch) << "CheckBoxWidget created successfully";
}

CheckBoxWidget::~CheckBoxWidget()
{
    qCDebug(logGrandSearch) << "Destroying CheckBoxWidget";
}

int CheckBoxWidget::addCheckBox(const QString &text, bool isChecked)
{
    int index = m_checkBoxList.size();

    qCDebug(logGrandSearch) << "Adding checkbox - Text:" << text
                            << "Checked:" << isChecked << "Index:" << index;
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
    if (index < 0 || index >= m_checkBoxList.size()) {
        qCWarning(logGrandSearch) << "Invalid checkbox index - Index:" << index
                                  << "Valid range: 0-" << (m_checkBoxList.size() - 1);
        return false;
    }

    qCDebug(logGrandSearch) << "Setting checkbox state - Index:" << index
                            << "Checked:" << isChecked;
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
        qCDebug(logGrandSearch) << "Checkbox state changed - Index:" << index
                                << "Text:" << c->text() << "Checked:" << checked;
        emit checkedChanged(index, checked);
    } else {
        qCWarning(logGrandSearch) << "Checkbox state change from unknown sender";
    }
}
