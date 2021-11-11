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
#include "audio-preview/audioview.h"
#include "global/commontools.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

TEST(AudioViewTest, ut_setItemInfo_0)
{
    stub_ext::StubExt st;
    st.set_lamda(GrandSearch::CommonTools::lineFeed, [](){ return "aaaa.mp3"; });
    st.set_lamda(GrandSearch::CommonTools::formatFileSize, [](){ return "1 MB"; });

    GrandSearch::ItemInfo info;
    info[PREVIEW_ITEMINFO_ICON] = "audio-mpeg";
    info[PREVIEW_ITEMINFO_NAME] = "AAAA.mp3";
    info[PREVIEW_ITEMINFO_ITEM] = "/test.mp3";

    AudioView view;
    EXPECT_NO_FATAL_FAILURE(view.setItemInfo(info));
}

TEST(AudioViewTest, ut_setItemInfo_1)
{
    stub_ext::StubExt st;
    st.set_lamda(GrandSearch::CommonTools::lineFeed, [](){ return "aaaa.mp3"; });
    st.set_lamda(GrandSearch::CommonTools::formatFileSize, [](){ return "1 MB"; });

    GrandSearch::ItemInfo info;
    info[PREVIEW_ITEMINFO_ICON] = "";
    info[PREVIEW_ITEMINFO_NAME] = "AAAA.mp3";
    info[PREVIEW_ITEMINFO_ITEM] = "/test.mp3";

    AudioView view;
    EXPECT_NO_FATAL_FAILURE(view.setItemInfo(info));
}
