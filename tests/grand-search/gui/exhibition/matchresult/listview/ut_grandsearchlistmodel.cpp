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

#include "gui/exhibition/matchresult/listview/grandsearchlistmodel.h"
#include "global/matcheditem.h"
#include "utils/utils.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QEvent>
#include <QWidget>

using namespace testing;
using namespace GrandSearch;

TEST(GrandSearchListModelTest, constructor)
{
    GrandSearchListModel *m = new GrandSearchListModel(2, 2);

    ASSERT_TRUE(m);

    delete m;
}

TEST(GrandSearchListModelTest, setData)
{
    GrandSearchListModel m(2, 2);

    typedef bool (*fptr)(QStandardItemModel*,const QModelIndex &, const QVariant &, int);
    fptr ut_setData = (fptr)(&QStandardItemModel::setData);

    stub_ext::StubExt stu;

    bool ut_call_setData = false;
    stu.set_lamda(ut_setData, [&](){
        ut_call_setData = true;
        return true;
    });

    m.setData(QModelIndex(), 0);
    EXPECT_TRUE(ut_call_setData);
}

TEST(GrandSearchListModelTest, flags)
{
    GrandSearchListModel m(2, 2);

    auto flags = m.flags(QModelIndex());
    EXPECT_TRUE(flags.testFlag(Qt::ItemIsDropEnabled));

    auto index = m.index(0, 0);
    flags = m.flags(index);
    EXPECT_TRUE(flags.testFlag(Qt::ItemIsDragEnabled));
}
