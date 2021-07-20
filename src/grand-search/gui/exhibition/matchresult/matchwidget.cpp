/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *             houchengqiu<houchengqiu@uniontech.com>
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
#include "matchwidget_p.h"
#include "matchwidget.h"
#include "listview/grandsearchlistview.h"
#include "business/matchresult/matchcontroller.h"
#include "business/query/querycontroller.h"
#include "gui/mainwindow.h"

#include <DScrollArea>
#include <DApplicationHelper>

#include <QDebug>
#include <QVBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

MatchWidgetPrivate::MatchWidgetPrivate(MatchWidget *parent)
    : q_p(parent)
{

}

MatchWidget::MatchWidget(QWidget *parent)
    : DWidget(parent)
    , d_p(new MatchWidgetPrivate(this))
{
    initUi();
    initConnect();

    reLayout();
}

MatchWidget::~MatchWidget()
{

}

void MatchWidget::connectToController()
{
    connect(MatchController::instance(), &MatchController::matchedResult, this, &MatchWidget::appendMatchedData);
    connect(QueryController::instance(), &QueryController::missionIdChanged, this, &MatchWidget::clearMatchedData);
    connect(MatchController::instance(), &MatchController::searchCompleted, this, &MatchWidget::onSearchCompleted);
}

void MatchWidget::appendMatchedData(const MatchedItemMap &matchedData)
{
    bool bNeedRelayout = false;

    // 数据处理
    MatchedItemMap::ConstIterator itData = matchedData.begin();
    while (itData != matchedData.end()) {

        QString groupHash = itData.key();

        // 根据groupHash创建对应类目列表，若已存在，直接返回已有类目列表
        GroupWidget* groupWidget = createGroupWidget(groupHash);
        if (!groupWidget)
            continue;

        // 追加匹配数据到类目列表中
        groupWidget->appendMatchedItems(itData.value());

        // 列表中有数据，显示类目列表
        groupWidget->setVisible(groupWidget->itemCount() > 0);

        //有新增匹配结果，需要调整重新布局
        if (itData.value().size() > 0)
            bNeedRelayout = true;

        itData++;
    }

    // 对需要显示的类目列表排序
    sortVislibleGroupList();

    // 重新调整布局
    if (bNeedRelayout)
        reLayout();
}

void MatchWidget::clearMatchedData()
{
    m_vGroupWidgets.clear();

    // 清空并隐藏所有类目列表
    for (GroupWidgetMap::Iterator it= m_groupWidgetMap.begin(); it!=m_groupWidgetMap.end(); ++it) {
        if (it.value())
            it.value()->clear();
    }

    // 重置滚动区域内容部件高度
    if (m_scrollAreaContent)
        m_scrollAreaContent->setFixedHeight(0);

    layout();
}

void MatchWidget::onSearchCompleted()
{
    if (!m_vGroupWidgets.isEmpty())
        return;

    MainWindow::instance()->showSerachNoContent(true);
}

void MatchWidget::onShowMore()
{
    reLayout();
}

void MatchWidget::initUi()
{
    m_vMainLayout = new QVBoxLayout(this);
    m_vMainLayout->setContentsMargins(0, 0, 0, 0);
    m_vMainLayout->setSpacing(0);
    m_scrollArea = new DScrollArea(this);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setWidgetResizable(true);
    m_scrollAreaContent = new DWidget();
    m_scrollArea->setWidget(m_scrollAreaContent);

    QPalette palette = m_scrollArea->palette();
    palette.setColor(QPalette::Background, Qt::transparent);
    m_scrollArea->setPalette(palette);
    m_scrollArea->setLineWidth(0);

    m_vMainLayout->addWidget(m_scrollArea);

    m_vScrollLayout = new QVBoxLayout(m_scrollAreaContent);
    m_vScrollLayout->setContentsMargins(0, 0, 0, 0);
    m_vScrollLayout->setSpacing(0);

}

void MatchWidget::initConnect()
{

}


void MatchWidget::reLayout()
{
    this->show();

    // 计算匹配结果界面整体高度
    int nTotalHeight = 0;
    for (int i = 0; i < m_vGroupWidgets.size(); i++) {
        GroupWidget *groupWidget = m_vGroupWidgets[i];
        if (groupWidget) {

            // 默认显示类目列表内分割横线
            groupWidget->showHorLine(true);

            // 隐藏视觉上最后一个类目列表横线
            if (i == m_vGroupWidgets.size() - 1)
                groupWidget->showHorLine(false);

            // 类目列表重新布局
            groupWidget->reLayout();
            int nHeight = groupWidget->height();
            nTotalHeight += nHeight;

            // 添加类目列表之间的间隙
            if (i != m_vGroupWidgets.size() - 1)
                nTotalHeight += 5;
        }
    }

    // 设置滚动区域内容部件整体高度
    if (m_scrollAreaContent)
        m_scrollAreaContent->setFixedHeight(nTotalHeight);

    // 打破当前布局，根据最新显示类目列表重新布局
    if (m_vScrollLayout) {
        delete m_vScrollLayout;
        m_vScrollLayout = nullptr;
    }
    m_vScrollLayout = new QVBoxLayout(m_scrollAreaContent);
    m_vScrollLayout->setContentsMargins(0, 0, 0, 0);
    m_vScrollLayout->setSpacing(0);
    m_scrollAreaContent->setLayout(m_vScrollLayout);

    for (auto groupWidget : m_vGroupWidgets) {
        if (groupWidget)
            m_vScrollLayout->addWidget(groupWidget, 0);
    }

    layout();
}

GroupWidget *MatchWidget::createGroupWidget(const QString &groupHash)
{
    if (groupHash.isEmpty())
        return nullptr;

    if (!m_groupWidgetMap[groupHash]) {
        GroupWidget* groupWidget = new GroupWidget(m_scrollAreaContent);
        connect(groupWidget, &GroupWidget::showMore, this, &MatchWidget::reLayout);

        groupWidget->setGroupName(groupHash);
        m_groupWidgetMap[groupHash] = groupWidget;

        return groupWidget;
    }

    return m_groupWidgetMap[groupHash];
}

void MatchWidget::sortVislibleGroupList()
{
    // 1.优先在界面中显示应用、文件夹和文件类目，显示顺序为: 应用 > 文件夹 > 文件
    m_vGroupWidgets.clear();
    for (auto groupHash : m_groupHashShowOrder) {
        if (m_groupWidgetMap[groupHash] && !m_groupWidgetMap[groupHash]->isHidden())
            m_vGroupWidgets.push_back(m_groupWidgetMap[groupHash]);
    }

    // 2.其他类目追加在后面
    GroupWidgetMap::Iterator itemWidget = m_groupWidgetMap.begin();
    while (itemWidget != m_groupWidgetMap.end()) {

        if (itemWidget.value()
                && !itemWidget.value()->isHidden()
                && !m_vGroupWidgets.contains(itemWidget.value()))
            m_vGroupWidgets.push_back(itemWidget.value());

        itemWidget++;
    }

}

void MatchWidget::paintEvent(QPaintEvent *event)
{
// 调试使用，最后发布时需删除todo
#ifdef SHOW_BACKCOLOR
    Q_UNUSED(event);

    QPainter painter(this);

    painter.setBrush(Qt::blue);
    painter.drawRect(rect());
#else
    DWidget::paintEvent(event);
#endif
}
