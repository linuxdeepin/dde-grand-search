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
#include "viewmore/viewmorebutton.h"
#include "utils/utils.h"
#include "gui/datadefine.h"

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

#define ListItemHeight 36
#define ScrollBarToListRightSpace 2

MatchWidgetPrivate::MatchWidgetPrivate(MatchWidget *parent)
    : q_p(parent)
{

}

MatchWidget::MatchWidget(QWidget *parent)
    : DWidget(parent)
    , d_p(new MatchWidgetPrivate(this))
{
    m_groupHashShowOrder << GRANDSEARCH_GROUP_BEST <<GRANDSEARCH_GROUP_APP << GRANDSEARCH_GROUP_SETTING << GRANDSEARCH_GROUP_WEB
                         << GRANDSEARCH_GROUP_FILE_VIDEO << GRANDSEARCH_GROUP_FILE_AUDIO << GRANDSEARCH_GROUP_FILE_PICTURE
                         << GRANDSEARCH_GROUP_FILE_DOCUMNET << GRANDSEARCH_GROUP_FOLDER << GRANDSEARCH_GROUP_FILE;
    initUi();
    initConnect();

    reLayout();
}

MatchWidget::~MatchWidget()
{

}

void MatchWidget::appendMatchedData(const MatchedItemMap &matchedData)
{
    bool bNeedRelayout = false;

    // ????????????
    MatchedItemMap::ConstIterator itData = matchedData.begin();
    while (itData != matchedData.end()) {

        // ??????groupHash????????????????????????????????????????????????????????????????????????
        GroupWidget* groupWidget = createGroupWidget(itData.key());
        if (!groupWidget) {
            itData++;
            continue;
        }

        // ????????????????????????????????????
        groupWidget->appendMatchedItems(itData.value(), itData.key());

        // ???????????????????????????????????????
        groupWidget->setVisible(groupWidget->itemCount() > 0);

        // ????????????????????????????????????????????????
        if (!bNeedRelayout && itData.value().size() > 0)
            bNeedRelayout = true;

        itData++;
    }

    // ????????????????????????????????????
    sortVislibleGroupList();

    // ??????????????????
    if (bNeedRelayout)
        reLayout();

    // ???????????????????????????????????????????????????????????????????????????
    if (!m_customSelected) {
        clearSelectItem();
        selectFirstItem();
    }
}

void MatchWidget::clearMatchedData()
{
    Q_ASSERT(m_scrollAreaContent);

    m_vGroupWidgets.clear();

    // ?????????????????????????????????
    for (GroupWidgetMap::Iterator it = m_groupWidgetMap.begin(); it != m_groupWidgetMap.end(); ++it) {
        if (it.value())
            it.value()->clear();
    }

    // ????????????????????????????????????
    if (Q_LIKELY(m_scrollAreaContent))
        m_scrollAreaContent->setFixedHeight(0);

    layout();

    m_customSelected = false;
}

void MatchWidget::onSearchCompleted()
{
    if (!m_vGroupWidgets.isEmpty())
        return;

    emit sigShowNoContent(true);
}

void MatchWidget::onShowMore()
{
    reLayout();
}

void MatchWidget::selectNextItem()
{
    if (this->isHidden())
        return;

    for (int i = 0; i < m_vGroupWidgets.count(); ++i) {

        if (!hasSelectItem(i))
            continue;

        // ???????????????????????????
        GroupWidget *group = m_vGroupWidgets.at(i);
        GrandSearchListView *listView = group->getListView();
        ViewMoreButton *viewMoreBtn = group->getViewMoreButton();

        int nextRow = -1;
        if (viewMoreBtn->isSelected()) {
            nextRow = 0;
            viewMoreBtn->setSelected(false);
        } else {
            const QModelIndex &index = listView->currentIndex();
            nextRow = index.row() + 1;
        }

        const QModelIndex &nextIndex = listView->model()->index(nextRow, 0);
        if (nextIndex.isValid()) {
            listView->setCurrentIndex(nextIndex);
            m_customSelected = true;
            currentIndexChanged(group->searchGroupName(), nextIndex);
            break;
        } else {
            // ?????????????????????????????????????????????????????????????????????????????????
            if (i < m_vGroupWidgets.count() - 1) {
                bool selected = selectFirstItem(++i);
                if (Q_LIKELY(selected)) {
                    // ???????????????????????????????????????????????????
                    listView->setCurrentIndex(QModelIndex());
                    m_customSelected = true;
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

    for (int i = 0; i < m_vGroupWidgets.count(); ++i) {

        if (!hasSelectItem(i))
            continue;

        // ???????????????????????????
        GroupWidget *group = m_vGroupWidgets.at(i);
        GrandSearchListView *listView = group->getListView();
        ViewMoreButton *viewMoreBtn = group->getViewMoreButton();

        const QModelIndex &index = listView->currentIndex();
        int previousRow = index.row() - 1;
        const QModelIndex &previousIndex = listView->model()->index(previousRow, 0);
        if (previousIndex.isValid()) {
            listView->setCurrentIndex(previousIndex);
            m_customSelected = true;
            currentIndexChanged(group->searchGroupName(), previousIndex);
            break;
        } else if (viewMoreBtn->isVisible() && !viewMoreBtn->isSelected()) {
            viewMoreBtn->setSelected(true);
            listView->setCurrentIndex(QModelIndex());
            currentIndexChanged(group->searchGroupName(), QModelIndex());
            m_customSelected = true;
            break;
        } else {
            // ?????????????????????????????????????????????????????????????????????????????????
            if (i > 0) {
                bool selected = selectLastItem(--i);
                if (Q_LIKELY(selected)) {
                    // ???????????????????????????????????????????????????
                    listView->setCurrentIndex(QModelIndex());
                    viewMoreBtn->setSelected(false);
                    m_customSelected = true;
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
    for (int i = 0; i < m_vGroupWidgets.count(); ++i) {
        if (hasSelectItem(i)) {
            ViewMoreButton *viewMoreBtn = m_vGroupWidgets.at(i)->getViewMoreButton();
            Q_ASSERT(viewMoreBtn);
            GrandSearchListView *listView = m_vGroupWidgets.at(i)->getListView();
            Q_ASSERT(listView);

            if (viewMoreBtn->isSelected()) {
                viewMoreBtn->click();
                break;
            }

            MatchedItem item = listView->currentIndex().data(DATA_ROLE).value<MatchedItem>();
            Utils::openMatchedItem(item);
            emit sigCloseWindow();
            break;
        }
    }
}

void MatchWidget::onPreviewStateChanged(const bool preview)
{
    if (this->isHidden() || preview == m_isPreviewItem)
        return ;

    m_isPreviewItem = preview;

    for (int i = 0; i < m_vGroupWidgets.count(); ++i) {

        GroupWidget *group = m_vGroupWidgets.at(i);
        Q_ASSERT(group);

        GrandSearchListView *listView = group->getListView();
        Q_ASSERT(listView);

        listView->updatePreviewItemState(preview);
    }
}

void MatchWidget::onSelectItemByMouse(const MatchedItem &item)
{
    QString searchGroupName;

    // ??????????????????????????????
    GrandSearchListView* listView = qobject_cast<GrandSearchListView*>(sender());
    if (listView) {
        for (int i = 0; i < m_vGroupWidgets.count(); ++i) {
            if (hasSelectItem(i)) {
                ViewMoreButton *viewMoreBtn = m_vGroupWidgets.at(i)->getViewMoreButton();
                Q_ASSERT(viewMoreBtn);
                viewMoreBtn->setSelected(false);

                GrandSearchListView *tmpListView = m_vGroupWidgets.at(i)->getListView();
                Q_ASSERT(tmpListView);
                if (listView != tmpListView) {
                    tmpListView->setCurrentIndex(QModelIndex());
                } else {
                    searchGroupName = m_vGroupWidgets.at(i)->searchGroupName();
                    m_customSelected = true;
                }
            }
        }
    }
    // ???????????????????????????
    emit sigCurrentItemChanged(searchGroupName, item);
}

bool MatchWidget::selectFirstItem(int groupNumber)
{
    if (this->isHidden())
        return false;

    for (int i = groupNumber; i < m_vGroupWidgets.count(); ++i) {

        GroupWidget *group = m_vGroupWidgets.at(i);
        Q_ASSERT(group);

        ViewMoreButton *viewMoreBtn = group->getViewMoreButton();
        Q_ASSERT(viewMoreBtn);

        if (viewMoreBtn->isVisible()) {
            viewMoreBtn->setSelected(true);
            currentIndexChanged(group->searchGroupName(), QModelIndex());
            return true;
        }

        GrandSearchListView *listView = group->getListView();
        Q_ASSERT(listView);

        if (Q_LIKELY(listView->rowCount() > 0)) {
            const QModelIndex &index = listView->model()->index(0, 0);
            if (Q_LIKELY(index.isValid())) {
                listView->setCurrentIndex(index);
                currentIndexChanged(group->searchGroupName(), index);
                return true;
            }
        }
    }
    return false;
}

bool MatchWidget::selectLastItem(int groupNumber)
{
    for (int i = groupNumber; i < m_vGroupWidgets.count(); --i) {

        GroupWidget *group = m_vGroupWidgets.at(i);
        Q_ASSERT(group);

        GrandSearchListView *listView = group->getListView();
        Q_ASSERT(listView);

        if (Q_LIKELY(listView->rowCount() > 0)) {
            const QModelIndex &index = listView->model()->index(listView->rowCount() - 1, 0);
            if (Q_LIKELY(index.isValid())) {
                listView->setCurrentIndex(index);
                currentIndexChanged(group->searchGroupName(), index);
                return true;
            }
        }
    }
    return false;
}

bool MatchWidget::hasSelectItem()
{
    for (int i = 0; i < m_vGroupWidgets.count(); ++i) {
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

    ViewMoreButton *viewMoreBtn = group->getViewMoreButton();
    if (Q_UNLIKELY(!viewMoreBtn))
        return false;

    if (viewMoreBtn->isSelected())
        return true;

    GrandSearchListView *listView = group->getListView();
    if (Q_UNLIKELY(!listView))
        return false;

    const QModelIndex &index = listView->currentIndex();
    if (index.isValid())
        return true;

    return false;
}

void MatchWidget::clearSelectItem()
{
    bool selectChanged = false;

    for (int i = 0; i < m_vGroupWidgets.count(); ++i) {
        if (hasSelectItem(i)) {
            ViewMoreButton *viewMoreBtn = m_vGroupWidgets.at(i)->getViewMoreButton();
            Q_ASSERT(viewMoreBtn);

            GrandSearchListView *tmpListView = m_vGroupWidgets.at(i)->getListView();
            Q_ASSERT(tmpListView);

            viewMoreBtn->setSelected(false);
            tmpListView->setCurrentIndex(QModelIndex());
            selectChanged = true;
        }
    }

    if (selectChanged) {
        // ???????????????????????????
        MatchedItem item;
        emit sigCurrentItemChanged(QString(), item);
    }
}

void MatchWidget::adjustScrollBar()
{
    int nCurSelHeight = 0;

    //??????????????????Item???contentWidget??????????????????
    for (auto group : m_vGroupWidgets) {
        if (Q_UNLIKELY((!group)))
            continue;

        ViewMoreButton *viewMoreBtn = group->getViewMoreButton();
        if (Q_UNLIKELY(!viewMoreBtn))
            continue;

        GrandSearchListView* listView = group->getListView();
        if (Q_UNLIKELY(!listView))
            continue;

        if (!listView->currentIndex().isValid() && !viewMoreBtn->isSelected()) {
            nCurSelHeight += group->height();
        } else {
            nCurSelHeight += group->getCurSelectHeight();
            break;
        }
    }

    int nScrollAreaHeight = m_scrollArea->height();// ??????????????????
    int nCurPosValue = m_scrollArea->verticalScrollBar()->value();//?????????????????????
    int nCurHeight = nScrollAreaHeight + nCurPosValue;//???????????????????????????????????????

    int nOffset = 0;// ??????????????????????????????
    int nNewPosValue = 0;// ?????????????????????

    // ???????????????????????????????????????????????????????????????
    if (nCurSelHeight > nCurHeight) {
        nNewPosValue = nCurSelHeight - nScrollAreaHeight + 2;
        m_scrollArea->verticalScrollBar()->setValue(nNewPosValue);
    }
    // ???????????????????????????????????????????????????????????????
    else if (nCurSelHeight <= nCurPosValue + ListItemHeight) {
        nOffset = nCurSelHeight - nCurPosValue;
        nNewPosValue = nCurPosValue + nOffset - ListItemHeight;

        if (nNewPosValue < ListItemHeight)
            nNewPosValue = 0;
        m_scrollArea->verticalScrollBar()->setValue(nNewPosValue);
    }

//    int nMin = m_scrollArea->verticalScrollBar()->minimum();
//    int nMax = m_scrollArea->verticalScrollBar()->maximum();
//    qDebug() << QString("nMin:%1 nMax:%2 nCurSelHeight%3 nCurPosValue:%4 nNewPosValue:%5").arg(nMin).arg(nMax).arg(nCurSelHeight).arg(nCurPosValue).arg(nNewPosValue);
}

void MatchWidget::currentIndexChanged(const QString &searchGroupName, const QModelIndex &index)
{
    MatchedItem item;
    if (index.isValid())
        item = index.data(DATA_ROLE).value<MatchedItem>();

    emit sigCurrentItemChanged(searchGroupName, item);
}

void MatchWidget::initUi()
{
    m_vMainLayout = new QVBoxLayout(this);
    m_vMainLayout->setContentsMargins(0, 0, 0, 0);
    m_vMainLayout->setSpacing(0);
    m_scrollArea = new DScrollArea(this);
    m_scrollArea->setFocusPolicy(Qt::NoFocus);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setWidgetResizable(true);
    m_scrollAreaContent = new DWidget();
    m_scrollAreaContent->setFocusPolicy(Qt::NoFocus);
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

    // ????????????????????????????????????
    int nTotalHeight = 0;
    for (int i = 0; i < m_vGroupWidgets.size(); i++) {
        GroupWidget *groupWidget = m_vGroupWidgets[i];
        if (Q_LIKELY(groupWidget)) {

            // ???????????????????????????????????????
            groupWidget->showHorLine(true);

            // ?????????????????????????????????????????????
            if (i == m_vGroupWidgets.size() - 1)
                groupWidget->showHorLine(false);

            // ????????????????????????
            groupWidget->reLayout();
            int nHeight = groupWidget->height();
            nTotalHeight += nHeight;
        }
    }

    // ??????????????????????????????????????????
    if (Q_LIKELY(m_scrollAreaContent))
        m_scrollAreaContent->setFixedHeight(nTotalHeight);

    // ???????????????????????????????????????????????????????????????
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

GroupWidget *MatchWidget::createGroupWidget(const QString &searchGroupName)
{
    if (Q_UNLIKELY(searchGroupName.isEmpty()))
        return nullptr;

    if (!m_groupWidgetMap[searchGroupName]) {

        GroupWidget *groupWidget = new GroupWidget(m_scrollAreaContent);
        groupWidget->setFocusPolicy(Qt::NoFocus);

        connect(groupWidget, &GroupWidget::showMore, this, &MatchWidget::reLayout);
        connect(groupWidget, &GroupWidget::sigCurrentItemChanged, this, &MatchWidget::sigCurrentItemChanged);

        GrandSearchListView *listView = groupWidget->getListView();
        Q_ASSERT(listView);
        listView->updatePreviewItemState(m_isPreviewItem);
        connect(listView, &GrandSearchListView::sigCurrentItemChanged, this, &MatchWidget::onSelectItemByMouse);
        connect(listView, &GrandSearchListView::sigItemClicked, this, &MatchWidget::sigCloseWindow);

        groupWidget->setSearchGroupName(searchGroupName);

        const QString &groupName = GroupWidget::convertDisplayName(searchGroupName);
        groupWidget->setGroupName(groupName);
        m_groupWidgetMap[searchGroupName] = groupWidget;

        return groupWidget;
    }

    return m_groupWidgetMap[searchGroupName];
}

void MatchWidget::sortVislibleGroupList()
{
    m_vGroupWidgets.clear();
    for (auto searchGroupName : m_groupHashShowOrder) {
        if (m_groupWidgetMap[searchGroupName] && !m_groupWidgetMap[searchGroupName]->isHidden())
            m_vGroupWidgets.push_back(m_groupWidgetMap[searchGroupName]);
    }

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
    DWidget::paintEvent(event);
}

void MatchWidget::resizeEvent(QResizeEvent *event)
{
    // ??????resizeEvent????????????????????????????????????????????????????????????????????????
    for (GroupWidgetMap::Iterator it = m_groupWidgetMap.begin(); it !=m_groupWidgetMap.end(); ++it) {
        if (it.value()) {
            it.value()->setFixedWidth(rect().width() - ScrollBarToListRightSpace);// ?????????????????????????????????2px??????
        }
    }

    DWidget::resizeEvent(event);
}
