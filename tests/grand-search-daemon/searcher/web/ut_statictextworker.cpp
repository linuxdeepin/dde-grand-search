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

#include "searcher/web/statictextworker.h"
#include "global/builtinsearch.h"
#include "configuration/configer.h"
#include "global/searchconfigdefine.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

TEST(StaticTextWorker, ut_setContext)
{
    StaticTextWorker worker(GRANDSEARCH_CLASS_WEB_STATICTEXT);
    EXPECT_NO_FATAL_FAILURE(worker.setContext(""));
}

TEST(StaticTextWorker, ut_isAsync)
{
    StaticTextWorker worker(GRANDSEARCH_CLASS_WEB_STATICTEXT);
    EXPECT_FALSE(worker.isAsync());
}

TEST(StaticTextWorker, ut_working_0)
{
    StaticTextWorker worker(GRANDSEARCH_CLASS_WEB_STATICTEXT);
    EXPECT_TRUE(worker.working(nullptr));

    worker.m_status.storeRelease(ProxyWorker::Runing);
    EXPECT_FALSE(worker.working(nullptr));
}

TEST(StaticTextWorker, ut_working_1)
{
    StaticTextWorker worker(GRANDSEARCH_CLASS_WEB_STATICTEXT);
    worker.setContext("test");
    stub_ext::StubExt stu;
    stu.set_lamda(&Configer::group, [](){
        QVariantHash datas;
        datas.insert(GRANDSEARCH_WEB_SEARCHENGINE, GRANDSEARCH_WEB_SEARCHENGINE_GOOGLE);
        UserPreferencePointer u(new UserPreference(datas));
        return u;
    });

    EXPECT_TRUE(worker.working(nullptr));
}

TEST(StaticTextWorker, ut_terminate)
{
    StaticTextWorker worker(GRANDSEARCH_CLASS_WEB_STATICTEXT);
    worker.terminate();
    EXPECT_EQ(worker.status(), ProxyWorker::Terminated);
}

TEST(StaticTextWorker, ut_hasItem)
{
    StaticTextWorker worker(GRANDSEARCH_CLASS_WEB_STATICTEXT);
    EXPECT_FALSE(worker.hasItem());
}

TEST(StaticTextWorker, ut_takeAll)
{
    StaticTextWorker worker(GRANDSEARCH_CLASS_WEB_STATICTEXT);
    GrandSearch::MatchedItem item;
    item.item = "test";
    item.name = "Search for test";
    item.type = "application/x-dde-grand-search-web-statictext";
    item.icon = item.type;
    item.searcher = worker.name();

    worker.m_items << item;
    EXPECT_FALSE(worker.takeAll().isEmpty());
}
