// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "indexwidget.h"
#include "blacklistwidget.h"
#include "intelligentretrieval/intelligentretrievalwidget.h"

#include <DFontSizeManager>
#include <QLabel>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

using namespace GrandSearch;

IndexWidget::IndexWidget(QWidget *parent)
    : DWidget(parent)
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(10);

    m_blackListWidget = new BlackListWidget(this);
    m_groupLabel = new QLabel(tr("Index"));
    DFontSizeManager::instance()->bind(m_groupLabel, DFontSizeManager::T5, QFont::Bold);

    m_mainLayout->addWidget(m_groupLabel);

#ifdef ENABLE_AI_SEARCH
    m_intelligent = new IntelligentRetrievalWidget(this);
    m_mainLayout->addWidget(m_intelligent);
    m_mainLayout->addSpacing(10);
#endif
    m_mainLayout->addWidget(m_blackListWidget);
}

bool IndexWidget::onCloseEvent()
{
#ifdef ENABLE_AI_SEARCH
    return m_intelligent->onCloseEvent();
#else
    return true;
#endif
}

IndexWidget::~IndexWidget()
{

}
