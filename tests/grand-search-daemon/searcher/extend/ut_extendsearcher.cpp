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

#include "searcher/extend/extendsearcher.h"
#include "searcher/extend/extendsearcher_p.h"
#include "searcher/extend/extendworker.h"
#include "searchplugin/pluginliaison.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>
#include <QDBusConnectionInterface>

TEST(ExtendSearcherTest, ut_setService)
{
    ExtendSearcher searcher("test");
    EXPECT_NO_FATAL_FAILURE(searcher.setService("", "", "", ""));
    EXPECT_NO_FATAL_FAILURE(searcher.setService("service", "address", "interface", "ver"));
}

TEST(ExtendSearcherTest, ut_setActivatable)
{
    ExtendSearcher searcher("test");
    searcher.setActivatable(ExtendSearcher::Inactivatable);
    EXPECT_EQ(searcher.d->m_activatable, ExtendSearcher::Inactivatable);
}

TEST(ExtendSearcherTest, ut_name)
{
    ExtendSearcher searcher("test");
    EXPECT_TRUE(searcher.name() == "test");
}

TEST(ExtendSearcherTest, ut_isActive)
{
    ExtendSearcher searcher("test");
    EXPECT_FALSE(searcher.isActive());

    stub_ext::StubExt st;
    st.set_lamda(&QDBusConnectionInterface::isServiceRegistered, [](){
        return QDBusReply<bool>();
    });
    searcher.d->m_service = "test_searvice";
    EXPECT_FALSE(searcher.isActive());
}

TEST(ExtendSearcherTest, ut_activate)
{
    stub_ext::StubExt st;
    st.set_lamda(QDBusMessage::createMethodCall, [](){
        return QDBusMessage::createError("test", "test");
    });

    ExtendSearcher searcher("test");
    searcher.d->m_activatable = ExtendSearcher::Inactivatable;
    EXPECT_FALSE(searcher.activate());

    searcher.d->m_activatable = ExtendSearcher::InnerActivation;
    EXPECT_FALSE(searcher.activate());

    searcher.d->m_activatable = ExtendSearcher::Trigger;
    EXPECT_TRUE(searcher.activate());
}

TEST(ExtendSearcherTest, ut_createWorker_0)
{
    stub_ext::StubExt st;
    st.set_lamda(&ExtendWorker::setService, [](){ return true; });

    ExtendSearcher searcher("test");
    auto worker = searcher.createWorker();
    EXPECT_TRUE(worker != nullptr);
    EXPECT_NO_FATAL_FAILURE(delete worker);
}

TEST(ExtendSearcherTest, ut_createWorker_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&ExtendWorker::setService, [](){ return false; });

    ExtendSearcher searcher("test");
    auto worker = searcher.createWorker();
    EXPECT_TRUE(worker == nullptr);
}

TEST(ExtendSearcherTest, ut_action_0)
{
    stub_ext::StubExt st;
    st.set_lamda(&PluginLiaison::init, [](){ return true; });

    ExtendSearcher searcher("test");
    EXPECT_FALSE(searcher.action("", ""));

    st.set_lamda(&PluginLiaison::action, [](){ return true; });
    EXPECT_TRUE(searcher.action("", ""));
}

TEST(ExtendSearcherTest, ut_action_1)
{
    stub_ext::StubExt st;
    st.set_lamda(&PluginLiaison::init, [](){ return false; });

    ExtendSearcher searcher("test");
    EXPECT_FALSE(searcher.action("", ""));
}
