// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    worker.m_status.storeRelease(ProxyWorker::Terminated);
    EXPECT_FALSE(worker.working(nullptr));
}

TEST(DesktopAppWorkerTest, ut_working_2)
{
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
    EXPECT_TRUE(worker.hasItem());
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

TEST(DesktopAppWorkerTest, ut_buildKeyword_0)
{
    DesktopAppWorker worker(GRANDSEARCH_CLASS_APP_DESKTOP);
    EXPECT_EQ(worker.buildKeyword("hello"), "hello");
    EXPECT_EQ(worker.buildKeyword("he*lo"), "he\\*lo");
    EXPECT_EQ(worker.buildKeyword(""), "");
}

TEST(DesktopAppWorkerTest, ut_buildKeyword_1)
{
    DesktopAppWorker worker(GRANDSEARCH_CLASS_APP_DESKTOP);
    QString json = R"({"Keyword":["app1","app2"]})";
    EXPECT_EQ(worker.buildKeyword(json), "app1|app2");
}

TEST(DesktopAppWorkerTest, ut_buildKeyword_2)
{
    DesktopAppWorker worker(GRANDSEARCH_CLASS_APP_DESKTOP);
    QString json = R"({"Keyword":[]})";
    EXPECT_EQ(worker.buildKeyword(json), ".*");
}

TEST(DesktopAppWorkerTest, ut_buildKeyword_3)
{
    DesktopAppWorker worker(GRANDSEARCH_CLASS_APP_DESKTOP);
    QString json = R"({"Keyword":[""]})";
    EXPECT_EQ(worker.buildKeyword(json), ".*");
}

TEST(DesktopAppWorkerTest, ut_buildKeyword_4)
{
    DesktopAppWorker worker(GRANDSEARCH_CLASS_APP_DESKTOP);
    QString json = R"({"Keyword":["ap*p","te?st"]})";
    EXPECT_EQ(worker.buildKeyword(json), "ap\\*p|te\\?st");
}

TEST(DesktopAppWorkerTest, ut_group)
{
    DesktopAppWorker worker(GRANDSEARCH_CLASS_APP_DESKTOP);
    EXPECT_EQ(worker.group(), GRANDSEARCH_GROUP_APP);
}

TEST(DesktopAppWorkerTest, ut_status)
{
    DesktopAppWorker worker(GRANDSEARCH_CLASS_APP_DESKTOP);
    EXPECT_EQ(worker.status(), ProxyWorker::Ready);

    worker.m_status.storeRelease(ProxyWorker::Runing);
    EXPECT_EQ(worker.status(), ProxyWorker::Runing);

    worker.m_status.storeRelease(ProxyWorker::Terminated);
    EXPECT_EQ(worker.status(), ProxyWorker::Terminated);
}
