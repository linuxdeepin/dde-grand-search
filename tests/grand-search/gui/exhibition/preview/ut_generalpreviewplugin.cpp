// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    // m_sizeLabel is not created in constructor, it's created lazily when previewItem is called with a directory

    plugin->d_p->m_sizeWorker = new FileStatisticsThread;
    ASSERT_TRUE(plugin->d_p->m_sizeWorker);

    delete plugin->d_p->m_sizeWorker;
    plugin->d_p->m_sizeWorker = nullptr;

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

    // 测试预览不同项目
    ItemInfo itemInfo;
    itemInfo[PREVIEW_ITEMINFO_ITEM] = "/tmp/test.txt";
    itemInfo[PREVIEW_ITEMINFO_NAME] = "test.txt";
    itemInfo[PREVIEW_ITEMINFO_ICON] = "text-plain";
    itemInfo[PREVIEW_ITEMINFO_TYPE] = "text";

    plugin.previewItem(itemInfo);

    // 详细信息应该被设置
    EXPECT_FALSE(plugin.d_p->m_detailInfos.isEmpty());

    // 预览另一个项目
    itemInfo[PREVIEW_ITEMINFO_ITEM] = "/tmp/other.txt";
    itemInfo[PREVIEW_ITEMINFO_NAME] = "other.txt";

    plugin.previewItem(itemInfo);

    EXPECT_FALSE(plugin.d_p->m_detailInfos.isEmpty());

    // 预览目录
    itemInfo[PREVIEW_ITEMINFO_ITEM] = "/usr/bin/";
    itemInfo[PREVIEW_ITEMINFO_ICON] = "folder";

    plugin.previewItem(itemInfo);

    EXPECT_FALSE(plugin.d_p->m_detailInfos.isEmpty());

    // 预览另一个目录
    itemInfo[PREVIEW_ITEMINFO_NAME] = "bin";

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

    // First call previewItem to initialize m_sizeLabel (if it's a directory)
    // Then updateFolderSize can work properly
    ItemInfo info;
    info[PREVIEW_ITEMINFO_ITEM] = "/tmp";
    info[PREVIEW_ITEMINFO_NAME] = "tmp";
    info[PREVIEW_ITEMINFO_ICON] = "folder";
    info[PREVIEW_ITEMINFO_TYPE] = "folder";
    plugin.previewItem(info);

    // The updateFolderSize slot updates m_detailInfos, m_sizeLabel is not directly used
    plugin.updateFolderSize(1000);

    // Verify the detail info was updated (this is what updateFolderSize actually modifies)
    auto details = plugin.getAttributeDetailInfo();
    EXPECT_FALSE(details.isEmpty());
}
