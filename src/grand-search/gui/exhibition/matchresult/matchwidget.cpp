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
#include <QScrollBar>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

#define GroupSpace 5
#define ListItemHeight 36

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

    // 确保有结果被选中
    if (!hasSelectItem())
        selectFirstItem();
}

void MatchWidget::clearMatchedData()
{
    Q_ASSERT(m_scrollAreaContent);

    m_vGroupWidgets.clear();

    // 清空并隐藏所有类目列表
    for (GroupWidgetMap::Iterator it= m_groupWidgetMap.begin(); it!=m_groupWidgetMap.end(); ++it) {
        if (it.value())
            it.value()->clear();
    }

    // 重置滚动区域内容部件高度
    if (Q_LIKELY(m_scrollAreaContent))
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

void MatchWidget::selectNextItem()
{
    if (this->isHidden())
        return;

    for (int i=0; i<m_vGroupWidgets.count(); ++i) {

        if (!hasSelectItem(i))
            continue;

        // 选择项在当前列表中
        GroupWidget *group = m_vGroupWidgets.at(i);
        GrandSearchListview *listView = group->getListView();
        const QModelIndex &index = listView->currentIndex();

        int nextRow = index.row() + 1;
        const QModelIndex &nextIndex = listView->model()->index(nextRow, 0);
        if (nextIndex.isValid()) {
            qDebug() << "select item:" << group->groupName() << nextIndex.row()/* << nextIndex.data(DATA_ROLE).value<MatchedItem>().name*/;
            listView->setCurrentIndex(nextIndex);
            break;
        } else {
            // 选择项是当前列表的最后一项，需要选择下一个列表的第一项
            if (i < m_vGroupWidgets.count() - 1) {
                bool selected = selectFirstItem(++i);
                if (Q_LIKELY(selected)) {
                    // 切换成功后，需要将当前列表选择为空
                    listView->setCurrentIndex(QModelIndex());
                } else {
                    qWarning() << "select next failed";
                }
                break;
            } else {
                qInfo() << "it's the last one";
            }
        }
    }

    adjustScrollBar();
}

void MatchWidget::selectPreviousItem()
{
    if (this->isHidden())
        return;

    for (int i=0; i<m_vGroupWidgets.count(); ++i) {

        if (!hasSelectItem(i))
            continue;

        // 选择项在当前列表中
        GroupWidget *group = m_vGroupWidgets.at(i);
        GrandSearchListview *listView = group->getListView();
        const QModelIndex &index = listView->currentIndex();

        int previousRow = index.row() - 1;
        const QModelIndex &previousIndex = listView->model()->index(previousRow, 0);
        if (previousIndex.isValid()) {
            qDebug() << "select item:" << group->groupName() << previousIndex.row()/* << previousIndex.data(DATA_ROLE).value<MatchedItem>().name*/;
            listView->setCurrentIndex(previousIndex);
            break;
        } else {
            // 选择项是当前列表的第一项，需要选择上一个列表的最后一项
            if (i > 0) {
                bool selected = selectLastItem(--i);
                if (Q_LIKELY(selected)) {
                    // 切换成功后，需要将当前列表选择为空
                    listView->setCurrentIndex(QModelIndex());
                } else {
                    qWarning() << "select previous failed";
                }
                break;
            } else {
                qInfo() << "it's the first one";
            }
        }
    }

    adjustScrollBar();
}

void MatchWidget::handleItem()
{
    // todo handle item
}

void MatchWidget::onSelectItemByMouse(const GrandSearchListview *listView)
{
    for (int i=0; i<m_vGroupWidgets.count(); ++i) {
        if (hasSelectItem(i)) {
            GrandSearchListview *tmpListView = m_vGroupWidgets.at(i)->getListView();
            Q_ASSERT(tmpListView);
            if (listView != tmpListView)
                tmpListView->setCurrentIndex(QModelIndex());
        }
    }
}

bool MatchWidget::selectFirstItem(int groupNumber)
{
    if (this->isHidden())
        return false;

    for (int i=groupNumber; i<m_vGroupWidgets.count(); ++i) {

        GroupWidget *group = m_vGroupWidgets.at(i);
        Q_ASSERT(group);

        GrandSearchListview *listView = group->getListView();
        Q_ASSERT(listView);

        if (Q_LIKELY(listView->rowCount() > 0)) {
            const QModelIndex &index = listView->model()->index(0, 0);
            if (Q_LIKELY(index.isValid())) {
                qDebug() << "select item:" << group->groupName() << index.row();
                listView->setCurrentIndex(index);
                return true;
            }
        }
    }
    return false;
}

bool MatchWidget::selectLastItem(int groupNumber)
{
    for (int i=groupNumber; i<m_vGroupWidgets.count(); --i) {

        GroupWidget *group = m_vGroupWidgets.at(i);
        Q_ASSERT(group);

        GrandSearchListview *listView = group->getListView();
        Q_ASSERT(listView);

        if (Q_LIKELY(listView->rowCount() > 0)) {
            const QModelIndex &index = listView->model()->index(listView->rowCount() - 1, 0);
            if (Q_LIKELY(index.isValid())) {
                qDebug() << "select item:" << group->groupName() << index.row();
                listView->setCurrentIndex(index);
                return true;
            }
        }
    }
    return false;
}

bool MatchWidget::hasSelectItem()
{
    for (int i=0; i<m_vGroupWidgets.count(); ++i) {
        if (hasSelectItem(i))
            return true;
    }
    return false;
}

bool MatchWidget::hasSelectItem(int groupNumber)
{
    GroupWidget *group = m_vGroupWidgets.at(groupNumber);
    if (Q_UNLIKELY(!group))
        return false;

    GrandSearchListview *listView = group->getListView();
    if (Q_UNLIKELY(!listView))
        return false;

    const QModelIndex &index = listView->currentIndex();
    if (index.isValid())
        return true;

    return false;
}

void MatchWidget::adjustScrollBar()
{
    int nCurSelHeight = 0;

    //计算当前选中Item在contentWidget整体高度位置
    for (auto group : m_vGroupWidgets) {
        if (Q_UNLIKELY((!group)))
            continue;

        GrandSearchListview* listView = group->getListView();
        if (Q_UNLIKELY(!listView))
            continue;

        if (!listView->currentIndex().isValid()) {
            nCurSelHeight += group->height();
        }
        else {
            if (nCurSelHeight != 0)
                nCurSelHeight += GroupSpace;

            nCurSelHeight += group->getCurSelectHeight();
            break;
        }
    }

    int nScrollAreaHeight = m_scrollArea->height();// 滚动区域高度
    int nCurPosValue = m_scrollArea->verticalScrollBar()->value();//滚动条当前位置
    int nCurHeight = nScrollAreaHeight + nCurPosValue;//滚动条当前可显示的内容高度

    int nOffset = 0;// 记录需要滚动的偏移量
    int nNewPosValue = 0;// 滚动条位置新值

    // 选中行定位到当前滚动区域底部，逐个向下滚动
    if (nCurSelHeight > nCurHeight) {
        nOffset = nCurSelHeight - nCurHeight;
        nNewPosValue = nCurPosValue + nOffset;
        m_scrollArea->verticalScrollBar()->setValue(nNewPosValue);
    }
    // 选中行定位到当前滚动区域顶部，逐个向上滚动
    else if (nCurSelHeight <= nCurPosValue + ListItemHeight) {
        nOffset = nCurSelHeight - nCurPosValue;
        nNewPosValue = nCurPosValue + nOffset - ListItemHeight;

        if (nNewPosValue < ListItemHeight)
            nNewPosValue = 0;
        m_scrollArea->verticalScrollBar()->setValue(nNewPosValue);
    }
//    float fStep = m_scrollArea->verticalScrollBar()->singleStep();
//    int nMin = m_scrollArea->verticalScrollBar()->minimum();
//    int nMax = m_scrollArea->verticalScrollBar()->maximum();
//    qDebug() << QString("nStep:%1 nMin:%2 nMax:%3 nCurPosValue:%4 nNewPosValue:%5").arg(fStep).arg(nMin).arg(nMax).arg(nCurPosValue).arg(nNewPosValue);
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
    Q_ASSERT(m_scrollAreaContent);

    this->show();

    // 计算匹配结果界面整体高度
    int nTotalHeight = 0;
    for (int i = 0; i < m_vGroupWidgets.size(); i++) {
        GroupWidget *groupWidget = m_vGroupWidgets[i];
        if (Q_LIKELY(groupWidget)) {

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
                nTotalHeight += GroupSpace;
        }
    }

    // 设置滚动区域内容部件整体高度
    if (Q_LIKELY(m_scrollAreaContent))
        m_scrollAreaContent->setFixedHeight(nTotalHeight);

    // 打破当前布局，根据最新显示类目列表重新布局
    if (Q_LIKELY(m_vScrollLayout)) {
        delete m_vScrollLayout;
        m_vScrollLayout = nullptr;
    }
    m_vScrollLayout = new QVBoxLayout(m_scrollAreaContent);
    m_vScrollLayout->setContentsMargins(0, 0, 0, 0);
    m_vScrollLayout->setSpacing(0);
    m_scrollAreaContent->setLayout(m_vScrollLayout);

    for (auto groupWidget : m_vGroupWidgets) {
        if (Q_LIKELY(groupWidget))
            m_vScrollLayout->addWidget(groupWidget, 0);
    }

    layout();
}

GroupWidget *MatchWidget::createGroupWidget(const QString &groupHash)
{
    if (Q_UNLIKELY(groupHash.isEmpty()))
        return nullptr;

    if (!m_groupWidgetMap[groupHash]) {
        GroupWidget* groupWidget = new GroupWidget(m_scrollAreaContent);
        connect(groupWidget, &GroupWidget::showMore, this, &MatchWidget::reLayout);

        GrandSearchListview *listView = groupWidget->getListView();
        Q_ASSERT(listView);
        connect(listView, &GrandSearchListview::sigSelectItemByMouse, this, &MatchWidget::onSelectItemByMouse);

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
