// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semanticsearchwidget.h"
#include "tipslabel.h"
#include "global/searchconfigdefine.h"
#include "business/config/searchconfig.h"

#include <QVBoxLayout>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

DWIDGET_USE_NAMESPACE

using namespace GrandSearch;

SemanticSearchWidget::SemanticSearchWidget(QWidget *parent)
    : DWidget(parent)
{
    initUi();
    initConnect();
}

SemanticSearchWidget::~SemanticSearchWidget()
{
}

void SemanticSearchWidget::initUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 智能搜索开关
    QHBoxLayout *switchLayout = new QHBoxLayout;
    switchLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *groupLabel = new QLabel(tr("Smart search"), this);
    m_switchBtn = new DSwitchButton(this);
    switchLayout->addWidget(groupLabel, 1);
    switchLayout->addWidget(m_switchBtn);

    // 从配置加载初始状态
    bool enabled = SearchConfig::instance()->getConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_SEMANTIC_ENABLED, true).toBool();
    m_switchBtn->setChecked(enabled);

    // 描述信息
    TipsLabel *tipsLabel = new TipsLabel(tr("When enabled, natural language search is supported in grand search, e.g., \"PPT documents edited yesterday\""), this);
    tipsLabel->setWordWrap(true);

    mainLayout->addLayout(switchLayout);
    mainLayout->addWidget(tipsLabel);
}

void SemanticSearchWidget::initConnect()
{
    connect(m_switchBtn, &DSwitchButton::checkedChanged, this, &SemanticSearchWidget::onSwitchToggled);
}

void SemanticSearchWidget::onSwitchToggled(bool checked)
{
    qCDebug(logGrandSearch) << "Smart search configuration changed - Enabled:" << checked;
    SearchConfig::instance()->setConfig(GRANDSEARCH_SEMANTIC_GROUP, GRANDSEARCH_SEMANTIC_ENABLED, checked);
}
