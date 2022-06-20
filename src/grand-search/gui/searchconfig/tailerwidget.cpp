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
#include "tailerwidget.h"
#include "business/config/searchconfig.h"
#include "global/searchconfigdefine.h"

#include <DFontSizeManager>
#include <DDialog>

DWIDGET_USE_NAMESPACE

TailerWidget::TailerWidget(QWidget *parent)
    : DWidget(parent)
{
    m_groupLabel = new QLabel(tr("Tailer settings"), this);
    DFontSizeManager::instance()->bind(m_groupLabel, DFontSizeManager::T5, QFont::Bold);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addWidget(m_groupLabel);
    m_mainLayout->addSpacerItem(new QSpacerItem(10, 4));

    QString content = tr("It is displayed at the end of search results for better "
                         "identification and distinction of items with duplicate names.");
    m_contentLabel = new QLabel(content, this);
    m_contentLabel->setWordWrap(true);
    m_mainLayout->addWidget(m_contentLabel);
    m_mainLayout->addSpacerItem(new QSpacerItem(10, 10));

    m_checkBoxWidget = new CheckBoxWidget(this);
    m_checkBoxWidget->addCheckBox(tr("Parent directory"),
                                  SearchConfig::instance()->getConfig(GRANDSEARCH_TAILER_GROUP, GRANDSEARCH_TAILER_PARENTDIR, false).toBool());
    m_checkBoxWidget->addCheckBox(tr("Time modified"),
                                  SearchConfig::instance()->getConfig(GRANDSEARCH_TAILER_GROUP, GRANDSEARCH_TAILER_TIMEMODEFIED, true).toBool());
    m_mainLayout->addWidget(m_checkBoxWidget);

    connect(m_checkBoxWidget, &CheckBoxWidget::checkedChanged, this, &TailerWidget::onCheckBoxStateChanged);
}

TailerWidget::~TailerWidget()
{
}

void TailerWidget::onCheckBoxStateChanged(int index, bool checked)
{
    switch (index) {
    case ParentDirectory:
        SearchConfig::instance()->setConfig(GRANDSEARCH_TAILER_GROUP, GRANDSEARCH_TAILER_PARENTDIR, checked);
        break;
    case TimeModified:
        SearchConfig::instance()->setConfig(GRANDSEARCH_TAILER_GROUP, GRANDSEARCH_TAILER_TIMEMODEFIED, checked);
        break;
    default:
        break;
    }
}
