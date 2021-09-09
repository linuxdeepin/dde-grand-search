/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     houchengqiu<houchengqiu@uniontech.com>
 *
 * Maintainer: houchengqiu<houchengqiu@uniontech.com>
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
#include "previewwidget_p.h"
#include "previewwidget.h"
#include "utils/utils.h"
#include "generalpreviewplugin.h"
#include "detailinfowidget.h"

#include <DScrollArea>
#include <DApplicationHelper>
#include <DFrame>

#include <QDebug>
#include <QVBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QScrollBar>
#include <QToolButton>
#include <QClipboard>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

#define CONTENT_WIDTH           372

PreviewWidgetPrivate::PreviewWidgetPrivate(PreviewWidget *parent)
    : q_p(parent)
{

}

PreviewWidget::PreviewWidget(QWidget *parent)
    : DWidget(parent)
    , d_p(new PreviewWidgetPrivate(this))
{
    m_generalPreview = new GeneralPreviewPlugin();

    initUi();
    initConnect();
}

PreviewWidget::~PreviewWidget()
{
    // 解除当前预览插件界面与预览主界面父子窗口关系，所有预览插件界面统一由插件管理类析构函数释放
    clearLayoutWidgets();

    delete m_generalPreview;
}

bool PreviewWidget::previewItem(const MatchedItem &item)
{
    m_item = item;

    PreviewPlugin* preview = m_pluginManager.getPreviewPlugin(item);

    if (!preview)
        preview = m_generalPreview;

    // 插件有变更， 更换新插件界面内容到主界面布局中
    if (preview != m_preview) {

        // 清空主界面布局中原有预览插件界面内容
        clearLayoutWidgets();

        // 1. 更换新插件界面内容部件到主界面布局
        // 1.1 若插件未定制内容部件，则显示默认内容部件
        QWidget* contentWidget = preview->contentWidget();
        if (!contentWidget)
            contentWidget = m_generalPreview->contentWidget();

        if (contentWidget) {
            contentWidget->setFixedWidth(CONTENT_WIDTH);
            contentWidget->adjustSize();

            m_vMainLayout->addWidget(contentWidget);
            contentWidget->show();
        }

        // 2. 添加详情属性部件到预览界面
        m_vMainLayout->addWidget(m_detailInfoWidget);

        // 3. 添加垂直弹簧条，将工具栏部件置底到底部显示
        m_vMainLayout->addSpacerItem(m_vSpaceItem);

        // 3. 更换新插件工具栏部件到主界面布局
        // 3.1 若插件未定制工具栏部件，则显示默认工具栏部件
        QWidget* toolBar = preview->toolBarWidget();
        if (!toolBar)
            toolBar = m_generalPreview->toolBarWidget();

        if (toolBar) {
            m_vMainLayout->addWidget(toolBar);

            // 3.2 插件控制工具栏是否显示
            toolBar->setVisible(preview->showToolBar());
        }

        m_preview = preview;
    }

    // 插件界面根据新来搜索结果刷新预览内容
    preview->previewItem(m_item);

    m_detailInfoWidget->setDetailInfo(preview->getAttributeDetailInfo());

    this->show();

    return true;
}

void PreviewWidget::initUi()
{
    m_vMainLayout = new QVBoxLayout(this);
    m_vMainLayout->setContentsMargins(0, 0, 0, 8);
    m_vMainLayout->setSpacing(0);

    m_detailInfoWidget = new DetailInfoWidget(this);

    m_vSpaceItem = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
}

void PreviewWidget::initConnect()
{

}

void PreviewWidget::clearLayoutWidgets()
{
    if (m_preview) {
        if (m_preview->contentWidget())
            m_preview->contentWidget()->hide();
        if (m_preview->toolBarWidget())
            m_preview->toolBarWidget()->hide();
        m_vMainLayout->removeWidget(m_preview->contentWidget());
        m_vMainLayout->removeWidget(m_preview->toolBarWidget());
    }

    m_vMainLayout->removeWidget(m_detailInfoWidget);
    m_vMainLayout->removeItem(m_vSpaceItem);

    if (m_generalPreview) {
        if (m_generalPreview->contentWidget())
            m_generalPreview->contentWidget()->hide();
        if (m_generalPreview->toolBarWidget())
            m_generalPreview->toolBarWidget()->hide();
        m_vMainLayout->removeWidget(m_generalPreview->contentWidget());
        m_vMainLayout->removeWidget(m_generalPreview->toolBarWidget());
    }
}
