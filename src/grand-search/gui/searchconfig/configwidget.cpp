// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
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

#include <QLabel>
#include <QPushButton>
#include <QCloseEvent>
#include <QScrollBar>
#include <QLoggingCategory>
#include <QPalette>
#include <DPalette>
#include <DPaletteHelper>
#include <DGuiApplicationHelper>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

#define MAINWINDOW_WIDTH    696
#define MAINWINDOW_HEIGHT   529
#define WINDOW_FRAME_WIDTH  10      // 窗口灰底边框宽度，内容白底距窗口边缘的距离
#define CONTENT_MARGIN      (109 - WINDOW_FRAME_WIDTH)  // 白底内左右留白（相对窗口为 109）

using namespace GrandSearch;
DWIDGET_USE_NAMESPACE

ConfigWidget::ConfigWidget(QWidget *parent)
    : DMainWindow(parent)
{
    qCDebug(logGrandSearch) << "Creating ConfigWidget";
    initUI();
    initData();
    qCDebug(logGrandSearch) << "ConfigWidget created successfully";
}

ConfigWidget::~ConfigWidget()
{
    qCDebug(logGrandSearch) << "Destroying ConfigWidget";
}

void ConfigWidget::initUI()
{
    setFixedSize(MAINWINDOW_WIDTH, MAINWINDOW_HEIGHT);

    // 禁用最小、最大化按钮
    setWindowFlag(Qt::WindowMinMaxButtonsHint, false);

    // 禁用默认菜单
    this->titlebar()->setMenuVisible(false);

    // 关闭标题栏阴影，避免阴影压暗内容白底顶部
    setTitlebarShadowEnabled(false);

    // 设置图标
    QIcon tmpIcon = QIcon(QString(":/icons/%1.svg").arg("dde-grand-search-setting"));
    this->titlebar()->setIcon(tmpIcon);
    setWindowIcon(tmpIcon);
    QWidget *mainWidget = new QWidget(this);
    mainWidget->setObjectName("mainBackgroundWidget");
    setCentralWidget(mainWidget);

    // 窗口灰底：内容白底四周留出边框，露出窗口底色
    m_mainLayout = new QVBoxLayout(mainWidget);
    m_mainLayout->setContentsMargins(WINDOW_FRAME_WIDTH, WINDOW_FRAME_WIDTH,
                                     WINDOW_FRAME_WIDTH, WINDOW_FRAME_WIDTH);
    m_mainLayout->setSpacing(0);
    mainWidget->setLayout(m_mainLayout);

    // 内容白底（圆角面板）
    m_contentPanel = new QWidget(mainWidget);
    m_contentPanel->setObjectName("contentBackgroundWidget");
    m_panelLayout = new QVBoxLayout(m_contentPanel);
    m_panelLayout->setContentsMargins(0, 0, 0, 0);
    m_panelLayout->setSpacing(0);
    m_contentPanel->setLayout(m_panelLayout);
    m_mainLayout->addWidget(m_contentPanel);

    m_scrollArea = new DScrollArea(m_contentPanel);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setLineWidth(0);
    m_panelLayout->addWidget(m_scrollArea);

    m_scrollAreaContent = new QWidget(m_scrollArea);
    m_scrollLayout = new QVBoxLayout(m_scrollAreaContent);
    m_scrollLayout->setContentsMargins(CONTENT_MARGIN, 20, CONTENT_MARGIN, 20);
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

    updateBackgroundColor();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &ConfigWidget::updateBackgroundColor);
}

void ConfigWidget::updateBackgroundColor()
{
    const DPalette palette = DPaletteHelper::instance()->palette(this);
    const QColor windowColor = palette.color(DPalette::Window);   // 窗口底色
    const QColor contentColor = palette.color(DPalette::Base);    // 内容白底

    // 窗口整体使用窗口底色，内容区域作为白底圆角面板浮在其上
    centralWidget()->setStyleSheet(
        QString("#mainBackgroundWidget { background-color: %1; }"
                "#contentBackgroundWidget { background-color: %2; border-radius: 8px; }")
                .arg(windowColor.name(), contentColor.name()));

    // 滚动区域自身透明，透出内容白底（含圆角），滑块绘制在白底右边缘
    m_scrollArea->viewport()->setAutoFillBackground(false);
    m_scrollArea->setObjectName("contentScrollArea");
    m_scrollArea->setStyleSheet("#contentScrollArea > QWidget { background-color: transparent; }");

    m_scrollAreaContent->setAutoFillBackground(false);
}

void ConfigWidget::closeEvent(QCloseEvent *event)
{
    if (!m_indexWidget->onCloseEvent())
        event->ignore();
}

void ConfigWidget::initData()
{
}

void ConfigWidget::scrollToAiConfig()
{
    qCDebug(logGrandSearch) << "Scrolling to AI configuration section";
    QPoint targetPos = m_indexWidget->mapTo(m_scrollArea->viewport(), QPoint(0, 0));
    m_scrollArea->verticalScrollBar()->setValue(targetPos.y());
}
