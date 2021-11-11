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

#include "gui/exhibition/preview/generalpreviewplugin.h"
#include "gui/exhibition/preview/generalpreviewplugin_p.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <DGuiApplicationHelper>

#include <QFileInfo>
#include <QIcon>
#include <QDir>
#include <QThread>

using namespace testing;
using namespace GrandSearch;
DGUI_USE_NAMESPACE;

TEST(GeneralPreviewPluginTest, constructor)
{
    stub_ext::StubExt stu;

    DGuiApplicationHelper::ColorType ut_themeType = DGuiApplicationHelper::DarkType;
    stu.set_lamda(ADDR(DGuiApplicationHelper, themeType), [&](){
        return ut_themeType;
    });

    GeneralPreviewPlugin *plugin = new GeneralPreviewPlugin;

    ASSERT_TRUE(plugin);
    ASSERT_TRUE(plugin->d_p);

    ASSERT_TRUE(plugin->d_p->m_iconLabel);
    ASSERT_TRUE(plugin->d_p->m_nameLabel);
    ASSERT_TRUE(plugin->d_p->m_sizeLabel);

    plugin->d_p->m_sizeWorker = new FileStatisticsThread;
    ASSERT_TRUE(plugin->d_p->m_sizeWorker);

    delete plugin;
}

TEST(GeneralPreviewPluginTest, init)
{
    GeneralPreviewPlugin plugin;

    plugin.init(nullptr);
}

TEST(GeneralPreviewPluginTest, previewItem)
{
    GeneralPreviewPlugin plugin;

    // 测试直接返回
    ItemInfo itemInfo;
    itemInfo[PREVIEW_ITEMINFO_ITEM] = "item";
    itemInfo[PREVIEW_ITEMINFO_NAME] = "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ"
                                      "QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ";
    itemInfo[PREVIEW_ITEMINFO_ICON] = "icon.png";
    itemInfo[PREVIEW_ITEMINFO_TYPE] = "type";

    MatchedItem item;
    item.item = itemInfo[PREVIEW_ITEMINFO_ITEM];
    item.name = itemInfo[PREVIEW_ITEMINFO_NAME];
    item.icon = itemInfo[PREVIEW_ITEMINFO_ICON];
    item.type = itemInfo[PREVIEW_ITEMINFO_TYPE];
    item.searcher = itemInfo[PREVIEW_ITEMINFO_SEARCHER];
    plugin.d_p->m_item = item;

    plugin.previewItem(itemInfo);

    EXPECT_TRUE(plugin.d_p->m_detailInfos.isEmpty());


    itemInfo[PREVIEW_ITEMINFO_ITEM] = "otherItem";

    plugin.previewItem(itemInfo);

    EXPECT_FALSE(plugin.d_p->m_detailInfos.isEmpty());

    itemInfo[PREVIEW_ITEMINFO_ITEM] = "/usr/bin/";
    itemInfo[PREVIEW_ITEMINFO_ICON] = "/test/icon.png";

    plugin.previewItem(itemInfo);

    EXPECT_FALSE(plugin.d_p->m_detailInfos.isEmpty());

    itemInfo[PREVIEW_ITEMINFO_NAME] = "WWWW";

    plugin.previewItem(itemInfo);

    EXPECT_FALSE(plugin.d_p->m_detailInfos.isEmpty());
}

TEST(GeneralPreviewPluginTest, item)
{
    GeneralPreviewPlugin plugin;

    MatchedItem item;
    item.item = PREVIEW_ITEMINFO_ITEM;
    item.name = PREVIEW_ITEMINFO_NAME;
    item.icon = PREVIEW_ITEMINFO_ICON;
    item.type = PREVIEW_ITEMINFO_TYPE;
    item.searcher = PREVIEW_ITEMINFO_SEARCHER;

    plugin.d_p->m_item = item;

    ItemInfo getVal = plugin.item();

    EXPECT_EQ(getVal[PREVIEW_ITEMINFO_ITEM], item.item);
    EXPECT_EQ(getVal[PREVIEW_ITEMINFO_NAME], item.name);
}

TEST(GeneralPreviewPluginTest, stopPreview)
{
    GeneralPreviewPlugin plugin;

    bool result = plugin.stopPreview();
    EXPECT_TRUE(result);
}

TEST(GeneralPreviewPluginTest, contentWidget)
{
    GeneralPreviewPlugin plugin;

    QWidget *getVal = plugin.contentWidget();

    EXPECT_EQ(getVal, plugin.d_p->m_contentWidget);
}

TEST(GeneralPreviewPluginTest, getAttributeDetailInfo)
{
    GeneralPreviewPlugin plugin;

    DetailInfoList getVal = plugin.getAttributeDetailInfo();

    EXPECT_EQ(getVal, plugin.d_p->m_detailInfos);
}

TEST(GeneralPreviewPluginTest, toolBarWidget)
{
    GeneralPreviewPlugin plugin;

    QWidget *getVal = plugin.toolBarWidget();

    EXPECT_FALSE(getVal);
}

TEST(GeneralPreviewPluginTest, showToolBar)
{
    GeneralPreviewPlugin plugin;

    bool getVal = plugin.showToolBar();

    EXPECT_TRUE(getVal);
}

TEST(GeneralPreviewPluginTest, updateFolderSize)
{
    GeneralPreviewPlugin plugin;

    plugin.updateFolderSize(1000);

    EXPECT_FALSE(plugin.d_p->m_sizeLabel->text().isEmpty());
}
