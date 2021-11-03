/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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

#include "searcher/web/statictextechoer.h"
#include "global/builtinsearch.h"
#include "searcher/web/statictextworker.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

TEST(StaticTextEchoerTest, ut_name)
{
    StaticTextEchoer ste;
    EXPECT_TRUE(ste.name() == GRANDSEARCH_CLASS_WEB_STATICTEXT);
}

TEST(StaticTextEchoerTest, ut_isActive)
{
    StaticTextEchoer ste;
    EXPECT_TRUE(ste.isActive());
}

TEST(StaticTextEchoerTest, ut_activate)
{
    StaticTextEchoer ste;
    EXPECT_FALSE(ste.activate());
}

TEST(StaticTextEchoerTest, ut_createWorker)
{
    StaticTextEchoer ste;
    auto worker = ste.createWorker();
    EXPECT_TRUE(worker != nullptr);
    EXPECT_NO_FATAL_FAILURE(delete worker);
}

TEST(StaticTextEchoerTest, ut_action)
{
    StaticTextEchoer ste;
    EXPECT_FALSE(ste.action("", ""));
}
