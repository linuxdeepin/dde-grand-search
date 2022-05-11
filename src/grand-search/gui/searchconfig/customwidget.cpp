/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "customwidget.h"
#include "switchwidget/switchwidget.h"
#include "gui/datadefine.h"
#include "business/config/searchconfig.h"
#include "utils/utils.h"
#include "global/builtinsearch.h"
#include "global/searchconfigdefine.h"
#include "planwidget.h"
#include "bestmatchwidget.h"
#include "searchenginewidget.h"
#include "tailerwidget.h"

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

CustomWidget::CustomWidget(QWidget *parent)
    : DWidget(parent)
{
    m_groupLabel = new QLabel(tr("Custom search"), this);
    DFontSizeManager::instance()->bind(m_groupLabel, DFontSizeManager::T5, QFont::Bold);
    m_groupLabel->setMargin(0);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_mainLayout->addWidget(m_groupLabel);

    m_innerLayout = new QVBoxLayout(this);
    m_innerLayout->setSpacing(20);
    m_innerLayout->setContentsMargins(0, 0, 0, 0);

    m_tailerWidget = new TailerWidget(this);
    m_bestMatchWidget = new BestMatchWidget(this);
    m_searchEngineWidget = new SearchEngineWidget(this);
    m_searchPlanWidget = new PlanWidget(this);
    m_innerLayout->addWidget(m_tailerWidget);
    m_innerLayout->addWidget(m_bestMatchWidget);
    m_innerLayout->addWidget(m_searchEngineWidget);
    m_innerLayout->addWidget(m_searchPlanWidget);

    m_mainLayout->addLayout(m_innerLayout);

}

CustomWidget::~CustomWidget()
{
    m_switchWidgets.clear();
}

void CustomWidget::onSwitchStateChanged(const bool checked)
{
    QObject *obj = sender();
    SwitchWidget *switchWidget = static_cast<SwitchWidget *>(obj);

    if (switchWidget) {
        QString group = switchWidget->property(GRANDSEARCH_CUSTOM_GROUP).toString();
        SearchConfig::instance()->setConfig(GRANDSEARCH_CUSTOM_GROUP, group, checked);
    }
}
