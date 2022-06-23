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
#include "global/grandsearch_global.h"
#include "searcher/app/desktopappworker.h"
#include "global/builtinsearch.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>
GRANDSEARCH_USE_NAMESPACE

TEST(DesktopAppWorkerTest, ut_setContext)
{
    DesktopAppWorker worker(GRANDSEARCH_CLASS_APP_DESKTOP);
    EXPECT_NO_FATAL_FAILURE(worker.setContext(""));
}

TEST(DesktopAppWorkerTest, ut_isAsync)
{
    DesktopAppWorker worker(GRANDSEARCH_CLASS_APP_DESKTOP);
    EXPECT_FALSE(worker.isAsync());
}

TEST(DesktopAppWorkerTest, ut_working_0)
{
    DesktopAppWorker worker(GRANDSEARCH_CLASS_APP_DESKTOP);
    EXPECT_TRUE(worker.working(nullptr));

    worker.m_status.storeRelease(DesktopAppWorker::Runing);
    EXPECT_FALSE(worker.working(nullptr));
}

TEST(DesktopAppWorkerTest, ut_working_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&QAtomicInt::loadAcquire, [](){ return ProxyWorker::Terminated; });

    QHash<QString, QList<QSharedPointer<GrandSearch::MatchedItem>>> indexTable;
    QSharedPointer<GrandSearch::MatchedItem> info(new GrandSearch::MatchedItem());
    info->name = "test app";
    info->item = "/usr/share/applications/test.desktop";
    info->icon = "application-x-desktop";
    info->type = "application/x-desktop";
    info->searcher = GRANDSEARCH_CLASS_APP_DESKTOP;
    indexTable.insert("test app", {info, info});

    DesktopAppWorker worker(GRANDSEARCH_CLASS_APP_DESKTOP);
    worker.setIndexTable(indexTable);
    worker.m_context = "test";
    EXPECT_FALSE(worker.working(nullptr));
}

TEST(DesktopAppWorkerTest, ut_working_2)
{
    stub_ext::StubExt st;
    st.set_lamda(&QTime::elapsed, [](){ return 100; });

    QHash<QString, QList<QSharedPointer<GrandSearch::MatchedItem>>> indexTable;
    QSharedPointer<GrandSearch::MatchedItem> info(new GrandSearch::MatchedItem());
    info->name = "test app";
    info->item = "/usr/share/applications/test.desktop";
    info->icon = "application-x-desktop";
    info->type = "application/x-desktop";
    info->searcher = GRANDSEARCH_CLASS_APP_DESKTOP;
    indexTable.insert("test app", {info, info});

    DesktopAppWorker worker(GRANDSEARCH_CLASS_APP_DESKTOP);
    worker.setIndexTable(indexTable);
    worker.m_context = "test";
    EXPECT_TRUE(worker.working(nullptr));
}

TEST(DesktopAppWorkerTest, ut_terminate)
{
    DesktopAppWorker worker(GRANDSEARCH_CLASS_APP_DESKTOP);
    worker.terminate();
    EXPECT_EQ(worker.status(), ProxyWorker::Terminated);
}

TEST(DesktopAppWorkerTest, ut_hasItem)
{
    DesktopAppWorker worker(GRANDSEARCH_CLASS_APP_DESKTOP);
    EXPECT_FALSE(worker.hasItem());
}

TEST(DesktopAppWorkerTest, ut_takeAll)
{
    GrandSearch::MatchedItem item;
    item.name = "test app";
    item.item = "/usr/share/applications/test.desktop";
    item.icon = "application-x-desktop";
    item.type = "application/x-desktop";
    item.searcher = GRANDSEARCH_CLASS_APP_DESKTOP;

    DesktopAppWorker worker(GRANDSEARCH_CLASS_APP_DESKTOP);
    worker.m_items << item;
    EXPECT_FALSE(worker.takeAll().isEmpty());
}

TEST(DesktopAppWorkerTest, ut_setIndexTable)
{
    DesktopAppWorker worker(GRANDSEARCH_CLASS_APP_DESKTOP);
    worker.m_status.storeRelease(ProxyWorker::Runing);
    EXPECT_NO_FATAL_FAILURE(worker.setIndexTable({}));

}
