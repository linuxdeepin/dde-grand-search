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

#include <DApplicationHelper>

#include <QDebug>
#include <QVBoxLayout>
#include <QPainter>
#include <QPaintEvent>

DWIDGET_USE_NAMESPACE

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

void MatchWidget::reLayout()
{
    this->show();

    // 计算匹配结果界面整体高度
    int nTotalHeight = 0;
    for (int i = 0; i < m_vGroupWidgets.size(); i++) {
        GroupWidget *groupWidget = m_vGroupWidgets[i];
        if (groupWidget) {

            // 默认显示组列表内分割横线
            groupWidget->showHorLine(true);

            // 隐藏视觉上最后一个组列表横线
            if (i == m_vGroupWidgets.size() - 1)
                groupWidget->showHorLine(false);

            // 组列表重新布局
            groupWidget->reLayout();
            int nHeight = groupWidget->height();
            qDebug() << QString("MatchWidget::reLayout groupWidget:[%1] height:[%2]").arg(groupWidget->groupName()).arg(nHeight);
            nTotalHeight += nHeight;

            // 添加组列表之间的间隙
            if (i != m_vGroupWidgets.size() - 1)
                nTotalHeight += 5;
        }
    }

    // 设置匹配界面整体高度
    setFixedHeight(nTotalHeight);

    // 打破当前布局，根据最新显示组列表重新布局
    if (m_vLayout) {
        delete m_vLayout;
        m_vLayout = nullptr;
    }
    m_vLayout = new QVBoxLayout(this);
    m_vLayout->setContentsMargins(0, 0, 0, 0);
    m_vLayout->setSpacing(0);
    this->setLayout(m_vLayout);

    for (auto groupWidget : m_vGroupWidgets) {
        if (groupWidget)
            m_vLayout->addWidget(groupWidget, 0);
    }

    layout();
    qDebug() << QString("MatchWidget::autoResize nTotalHeight:[%1]").arg(nTotalHeight);
}

void MatchWidget::setMatchedData(const MatchedItemMap &matchedData)
{
    qDebug() << "MatchWidget::setMatchedData";

    m_MatchedItemMap = matchedData;

    // 动态创建组列表
    MatchedItemMap::Iterator itData = m_MatchedItemMap.begin();
    while (itData != m_MatchedItemMap.end()) {

        QString groupHash = itData.key();
        if (groupHash.isEmpty())
            continue;

        if (!m_GroupWidgetMap.contains(groupHash)) {
            GroupWidget *groupWidget = new GroupWidget(this);
            groupWidget->setGroupName(groupHash);

            m_GroupWidgetMap[groupHash] = groupWidget;
        }

        itData++;
    }

    // 动态显隐组列表，设置匹配结果到组列表
    GroupWidgetMap::Iterator itemWidget = m_GroupWidgetMap.begin();
    while (itemWidget != m_GroupWidgetMap.end()) {

        QString groupHash = itemWidget.key();
        GroupWidget *groupWidget = itemWidget.value();
        if (nullptr == groupWidget || groupHash.isEmpty())
            continue;

        if (m_MatchedItemMap.contains(groupHash))
            groupWidget->setMatchedItems(m_MatchedItemMap[groupHash]);

        // 组内匹配数据若为空，隐藏对应组列表，否则，显示
        groupWidget->setVisible(m_MatchedItemMap.contains(groupHash) && m_MatchedItemMap[groupHash].size() > 0);

        itemWidget++;
    }

    // 对需要显示的组列表排序，在界面中从上到下按 应用 > 文件夹 > 文件 顺序显示
    m_vGroupWidgets.clear();
    for (auto groupHash : m_GroupHashShowOrder) {
        if (!groupHash.isEmpty()
                && m_GroupWidgetMap.contains(groupHash)
                && m_GroupWidgetMap[groupHash]
                && !m_GroupWidgetMap[groupHash]->isHidden())
            m_vGroupWidgets.push_back(m_GroupWidgetMap[groupHash]);
    }

    reLayout();
}

void MatchWidget::initUi()
{

}

void MatchWidget::initConnect()
{
}


void MatchWidget::paintEvent(QPaintEvent *event)
{
#if 0
    Q_UNUSED(event);

    QPainter painter(this);

    painter.setBrush(Qt::blue);
    painter.drawRect(rect());
#else
    DWidget::paintEvent(event);
#endif
}
