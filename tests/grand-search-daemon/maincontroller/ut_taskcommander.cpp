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

#include "maincontroller/task/taskcommander.h"
#include "maincontroller/task/taskcommander_p.h"

//#include "maincontroller/maincontroller.h"
//#include "maincontroller/maincontroller_p.h"
//#include "configuration/configer.h"
//#include "searcher/searchergroup_p.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

TEST(TaskCommanderTest, ut_taskID)
{
    TaskCommander tc("");
    EXPECT_FALSE(tc.taskID().isEmpty());
}

TEST(TaskCommanderTest, ut_content)
{
    TaskCommander tc("");
    EXPECT_TRUE(tc.content().isEmpty());

    const QString content = "test";
    TaskCommander tc2(content);
    EXPECT_EQ(tc2.content(), content);
}

TEST(TaskCommanderTest, ut_getResults)
{
    TaskCommander tc("");
    EXPECT_TRUE(tc.getResults().isEmpty());

    GrandSearch::MatchedItemMap rets;
    rets.insert("test", {GrandSearch::MatchedItem()});

    tc.d->m_results = rets;

    EXPECT_EQ(tc.getResults(), rets);
}

TEST(TaskCommanderTest, ut_readBuffer)
{
    TaskCommander tc("");
    EXPECT_TRUE(tc.readBuffer().isEmpty());

    GrandSearch::MatchedItemMap rets;
    rets.insert("test", {GrandSearch::MatchedItem()});

    tc.d->m_buffer = rets;

    EXPECT_EQ(tc.readBuffer(), rets);
    EXPECT_TRUE(tc.d->m_buffer.isEmpty());
}

TEST(TaskCommanderTest, ut_isEmptyBuffer)
{
    TaskCommander tc("");
    EXPECT_TRUE(tc.isEmptyBuffer());

    GrandSearch::MatchedItemMap rets;
    rets.insert("test", {GrandSearch::MatchedItem()});

    tc.d->m_buffer = rets;

    EXPECT_FALSE(tc.isEmptyBuffer());
}
