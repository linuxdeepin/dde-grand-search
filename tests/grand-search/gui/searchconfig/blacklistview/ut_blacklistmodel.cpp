/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhaohanxi<zhaohanxi@uniontech.com>
 *
 * Maintainer: zhaohanxi<zhaohanxi@uniontech.com>
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

#include "global/grandsearch_global.h"
#include "gui/searchconfig/blacklistview/blacklistmodel.h"

#include "stubext.h"

#include <gtest/gtest.h>

GRANDSEARCH_USE_NAMESPACE

TEST(BlackListModelTest, constructor)
{
    BlackListModel model(10, 10);

    EXPECT_EQ(model.rowCount(), 10);
    EXPECT_EQ(model.columnCount(), 10);
}

TEST(BlackListModelTest, flags)
{
    stub_ext::StubExt stu;
    QVariant data;
    data.setValue(QString("test"));
    stu.set_lamda(&QModelIndex::data, [&](){
        return data;
    });

    stu.set_lamda(VADDR(QStandardItemModel, flags), [](){
        return Qt::NoItemFlags;
    });

    BlackListModel model(10, 10);
    EXPECT_TRUE(model.flags(QModelIndex()).testFlag(Qt::ItemIsSelectable));

    data.clear();
    data.setValue(QString(""));
    EXPECT_FALSE(model.flags(QModelIndex()).testFlag(Qt::ItemIsSelectable));
}

TEST(BlackListModelTest, mimeTypes)
{
    BlackListModel model(10, 10);
    stub_ext::StubExt stu;
    bool ut_call_mimeTypes = false;
    stu.set_lamda(VADDR(QStandardItemModel, mimeTypes), [&]{
        ut_call_mimeTypes = true;
        return QStringList();
    });
    model.mimeTypes();
    EXPECT_TRUE(ut_call_mimeTypes);
}
