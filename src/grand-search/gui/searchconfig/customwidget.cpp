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

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

CustomWidget::CustomWidget(QWidget *parent)
    : DWidget(parent)
{
    m_groupLabel = new QLabel(tr("Custom search"), this);
    DFontSizeManager::instance()->bind(m_groupLabel, DFontSizeManager::T5, QFont::Bold);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_mainLayout->addWidget(m_groupLabel);

    m_groupName = {{GRANDSEARCH_CUSTOM_BESTMATCH, GroupName_Best}};

    QStringList displayOrder;
    displayOrder << GRANDSEARCH_CUSTOM_BESTMATCH;

    m_displayIcons << QStringLiteral("bestmatch");

    for (int i = 0; i < displayOrder.count(); ++i) {
        const QString &searchGroup = displayOrder.at(i);
        bool enable = SearchConfig::instance()->getConfig(GRANDSEARCH_CUSTOM_GROUP, searchGroup, true).toBool();
        SwitchWidget *switchWidget = new SwitchWidget(this);
        switchWidget->setFixedSize(SWITCHWIDGETWIDTH, SWITCHWIDGETHEIGHT);
        switchWidget->setEnableBackground(true);
        switchWidget->setTitle(m_groupName.value(searchGroup));
        switchWidget->setChecked(enable);
        switchWidget->setProperty(GRANDSEARCH_CUSTOM_GROUP, searchGroup);

        m_mainLayout->addWidget(switchWidget);
        m_switchWidgets.append(switchWidget);

        connect(switchWidget, &SwitchWidget::checkedChanged, this, &CustomWidget::onSwitchStateChanged);
    }

    setLayout(m_mainLayout);
    updateIcons();

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &CustomWidget::updateIcons);

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

void CustomWidget::updateIcons()
{
    Q_ASSERT(m_switchWidgets.count() == m_displayIcons.count());

    QString suffix = Utils::iconThemeSuffix();

    for (int i = 0; i < m_switchWidgets.count(); ++i) {

        QString iconName = m_displayIcons.at(i);
        QIcon icon = QIcon(QString(":/icons/%1%2.svg").arg(iconName).arg(suffix));

        auto switchWidget = m_switchWidgets.at(i);
        Q_ASSERT(switchWidget);

        switchWidget->setIcon(icon, QSize(SWITCHWIDGETICONSIZE, SWITCHWIDGETICONSIZE));
    }

    update();
}
