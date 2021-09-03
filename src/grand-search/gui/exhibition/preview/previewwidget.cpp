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

#include <DScrollArea>
#include <DApplicationHelper>

#include <QDebug>
#include <QVBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QScrollBar>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

#define FIX_CONTENT_WIDTH       372
#define FIX_CONTENT_HEIGHT      444

PreviewWidgetPrivate::PreviewWidgetPrivate(PreviewWidget *parent)
    : q_p(parent)
{

}

PreviewWidget::PreviewWidget(QWidget *parent)
    : DWidget(parent)
    , d_p(new PreviewWidgetPrivate(this))
{
    initUi();
    initConnect();
}

PreviewWidget::~PreviewWidget()
{
    // 解除当前预览插件界面与预览主界面父子窗口关系，所有预览插件界面统一由插件管理类析构函数释放
    clearLayoutWidgets();
}

bool PreviewWidget::previewItem(const MatchedItem &item)
{
    // 搜索类型为空，或web搜索、应用、设置等，不显示预览
    if (item.searcher.isEmpty() || item.searcher == GRANDSEARCH_CLASS_APP_DESKTOP) {
        this->hide();
        return false;
    }

    PreviewPlugin* preview = m_pluginManager.getPreviewPlugin(item);

    // 插件有变更， 更换新插件界面内容到主界面布局中
    if (preview != m_preview) {

        // 清空主界面布局中原有预览插件界面内容
        clearLayoutWidgets();

        // 更换新插件界面内容到主界面布局
        preview->contentWidget()->setFixedSize(FIX_CONTENT_WIDTH, FIX_CONTENT_HEIGHT);
        preview->contentWidget()->adjustSize();

        m_vMainLayout->insertWidget(0, preview->contentWidget());
        m_preview = preview;
    }

    // 插件界面根据新来搜索结果刷新预览内容
    preview->previewItem(item);

    this->show();

    return true;
}

void PreviewWidget::initUi()
{
    m_vMainLayout = new QVBoxLayout(this);
    m_vMainLayout->setContentsMargins(0, 0, 0, 0);
    m_vMainLayout->setSpacing(0);
}

void PreviewWidget::initConnect()
{

}

void PreviewWidget::clearLayoutWidgets()
{
    if (m_preview) {
        m_vMainLayout->removeWidget(m_preview->contentWidget());
    }
}

void PreviewWidget::paintEvent(QPaintEvent *event)
{
    DWidget::paintEvent(event);

    // 预览界面背景阴影显示
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, static_cast<int>(255*0.03)));
    painter.drawRoundRect(rect(), 5, 5);
}
