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

#include "searcher/extend/extendworker.h"
#include "searchplugin/pluginliaison.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

TEST(ExtendWorkerTest, ut_setService_0)
{
    stub_ext::StubExt st;
    st.set_lamda(&PluginLiaison::init, [](){ return false; });

    ExtendWorker worker("test");
    EXPECT_FALSE(worker.setService("", "", "", ""));

    worker.m_liaison = new PluginLiaison(&worker);
    EXPECT_FALSE(worker.setService("test", "test", "test", "test"));

    delete worker.m_liaison;
    worker.m_liaison = nullptr;
    EXPECT_FALSE(worker.setService("test", "test", "test", "test"));
}

TEST(ExtendWorkerTest, ut_setService_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&PluginLiaison::init, [](){ return true; });

    ExtendWorker worker("test");
    EXPECT_TRUE(worker.setService("test", "test", "test", "test"));
}

TEST(ExtendWorkerTest, ut_setContext)
{
    ExtendWorker worker("test");
    worker.setContext("test");
    EXPECT_TRUE(worker.m_context == "test");
}

TEST(ExtendWorkerTest, ut_isAsync)
{
    ExtendWorker worker("test");
    EXPECT_TRUE(worker.isAsync());
}

TEST(ExtendWorkerTest, ut_working_0)
{
    ExtendWorker worker("test");
    QString context("test");
    EXPECT_FALSE(worker.working(&context));

    worker.m_status.storeRelease(ProxyWorker::Runing);
    EXPECT_FALSE(worker.working(&context));
}

TEST(ExtendWorkerTest, ut_working_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&PluginLiaison::isVaild, [](){ return true; });
    st.set_lamda(&PluginLiaison::search, [](){ return true; });

    ExtendWorker worker("test");
    worker.m_liaison = new PluginLiaison(&worker);
    worker.setContext("test");
    QString context("test");
    EXPECT_TRUE(worker.working(&context));

    st.reset(&PluginLiaison::search);
    st.set_lamda(&PluginLiaison::search, [](){ return false; });
    worker.m_status.storeRelease(ProxyWorker::Ready);
    EXPECT_FALSE(worker.working(&context));

    st.reset(&PluginLiaison::isVaild);
    st.set_lamda(&PluginLiaison::isVaild, [](){ return false; });
    worker.m_status.storeRelease(ProxyWorker::Ready);
    EXPECT_TRUE(worker.working(&context));
}

TEST(ExtendWorkerTest, ut_terminate)
{
    stub_ext::StubExt st;
    st.set_lamda(&PluginLiaison::stop, [](){ return true; });

    ExtendWorker worker("test");
    worker.m_liaison = new PluginLiaison(&worker);
    worker.terminate();
    EXPECT_TRUE(worker.status() == ProxyWorker::Terminated);
}

TEST(ExtendWorkerTest, ut_hasItem)
{
    ExtendWorker worker("test");
    EXPECT_FALSE(worker.hasItem());
}

TEST(ExtendWorkerTest, ut_takeAll)
{
    GrandSearch::MatchedItem item;
    item.name = "test";
    item.item = "test";
    item.icon = "test";
    item.type = "test";
    item.searcher = "test";

    ExtendWorker worker("test");
    worker.m_items.insert("test", {item});
    EXPECT_TRUE(!worker.takeAll().isEmpty());
}

TEST(ExtendWorkerTest, ut_tryWorking)
{
    stub_ext::StubExt st;
    st.set_lamda(&PluginLiaison::search, [](){ return false; });

    ExtendWorker worker("test");
    worker.m_liaison = new PluginLiaison(&worker);
    EXPECT_NO_FATAL_FAILURE(worker.tryWorking());

    worker.m_callSerach = ExtendWorker::WaitCall;
    EXPECT_NO_FATAL_FAILURE(worker.tryWorking());
}

TEST(ExtendWorkerTest, ut_onWorkFinished)
{
    GrandSearch::MatchedItem item;
    GrandSearch::MatchedItemMap itemMap;
    itemMap.insert("test", {item});

    ExtendWorker worker("test");
    EXPECT_NO_FATAL_FAILURE(worker.onWorkFinished(itemMap));
}
