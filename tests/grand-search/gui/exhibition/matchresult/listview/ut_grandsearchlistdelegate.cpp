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

#include "gui/exhibition/matchresult/listview/grandsearchlistview.h"
#include "gui/exhibition/matchresult/listview/grandsearchlistdelegate.h"
#include "gui/exhibition/matchresult/listview/grandsearchlistmodel.h"
#include "global/matcheditem.h"
#include "global/builtinsearch.h"
#include "utils/utils.h"

#include "stubext.h"

#include <DStyledItemDelegate>

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QEvent>
#include <QWidget>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QPainter>
#include <QHelpEvent>
#include <QPen>

using namespace testing;
using namespace GrandSearch;
DWIDGET_USE_NAMESPACE;

TEST(GrandSearchListDelegateTest, constructor)
{
    auto view = new GrandSearchListView;
    GrandSearchListDelegate *delegate = view->m_delegate;

    ASSERT_TRUE(view);
    ASSERT_TRUE(delegate);

    delete view;
}

TEST(GrandSearchListDelegateTest, initStyleOption)
{
    GrandSearchListView view;
    GrandSearchListDelegate *delegate = view.m_delegate;

    typedef void (*fptr)(DStyledItemDelegate*,QStyleOptionViewItem *, const QModelIndex &);
    fptr ut_initStyleOption = (fptr)(&DStyledItemDelegate::initStyleOption);

    stub_ext::StubExt stu;

    bool ut_call_initStyleOption = false;
    stu.set_lamda(ut_initStyleOption, [&](){
        ut_call_initStyleOption = true;
    });

    QStyleOptionViewItem option;
    QModelIndex index;

    delegate->initStyleOption(&option, index);
    EXPECT_TRUE(ut_call_initStyleOption);
}

TEST(GrandSearchListDelegateTest, editorEvent)
{
    GrandSearchListView view;
    GrandSearchListDelegate *delegate = view.m_delegate;

    QEvent event(QEvent::FocusIn);
    GrandSearchListModel model(2, 2);
    QStyleOptionViewItem option;
    QModelIndex index;

    bool result = delegate->editorEvent(&event, &model, option, index);
    EXPECT_TRUE(result);
}

TEST(GrandSearchListDelegateTest, paint)
{
    GrandSearchListView view;
    GrandSearchListDelegate *delegate = view.m_delegate;

    stub_ext::StubExt stu;

    bool ut_call_drawSelectState = false;
    stu.set_lamda(&GrandSearchListDelegate::drawSelectState, [&](){
        ut_call_drawSelectState = true;
    });

    bool ut_call_drawSearchResultText = false;
    stu.set_lamda(&GrandSearchListDelegate::drawSearchResultText, [&](){
        ut_call_drawSearchResultText = true;
    });

    bool ut_call_paint = false;
    typedef void (*fptr)(QStyledItemDelegate*,QPainter *, const QStyleOptionViewItem &, const QModelIndex &);
    fptr ut_paint = (fptr)(&QStyledItemDelegate::paint);
    stu.set_lamda(ut_paint, [&](){
        ut_call_paint = true;
    });

    QPainter painter;
    QStyleOptionViewItem option;
    QModelIndex index;

    delegate->paint(&painter, option, index);

    EXPECT_TRUE(ut_call_drawSelectState);
    EXPECT_TRUE(ut_call_drawSearchResultText);
    EXPECT_TRUE(ut_call_paint);
}

TEST(GrandSearchListDelegateTest, sizeHint)
{
    GrandSearchListView view;
    GrandSearchListDelegate *delegate = view.m_delegate;

    QStyleOptionViewItem option;
    QModelIndex index;

    QSize actual = delegate->sizeHint(option, index);
    QSize expect(350, 36);

    EXPECT_EQ(actual.width(), expect.width());
    EXPECT_EQ(actual.height(), expect.height());
}

TEST(GrandSearchListDelegateTest, helpEvent)
{
    GrandSearchListView view;
    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item, item};

    view.setMatchedItems(items);

    GrandSearchListDelegate *delegate = view.m_delegate;

    QHelpEvent helpEventFocusIn(QEvent::FocusIn, QPoint(10, 10), QPoint(100, 100));

    QStyleOptionViewItem option;
    QModelIndex index;

    delegate->helpEvent(&helpEventFocusIn, &view, option, index);

    QHelpEvent helpEventToolTip(QEvent::ToolTip, QPoint(10, 10), QPoint(100, 100));

    index = view.model()->index(0, 0);

    bool result = delegate->helpEvent(&helpEventToolTip, &view, option, index);
    EXPECT_TRUE(result);

    view.m_model->setData(index, QString("TestToolTip"), Qt::ToolTipRole);
    result = delegate->helpEvent(&helpEventToolTip, &view, option, index);
    EXPECT_TRUE(result);

    view.m_model->setData(index, QString(), Qt::ToolTipRole);
    result = delegate->helpEvent(&helpEventToolTip, &view, option, index);
    EXPECT_TRUE(result);
}

TEST(GrandSearchListDelegateTest, drawSelectState)
{
    GrandSearchListView view;
    GrandSearchListDelegate *delegate = view.m_delegate;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    MatchedItems items{item, item};

    view.setMatchedItems(items);

    stub_ext::StubExt stu;

    bool ut_call_save = false;
    stu.set_lamda(&QPainter::save, [&](){
        ut_call_save = true;
    });

    bool ut_call_restore = false;
    stu.set_lamda(&QPainter::restore, [&](){
        ut_call_restore = true;
    });

    QStyleOptionViewItem option;
    QModelIndex index = view.model()->index(0, 0);

    delegate->initStyleOption(&option, index);
    option.state = option.state | QStyle::State_Selected;

    QPainter painter;
    delegate->drawSelectState(&painter, option, index);
    EXPECT_FALSE(ut_call_save);
    EXPECT_FALSE(ut_call_restore);

    option.state = option.state & (~QStyle::State_Selected);
    option.state = option.state | QStyle::State_MouseOver;

    delegate->drawSelectState(&painter, option, index);
    EXPECT_TRUE(ut_call_save);
    EXPECT_TRUE(ut_call_restore);
}

TEST(GrandSearchListDelegateTest, drawSearchResultText)
{
    GrandSearchListView view;
    GrandSearchListDelegate *delegate = view.m_delegate;

    QString searchGroupName(GRANDSEARCH_GROUP_FOLDER);
    MatchedItem item;
    item.name = "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ"
                "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ";
    item.searcher = "com.deepin.dde-grand-search.web-statictext";
    MatchedItems items{item, item};

    view.setMatchedItems(items);

    QModelIndex index = view.model()->index(0, 0);
    view.setCurrentIndex(index);

    QPainter painter;
    QStyleOptionViewItem option;

    view.m_themeType = 2;
    option.widget = &view;
    option.rect = QRect(QPoint(10, 10), QSize(10, 10));
    option.state = option.state | QStyle::State_HasFocus;

    stub_ext::StubExt stu;

    bool ut_call_save = false;
    stu.set_lamda(&QPainter::save, [&](){
        ut_call_save = true;
    });

    bool ut_call_restore = false;
    stu.set_lamda(&QPainter::restore, [&](){
        ut_call_restore = true;
    });

    delegate->drawSearchResultText(&painter, option, index);
    EXPECT_TRUE(ut_call_save);
    EXPECT_TRUE(ut_call_restore);

    ut_call_save = false;
    ut_call_restore = false;

    view.m_themeType = 1;
    delegate->drawSearchResultText(&painter, option, index);
    EXPECT_TRUE(ut_call_save);
    EXPECT_TRUE(ut_call_restore);
}
