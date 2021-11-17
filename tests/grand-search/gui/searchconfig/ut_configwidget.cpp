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

#include "gui/searchconfig/configwidget.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;

TEST(ConfigWidgetTest, constructor)
{
    stub_ext::StubExt stu;

    bool ut_call_init = false;

    stu.set_lamda(ADDR(ConfigWidget, initUI), [&](){
        ut_call_init = true;
    });

    ConfigWidget *w = new ConfigWidget;

    EXPECT_TRUE(w);
    EXPECT_TRUE(ut_call_init);

    delete w;
}

TEST(ConfigWidgetTest, initUI)
{
    ConfigWidget w;

    EXPECT_TRUE(w.m_searchGroupWidget);
    EXPECT_TRUE(w.m_searchPlanWidget);
}
