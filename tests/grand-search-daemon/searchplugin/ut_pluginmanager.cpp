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

#include "searchplugin/pluginmanager_p.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

TEST(PluginManager, ut_loadPlugin)
{
    stub_ext::StubExt st;
    bool load = false;
    st.set_lamda(&PluginLoader::load, [&load]() {
        load = true;
        return true;
    });
    PluginManager pm;
    ASSERT_EQ(pm.d->m_loader, nullptr);
    ASSERT_EQ(pm.d->m_process, nullptr);

    EXPECT_TRUE(pm.loadPlugin());
    EXPECT_TRUE(load);
    EXPECT_NE(pm.d->m_loader, nullptr);
    EXPECT_NE(pm.d->m_process, nullptr);
}
