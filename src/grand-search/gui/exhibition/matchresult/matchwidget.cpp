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
    connect(MatchController::instance(), &MatchController::matchedResult, this, &MatchWidget::setMatchedData);
    connect(QueryController::instance(), &QueryController::missionIdChanged, this, &MatchWidget::clearMatchedData);
}

void MatchWidget::setMatchedData(const MatchedItemMap &matchedData)
{
    qDebug() << "MatchWidget::setMatchedData";

    m_matchedItemMap = matchedData;

    // 动态创建类目列表
    MatchedItemMap::Iterator itData = m_matchedItemMap.begin();
    while (itData != m_matchedItemMap.end()) {

        QString groupHash = itData.key();
        if (groupHash.isEmpty())
            continue;

        if (!m_groupWidgetMap.contains(groupHash) || nullptr == m_groupWidgetMap[groupHash]) {
            GroupWidget *groupWidget = new GroupWidget(m_scrollAreaContent);
            connect(groupWidget, &GroupWidget::showMore, this, &MatchWidget::reLayout);

            groupWidget->setGroupName(groupHash);

            m_groupWidgetMap[groupHash] = groupWidget;
        }

        itData++;
    }

    // 动态显隐类目列表，设置匹配结果到类目列表
    GroupWidgetMap::Iterator itemWidget = m_groupWidgetMap.begin();
    while (itemWidget != m_groupWidgetMap.end()) {

        QString groupHash = itemWidget.key();
        GroupWidget *groupWidget = itemWidget.value();
        if (!groupWidget) {
            itemWidget++;
            continue;
        }

        if (m_matchedItemMap.contains(groupHash))
            groupWidget->setMatchedItems(m_matchedItemMap[groupHash]);

        // 匹配结果类目内有数据，显示对应类目列表，否则，不显示
        groupWidget->setVisible(m_matchedItemMap.contains(groupHash) && m_matchedItemMap[groupHash].size() > 0);

        itemWidget++;
    }

    // 对需要显示的类目列表排序
    // a.优先在界面中显示应用、文件夹和文件类目，显示顺序为: 应用 > 文件夹 > 文件
    m_vGroupWidgets.clear();
    for (auto groupHash : m_groupHashShowOrder) {
        if (m_groupWidgetMap[groupHash] && !m_groupWidgetMap[groupHash]->isHidden())
            m_vGroupWidgets.push_back(m_groupWidgetMap[groupHash]);
    }

    // b.其他类目追加在后面
    itemWidget = m_groupWidgetMap.begin();
    while (itemWidget != m_groupWidgetMap.end()) {

        if (itemWidget.value()
                && !itemWidget.value()->isHidden()
                && !m_vGroupWidgets.contains(itemWidget.value()))
            m_vGroupWidgets.push_back(itemWidget.value());

        itemWidget++;
    }

    reLayout();

    //有搜索结果 显示展示界面
    MainWindow::instance()->showExhitionWidget(m_vGroupWidgets.size() > 0);
}

void MatchWidget::clearMatchedData()
{
    m_matchedItemMap.clear();
    m_vGroupWidgets.clear();

    MatchedItems items;
    items.clear();

    //清空并隐藏所有类目列表
    for (GroupWidgetMap::Iterator it= m_groupWidgetMap.begin(); it!=m_groupWidgetMap.end(); ++it) {
        if (it.value()) {
            it.value()->setMatchedItems(items);
            it.value()->hide();
        }
    }

    //重置滚动区域内容部件高度
    if (m_scrollAreaContent)
        m_scrollAreaContent->setFixedHeight(0);

    layout();
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
            qDebug() << QString("MatchWidget::reLayout groupWidget:[%1] height:[%2]").arg(groupWidget->groupName()).arg(nHeight);
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
    qDebug() << QString("MatchWidget::reLayout nTotalHeight:[%1]").arg(nTotalHeight);
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
