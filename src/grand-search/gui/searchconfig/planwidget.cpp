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
#include "planwidget.h"
#include "business/config/searchconfig.h"
#include "global/builtinsearch.h"
#include "global/searchconfigdefine.h"
#include "switchwidget/switchwidget.h"

#include <DFontSizeManager>

#include <QLabel>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

PlanWidget::PlanWidget(QWidget *parent)
    : DWidget(parent)
{
    m_groupLabel = new QLabel(tr("搜索体验计划"), this);
    DFontSizeManager::instance()->bind(m_groupLabel, DFontSizeManager::T5, QFont::Bold);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addWidget(m_groupLabel);

    QString display = QObject::tr("加入搜索体验计划");
    QString iconName("experienceplan");
    QIcon icon = QIcon(QString(":/icons/%1.svg").arg(iconName));

    bool enable = SearchConfig::instance()->getConfig(GRANDSEARCH_PLAN_GROUP, GRANDSEARCH_PLAN_EXPERIENCE, true).toBool();
    SwitchWidget *switchWidget = new SwitchWidget(this);
    switchWidget->setFixedSize(SWITCHWIDGETWIDTH, SWITCHWIDGETHEIGHT);
    switchWidget->setEnableBackground(true);
    switchWidget->setIcon(icon, QSize(SWITCHWIDGETICONSIZE, SWITCHWIDGETICONSIZE));
    switchWidget->setTitle(display);
    switchWidget->setChecked(enable);
    m_mainLayout->addWidget(switchWidget);
    switchWidget->setProperty(GRANDSEARCH_SEARCH_GROUP, GRANDSEARCH_PLAN_EXPERIENCE);

    QString content = QObject::tr("开启搜索体验计划，视为您授权我们收集与您的设备、系统、文件图标、部分文件内容、文件属性"
                                  "、应用软件及配置等信息，您可以关闭搜索体验计划，以拒绝我们对前述信息的搜集与使用。");
    m_contentLabel = new QLabel(content, this);
    m_contentLabel->setWordWrap(true);

    QString tips = QObject::tr("了解对数据的管理与使用方式，请参考统信软件隐私政策"
                               "（https://www.uniontech.com/agreenment/privacy-cn）。");
    m_tipsLabel = new QLabel(tips, this);
    m_tipsLabel->setWordWrap(true);

    m_mainLayout->addWidget(m_contentLabel);
    m_mainLayout->addWidget(m_tipsLabel);

    connect(switchWidget, &SwitchWidget::checkedChanged, this, &PlanWidget::onSwitchStateChanged);
}

PlanWidget::~PlanWidget()
{

}

void PlanWidget::onSwitchStateChanged(const bool checked)
{
    QObject *obj = sender();
    SwitchWidget *switchWidget = static_cast<SwitchWidget *>(obj);

    if (switchWidget) {
        SearchConfig::instance()->setConfig(GRANDSEARCH_PLAN_GROUP, GRANDSEARCH_PLAN_EXPERIENCE, checked);
    }
}
