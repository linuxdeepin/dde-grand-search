/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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

#include "text-preview/textview.h"

#include <stubext.h>

#include <gtest/gtest.h>

TEST(TextView, ut_initUI)
{
    TextView tv;
    EXPECT_EQ(tv.layout(), nullptr);
    EXPECT_EQ(tv.m_errLabel, nullptr);
    EXPECT_EQ(tv.m_stackedWidget, nullptr);
    EXPECT_EQ(tv.m_browser, nullptr);

    tv.initUI();
    ASSERT_NE(tv.layout(), nullptr);
    ASSERT_NE(tv.m_errLabel, nullptr);
    ASSERT_NE(tv.m_stackedWidget, nullptr);
    ASSERT_NE(tv.m_browser, nullptr);
}
