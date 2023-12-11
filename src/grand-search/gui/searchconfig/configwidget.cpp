// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configwidget.h"
#include "business/config/searchconfig.h"
#include "global/builtinsearch.h"
#include "scopewidget.h"
#include "customwidget.h"
#include "indexwidget.h"

#include <DSwitchButton>
#include <DScrollArea>
#include <DTitlebar>
#include <DFrame>

#include <QLabel>
#include <QPushButton>

#define MAINWINDOW_WIDTH    696
#define MAINWINDOW_HEIGHT   529

using namespace GrandSearch;
DWIDGET_USE_NAMESPACE

ConfigWidget::ConfigWidget(QWidget *parent)
    : DMainWindow(parent)
{
    initUI();
    initData();
}

ConfigWidget::~ConfigWidget()
{
}

void ConfigWidget::initUI()
{
    setFixedSize(MAINWINDOW_WIDTH, MAINWINDOW_HEIGHT);

    // 禁用最小、最大化按钮
    setWindowFlag(Qt::WindowMinMaxButtonsHint, false);

    // 禁用默认菜单
    this->titlebar()->setMenuVisible(false);

    // 设置图标
    QIcon tmpIcon = QIcon::fromTheme("dde-grand-search-setting");
    this->titlebar()->setIcon(tmpIcon);
    setWindowIcon(tmpIcon);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    auto centralLayout = new QVBoxLayout(central);
    centralLayout->setContentsMargins(10, 10, 10, 10);

    DFrame *mainWidget = new DFrame(this);
    mainWidget->setFrameRounded(true);
    centralLayout->addWidget(mainWidget);

    m_mainLayout = new QVBoxLayout(mainWidget);
    m_mainLayout->setContentsMargins(10, 10, 10, 10); // 设置10px用于画圆角
    m_mainLayout->setSpacing(0);
    mainWidget->setLayout(m_mainLayout);

    m_scrollArea = new DScrollArea;
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setLineWidth(0);
    m_mainLayout->addWidget(m_scrollArea);

    m_scrollAreaContent = new QWidget(m_scrollArea);
    m_scrollLayout = new QVBoxLayout(m_scrollAreaContent);
    // 左右边距100px，上面圆角用了10px，因此这里是90px
    m_scrollLayout->setContentsMargins(90, 10, 90, 10);
    m_scrollLayout->setSpacing(20);
    m_scrollAreaContent->setLayout(m_scrollLayout);

    m_searchGroupWidget = new ScopeWidget(m_scrollAreaContent);
    m_searchCustomWidget = new CustomWidget(m_scrollAreaContent);
    m_indexWidget = new IndexWidget(m_scrollAreaContent);
    m_scrollLayout->addWidget(m_searchGroupWidget);
    m_scrollLayout->addWidget(m_indexWidget);
    m_scrollLayout->addWidget(m_searchCustomWidget);
    m_scrollLayout->addStretch();

    m_scrollArea->setWidget(m_scrollAreaContent);
}

void ConfigWidget::initData()
{
}
