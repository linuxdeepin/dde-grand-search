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

    // 设置滚动区域内容部件整体高度
    if (m_scrollAreaContent)
        m_scrollAreaContent->setFixedHeight(nTotalHeight);

    // 打破当前布局，根据最新显示组列表重新布局
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

        if (!m_GroupWidgetMap.contains(groupHash) || nullptr == m_GroupWidgetMap[groupHash]) {
            GroupWidget *groupWidget = new GroupWidget(m_scrollAreaContent);
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
        if (!groupWidget) {
            itemWidget++;
            continue;
        }

        if (m_MatchedItemMap.contains(groupHash))
            groupWidget->setMatchedItems(m_MatchedItemMap[groupHash]);

        // 匹配结果组内有数据，显示对应组列表，否则，不显示
        groupWidget->setVisible(m_MatchedItemMap.contains(groupHash) && m_MatchedItemMap[groupHash].size() > 0);

        itemWidget++;
    }

    // 对需要显示的组列表排序
    // a.优先在界面中显示应用、文件夹和文件组，显示顺序为: 应用 > 文件夹 > 文件
    m_vGroupWidgets.clear();
    for (auto groupHash : m_GroupHashShowOrder) {
        if (m_GroupWidgetMap[groupHash] && !m_GroupWidgetMap[groupHash]->isHidden())
            m_vGroupWidgets.push_back(m_GroupWidgetMap[groupHash]);
    }

    // b.其他组追加在后面
    itemWidget = m_GroupWidgetMap.begin();
    while (itemWidget != m_GroupWidgetMap.end()) {

        if (itemWidget.value()
                && !itemWidget.value()->isHidden()
                && !m_vGroupWidgets.contains(itemWidget.value()))
            m_vGroupWidgets.push_back(itemWidget.value());

        itemWidget++;
    }

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


void MatchWidget::paintEvent(QPaintEvent *event)
{
#ifdef SHOW_BACKCOLOR
    Q_UNUSED(event);

    QPainter painter(this);

    painter.setBrush(Qt::blue);
    painter.drawRect(rect());
#else
    DWidget::paintEvent(event);
#endif
}
