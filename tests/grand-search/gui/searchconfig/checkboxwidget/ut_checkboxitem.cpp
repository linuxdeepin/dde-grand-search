/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liqianga@uniontech.com>
 *
 * Maintainer: liuzhangjian<liqianga@uniontech.com>
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
#include "gui/searchconfig/checkboxwidget/checkboxitem.h"
#include "stubext.h"

#include <gtest/gtest.h>

#include <QPaintEvent>

using namespace testing;
GRANDSEARCH_USE_NAMESPACE

TEST(CheckBoxItemTest, setChecked)
{
    CheckBoxItem item("test");
    item.setChecked(true);
    EXPECT_TRUE(item.m_checkBox->isChecked());
}

TEST(CheckBoxItemTest, setTopRound)
{
    CheckBoxItem item("test");
    item.setTopRound(true);
    EXPECT_TRUE(item.m_topRound);
}

TEST(CheckBoxItemTest, setBottomRound)
{
    CheckBoxItem item("test");
    item.setBottomRound(true);
    EXPECT_TRUE(item.m_bottomRound);
}

TEST(CheckBoxItemTest, text)
{
    CheckBoxItem item("test");
    EXPECT_EQ(item.text(), "test");
}

TEST(CheckBoxItemTest, paintEvent1)
{
    QPaintEvent event(QRect(QPoint(10, 10), QSize(20, 20)));
    CheckBoxItem item("test");
    EXPECT_NO_FATAL_FAILURE(item.paintEvent(&event));
}

TEST(CheckBoxItemTest, paintEvent2)
{
    QPaintEvent event(QRect(QPoint(10, 10), QSize(20, 20)));
    CheckBoxItem item("test");
    item.m_topRound = true;
    item.m_bottomRound = true;

    EXPECT_NO_FATAL_FAILURE(item.paintEvent(&event));
}
