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

#include "configuration/configer.h"
#include "configuration/configer_p.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

TEST(UserPreference, ut_initDefault)
{
    Configer conf;
    ASSERT_EQ(conf.d->m_root.get(), nullptr);
    conf.initDefault();

    ASSERT_NE(conf.d->m_root.get(), nullptr);
    EXPECT_NE(conf.d->m_root->group(GRANDSEARCH_PREF_SEARCHERENABLED).get(), nullptr);
    EXPECT_NE(conf.d->m_root->group(GRANDSEARCH_CLASS_FILE_DEEPIN).get(), nullptr);
    EXPECT_NE(conf.d->m_root->group(GRANDSEARCH_CLASS_FILE_FSEARCH).get(), nullptr);
}

TEST(UserPreference, ut_init)
{
    Configer conf;
    stub_ext::StubExt st;
    bool init = false;
    st.set_lamda(&Configer::initDefault, [&init]() {
        init = true;
    });

    bool load = false;
    st.set_lamda(&Configer::onLoadConfig, [&load]() {
        load = true;
    });

    auto orgFunc = (bool(QFileInfo::*)()const)&QFileInfo::exists;
    st.set_lamda(orgFunc, []() {
        return true;
    });

    ASSERT_EQ(conf.d->m_watcher, nullptr);
    ASSERT_TRUE(conf.d->m_configPath.isEmpty());

    //测试更换&释放 watcher
    auto watcher = new QFileSystemWatcher;
    conf.d->m_watcher = watcher;

    EXPECT_TRUE(conf.init());
    EXPECT_NE(conf.d->m_watcher, nullptr);
    EXPECT_NE(conf.d->m_watcher, watcher);
    EXPECT_FALSE(conf.d->m_configPath.isEmpty());
    EXPECT_TRUE(init);
    EXPECT_TRUE(load);
}

TEST(UserPreference, ut_group)
{
    Configer conf;
    ASSERT_EQ(conf.d->m_root.get(), nullptr);
    EXPECT_EQ(conf.group("test"), nullptr);

    conf.d->m_root.reset(new UserPreference({}));
    UserPreferencePointer va(new UserPreference({}));
    conf.d->m_root->setValue("test", QVariant::fromValue(va));

    EXPECT_EQ(conf.group("test"), va.get());
}

TEST(ConfigerPrivate, ut_defaultSearcher)
{
    auto up = ConfigerPrivate::defaultSearcher();
    EXPECT_TRUE(up->value(GRANDSEARCH_CLASS_FILE_DEEPIN, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_CLASS_FILE_FSEARCH, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_CLASS_APP_DESKTOP, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_CLASS_SETTING_CONTROLCENTER, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_CLASS_WEB_STATICTEXT, false));
}

TEST(ConfigerPrivate, ut_fileSearcher)
{
    auto up = ConfigerPrivate::fileSearcher();
    EXPECT_TRUE(up->value(GRANDSEARCH_GROUP_FOLDER, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_GROUP_FILE, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_GROUP_FILE_VIDEO, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_GROUP_FILE_AUDIO, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_GROUP_FILE_PICTURE, false));
    EXPECT_TRUE(up->value(GRANDSEARCH_GROUP_FILE_DOCUMNET, false));
}

TEST(ConfigerPrivate, ut_updateConfig1)
{
    Configer conf;
    EXPECT_FALSE(conf.d->updateConfig1(nullptr));
    QSettings set;
    EXPECT_FALSE(conf.d->updateConfig1(&set));

    conf.initDefault();
    EXPECT_TRUE(conf.d->updateConfig1(&set));
}
