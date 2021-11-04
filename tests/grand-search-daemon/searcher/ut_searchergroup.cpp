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

#include "searcher/searchergroup.h"
#include "searcher/searchergroup_p.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

TEST(SearcherGroupTest, ut_init)
{
    stub_ext::StubExt st;
    st.set_lamda(&SearcherGroupPrivate::initBuiltin, [](){ return; });
    st.set_lamda(&SearcherGroupPrivate::initPluinManager, [](){ return false; });
    st.set_lamda(&SearcherGroupPrivate::initExtendSearcher, [](){ return; });
    st.set_lamda(&PluginManager::autoActivate, [](){ return; });

    SearcherGroup group;
    EXPECT_FALSE(group.init());

    st.reset(&SearcherGroupPrivate::initPluinManager);
    st.set_lamda(&SearcherGroupPrivate::initPluinManager, [](){ return true; });
    group.d->m_pluginManager = new PluginManager(&group);
    EXPECT_TRUE(group.init());
}

TEST(SearcherGroupTest, ut_searchers)
{
    SearcherGroup group;
    EXPECT_TRUE(group.searchers().isEmpty());
}

TEST(SearcherGroupTest, ut_searcher)
{
    SearcherGroup group;
    DesktopAppSearcher *searcher = new DesktopAppSearcher(&group);
    group.d->m_builtin << searcher;
    EXPECT_TRUE(group.searcher(searcher->name()) != nullptr);
    EXPECT_TRUE(group.searcher("test") == nullptr);
}

TEST(SearcherGroupTest, ut_dormancy)
{
    stub_ext::StubExt st;
    st.set_lamda(&PluginManager::plugins, []()
    {
        GrandSearch::SearchPluginInfo info;
        info.mode = GrandSearch::SearchPluginInfo::Auto;
        info.priority = GrandSearch::SearchPluginInfo::Low;
        return QList<GrandSearch::SearchPluginInfo>({info});
    });
    st.set_lamda(&PluginManager::inactivate, [](){ return; });

    SearcherGroup group;
    group.d->m_pluginManager = new PluginManager(&group);
    EXPECT_NO_FATAL_FAILURE(group.dormancy());
}

// SearcherGroupPrivate
TEST(SearcherGroupTest, ut_initBuiltin)
{
    stub_ext::StubExt st;
    st.set_lamda(&DesktopAppSearcher::asyncInit, [](){ return; });
#ifdef ENABLE_FSEARCH
    st.set_lamda(&FsSearcher::asyncInitDataBase, [](){ return; });
#endif
    SearcherGroup group;
    EXPECT_NO_FATAL_FAILURE(group.d->initBuiltin());
}

TEST(SearcherGroupTest, ut_addExtendSearcher)
{
    GrandSearch::SearchPluginInfo info;
    SearcherGroup group;
    EXPECT_FALSE(group.d->addExtendSearcher(info));

    info.name = "test";
    info.address = "test";
    info.address = "test";
    info.service = "test";
    info.interface = "test";
    info.ifsVersion = "test";
    info.mode = GrandSearch::SearchPluginInfo::Auto;
    EXPECT_TRUE(group.d->addExtendSearcher(info));

    group.d->m_extend.clear();
    info.mode = GrandSearch::SearchPluginInfo::Trigger;
    EXPECT_TRUE(group.d->addExtendSearcher(info));
    EXPECT_FALSE(group.d->addExtendSearcher(info));
}

TEST(SearcherGroupTest, ut_onActivatePlugin)
{
    stub_ext::StubExt st;
    st.set_lamda(&PluginManager::activatePlugin, [](){ return true; });

    SearcherGroup group;
    group.d->m_pluginManager = new PluginManager(&group);
    bool ret = false;
    group.d->onActivatePlugin("test", ret);
    EXPECT_TRUE(ret);
}

TEST(SearcherGroupTest, ut_initPluinManager)
{
    stub_ext::StubExt st;
    st.set_lamda(&PluginManager::loadPlugin, [](){ return true; });

    SearcherGroup group;
    EXPECT_TRUE(group.d->initPluinManager());
}

TEST(SearcherGroupTest, ut_initExtendSearcher)
{
    stub_ext::StubExt st;
    st.set_lamda(&PluginManager::plugins, [](){
        GrandSearch::SearchPluginInfo info;
        return QList<GrandSearch::SearchPluginInfo>({info});
    });

    SearcherGroup group;
    group.d->m_pluginManager = new PluginManager(&group);
    EXPECT_NO_FATAL_FAILURE(group.d->initExtendSearcher());
}
