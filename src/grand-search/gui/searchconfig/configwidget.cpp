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
#include "configwidget.h"
#include "business/config/searchconfig.h"
#include "global/builtinsearch.h"
#include "scopewidget.h"
#include "planwidget.h"
#include "customwidget.h"
#include "tailerwidget.h"

#include <DSwitchButton>
#include <DScrollArea>
#include <DTitlebar>

#include <QLabel>
#include <QPushButton>

#define MAINWINDOW_WIDTH    696
#define MAINWINDOW_HEIGHT   529

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
    QIcon tmpIcon = QIcon(QString(":/icons/%1.svg").arg("dde-grand-search-setting"));
    this->titlebar()->setIcon(tmpIcon);
    setWindowIcon(tmpIcon);
    QWidget *mainWidget = new QWidget(this);
    setCentralWidget(mainWidget);

    m_mainLayout = new QVBoxLayout(mainWidget);
    m_mainLayout->setContentsMargins(0, 10, 0, 10);
    m_mainLayout->setSpacing(0);
    mainWidget->setLayout(m_mainLayout);

    m_scrollArea = new DScrollArea;
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setLineWidth(0);
    m_mainLayout->addWidget(m_scrollArea);

    m_scrollAreaContent = new QWidget(m_scrollArea);
    m_scrollLayout = new QVBoxLayout(m_scrollAreaContent);
    m_scrollLayout->setContentsMargins(109, 20, 109, 20);
    m_scrollLayout->setSpacing(20);
    m_scrollAreaContent->setLayout(m_scrollLayout);

    m_searchGroupWidget = new ScopeWidget(m_scrollAreaContent);
    m_searchCustomWidget = new CustomWidget(m_scrollAreaContent);
    m_scrollLayout->addWidget(m_searchGroupWidget);
    m_scrollLayout->addWidget(m_searchCustomWidget);
    m_scrollLayout->addStretch();

    m_scrollArea->setWidget(m_scrollAreaContent);
}

void ConfigWidget::initData()
{
}
