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

#include "maincontroller/maincontroller.h"
#include "maincontroller/maincontrollerprivate.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

TEST(MainControllerTest,ut_init_0)
{
    MainController mc;
    stub_ext::StubExt st;
    bool searcherGroup = false;
    st.set_lamda(&SearcherGroup::init, [&searcherGroup]() {
        searcherGroup = true;
        return false;
    });
    EXPECT_FALSE(mc.init());
    EXPECT_TRUE(searcherGroup);
    EXPECT_TRUE(mc.d->m_searchers != nullptr);
}

TEST(MainControllerTest,ut_init_1)
{
    MainController mc;
    stub_ext::StubExt st;
    bool searcherGroup = false;
    st.set_lamda(&SearcherGroup::init, [&searcherGroup]() {
        searcherGroup = true;
        return true;
    });

    EXPECT_TRUE(mc.init());
    EXPECT_TRUE(searcherGroup);
    EXPECT_TRUE(mc.d->m_searchers != nullptr);
}
