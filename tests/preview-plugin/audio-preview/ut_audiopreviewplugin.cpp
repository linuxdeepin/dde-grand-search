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
#include "audio-preview/audiopreviewplugin.h"
#include "audio-preview/audiofileinfo.h"
#include "audio-preview/audioview.h"
#include "audio-preview/libaudioviewer.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

using namespace GrandSearch;
TEST(AudioPreviewPluginTest, ut_init)
{
    AudioPreviewPlugin plugin;
    plugin.init(nullptr);
    EXPECT_TRUE(plugin.m_audioView);
}

TEST(AudioPreviewPluginTest, ut_previewItem)
{
    stub_ext::StubExt st;
    st.set_lamda(&AudioFileInfo::openAudioFile, [](){
        AudioFileInfo::AudioMetaData data;
        data.artist = "Artist";
        data.album = "Album";
        data.duration = "00:00:10";
        return data;
    });
    st.set_lamda(&AudioView::setItemInfo, [](){ return; });
    st.set_lamda(&QFileInfo::lastModified, [](){
        return QDateTime::currentDateTime();
    });
    st.set_lamda(&QFileInfo::absoluteFilePath, [](){ return "/home"; });

    GrandSearch::ItemInfo info;
    info[PREVIEW_ITEMINFO_ITEM] = "";

    AudioPreviewPlugin plugin;
    plugin.init(nullptr);
    EXPECT_FALSE(plugin.previewItem(info));

    info[PREVIEW_ITEMINFO_ITEM] = "/test.mp3";
    EXPECT_TRUE(plugin.previewItem(info));
}

TEST(AudioPreviewPluginTest, ut_previewItem_2)
{
    stub_ext::StubExt st;
    st.set_lamda(&AudioFileInfo::openAudioFile, [](){
        AudioFileInfo::AudioMetaData data;
        data.artist = "Artist";
        data.album = "Album";
        data.duration.clear();
        return data;
    });
    st.set_lamda(&AudioView::setItemInfo, [](){ return; });
    st.set_lamda(&QFileInfo::lastModified, [](){
        return QDateTime::currentDateTime();
    });
    st.set_lamda(&QFileInfo::absoluteFilePath, [](){ return "/home"; });

    GrandSearch::ItemInfo info;
    info[PREVIEW_ITEMINFO_ITEM] = "/test.mp3";

    AudioPreviewPlugin plugin;
    plugin.init(nullptr);
    ASSERT_EQ(plugin.m_parser.get(), nullptr);

    bool called = false;
    st.set_lamda(&LibAudioViewer::getDuration, [&called](LibAudioViewer *self, const QUrl &url, qint64 &duration){
        duration = 10;
        called = true;
        return true;
    });

    EXPECT_TRUE(plugin.previewItem(info));
    EXPECT_FALSE(called);

    called = false;
    plugin.m_parser.reset(new LibAudioViewer);

    EXPECT_TRUE(plugin.previewItem(info));
    EXPECT_TRUE(called);
}

TEST(AudioPreviewPluginTest, ut_item)
{
    AudioPreviewPlugin plugin;
    EXPECT_TRUE(plugin.item().isEmpty());
}

TEST(AudioPreviewPluginTest, ut_stopPreview)
{
    AudioPreviewPlugin plugin;
    EXPECT_TRUE(plugin.stopPreview());
}

TEST(AudioPreviewPluginTest, ut_contentWidget)
{
    AudioPreviewPlugin plugin;
    EXPECT_FALSE(plugin.contentWidget());
}

TEST(AudioPreviewPluginTest, ut_getAttributeDetailInfo)
{
    AudioPreviewPlugin plugin;
    EXPECT_TRUE(plugin.getAttributeDetailInfo().isEmpty());
}

TEST(AudioPreviewPluginTest, ut_toolBarWidget)
{
    AudioPreviewPlugin plugin;
    EXPECT_FALSE(plugin.toolBarWidget());
}

TEST(AudioPreviewPluginTest, ut_showToolBar)
{
    AudioPreviewPlugin plugin;
    EXPECT_TRUE(plugin.showToolBar());
}
