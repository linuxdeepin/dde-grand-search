/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#include "global/searchconfigdefine.h"
#include "global/builtinsearch.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QStandardPaths>
#include <QFile>
#include <QDir>
#include <QApplication>

#define private public
#define protected public
#include "business/config/searchconfig.h"

GRANDSEARCH_USE_NAMESPACE
using namespace testing;

TEST(SearchConfigTest, init)
{
    if (!QStandardPaths::isTestModeEnabled())
        QStandardPaths::setTestModeEnabled(true);

    auto configPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
    configPath = configPath
                 + "/" + QApplication::organizationName()
                 + "/" + "dde-grand-search-daemon"
                 + "/" + "dde-grand-search-daemon" + ".conf";

    QFile configFile(configPath);
    if (configFile.exists())
        configFile.remove();

    // 测试新建配置文件时无版本信息
    SearchConfig config;
    config.m_settings->beginGroup(GRANDSEARCH_VERSION_GROUP);
    QString curVersion = config.m_settings->value(GRANDSEARCH_VERSION_CONFIG).toString();
    config.m_settings->endGroup();
    EXPECT_EQ(curVersion, GRANDSEARCH_VERSION_CONFIG_CURRENT);

    // 测试版本信息存在但是不匹配自动升级
    config.m_settings->beginGroup(GRANDSEARCH_VERSION_GROUP);
    config.m_settings->setValue(GRANDSEARCH_VERSION_CONFIG, "aaaaaaa");
    config.m_settings->endGroup();

    // 构造单例对象供后续测试用例复用
    SearchConfig *otherConfig = SearchConfig::instance();
    ASSERT_TRUE(otherConfig);

    otherConfig->m_settings->beginGroup(GRANDSEARCH_VERSION_GROUP);
    curVersion = otherConfig->m_settings->value(GRANDSEARCH_VERSION_CONFIG).toString();
    otherConfig->m_settings->endGroup();
    EXPECT_EQ(curVersion, GRANDSEARCH_VERSION_CONFIG_CURRENT);
}

TEST(SearchConfigTest, getConfig)
{
    QString value = SearchConfig::instance()->getConfig("aaaaaaaaa", "bbbbbbbbb").toString();
    EXPECT_TRUE(value.isEmpty());

    value = SearchConfig::instance()->getConfig("aaaaaaaaa", GRANDSEARCH_VERSION_CONFIG).toString();
    EXPECT_TRUE(value.isEmpty());

    value = SearchConfig::instance()->getConfig(GRANDSEARCH_VERSION_GROUP, "bbbbbbbbb").toString();
    EXPECT_TRUE(value.isEmpty());

    value = SearchConfig::instance()->getConfig(GRANDSEARCH_VERSION_GROUP, GRANDSEARCH_VERSION_CONFIG).toString();
    EXPECT_EQ(value, GRANDSEARCH_VERSION_CONFIG_CURRENT);
}

TEST(SearchConfigTest, setConfig)
{
    SearchConfig::instance()->setConfig(GRANDSEARCH_SEARCH_GROUP, GRANDSEARCH_GROUP_FILE, true);
    SearchConfig::instance()->setConfig(GRANDSEARCH_SEARCH_GROUP, GRANDSEARCH_GROUP_FOLDER, true);
    SearchConfig::instance()->setConfig(GRANDSEARCH_PLAN_GROUP, GRANDSEARCH_PLAN_EXPERIENCE, false);

    SearchConfig::instance()->m_settings->beginGroup(GRANDSEARCH_SEARCH_GROUP);

    bool value = SearchConfig::instance()->m_settings->value(GRANDSEARCH_GROUP_FILE, false).toBool();
    EXPECT_TRUE(value);

    value = SearchConfig::instance()->m_settings->value(GRANDSEARCH_GROUP_FOLDER, false).toBool();
    EXPECT_TRUE(value);

    SearchConfig::instance()->m_settings->endGroup();

    SearchConfig::instance()->m_settings->beginGroup(GRANDSEARCH_PLAN_GROUP);
    value = SearchConfig::instance()->m_settings->value(GRANDSEARCH_PLAN_EXPERIENCE, true).toBool();
    EXPECT_FALSE(value);

    SearchConfig::instance()->m_settings->endGroup();
}

TEST(SearchConfigTest, removeConfig)
{
    SearchConfig::instance()->m_settings->beginGroup(GRANDSEARCH_SEARCH_GROUP);
    SearchConfig::instance()->m_settings->setValue(GRANDSEARCH_GROUP_FILE_DOCUMNET, true);
    SearchConfig::instance()->m_settings->endGroup();

    SearchConfig::instance()->removeConfig(GRANDSEARCH_SEARCH_GROUP, GRANDSEARCH_GROUP_FILE_DOCUMNET);

    SearchConfig::instance()->m_settings->beginGroup(GRANDSEARCH_SEARCH_GROUP);
    bool value = SearchConfig::instance()->m_settings->value(GRANDSEARCH_GROUP_FILE_DOCUMNET, false).toBool();
    SearchConfig::instance()->m_settings->endGroup();
    EXPECT_FALSE(value);
}

TEST(SearchConfigTest, setConfigList)
{
    QStringList keys{GRANDSEARCH_GROUP_APP, GRANDSEARCH_GROUP_SETTING, GRANDSEARCH_GROUP_FILE_VIDEO};
    QVariantList values{true, true, true};
    SearchConfig::instance()->setConfigList(GRANDSEARCH_SEARCH_GROUP, keys, values);

    SearchConfig::instance()->m_settings->beginGroup(GRANDSEARCH_SEARCH_GROUP);

    bool value = SearchConfig::instance()->m_settings->value(GRANDSEARCH_GROUP_APP, false).toBool();
    EXPECT_TRUE(value);

    value = SearchConfig::instance()->m_settings->value(GRANDSEARCH_GROUP_SETTING, false).toBool();
    EXPECT_TRUE(value);

    value = SearchConfig::instance()->m_settings->value(GRANDSEARCH_GROUP_FILE_VIDEO, false).toBool();
    EXPECT_TRUE(value);

    SearchConfig::instance()->m_settings->endGroup();
}

TEST(SearchConfigTest, removeConfigList)
{
    QStringList keys{GRANDSEARCH_GROUP_APP, GRANDSEARCH_GROUP_SETTING, GRANDSEARCH_GROUP_FILE_VIDEO};
    SearchConfig::instance()->removeConfigList(GRANDSEARCH_SEARCH_GROUP, keys);

    SearchConfig::instance()->m_settings->beginGroup(GRANDSEARCH_SEARCH_GROUP);

    bool value = SearchConfig::instance()->m_settings->value(GRANDSEARCH_GROUP_APP, false).toBool();
    EXPECT_FALSE(value);

    value = SearchConfig::instance()->m_settings->value(GRANDSEARCH_GROUP_SETTING, false).toBool();
    EXPECT_FALSE(value);

    value = SearchConfig::instance()->m_settings->value(GRANDSEARCH_GROUP_FILE_VIDEO, false).toBool();
    EXPECT_FALSE(value);

    SearchConfig::instance()->m_settings->endGroup();
}
