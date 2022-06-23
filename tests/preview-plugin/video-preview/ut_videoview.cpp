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
#include "video-preview/videopreview_global.h"
#include "video-preview/videoview.h"

#include <QLayout>

#include <stubext.h>

#include <gtest/gtest.h>

VIDEO_PREVIEW_USE_NAMESPACE

TEST(VideoView, ut_initUI)
{
    VideoView view;
    ASSERT_EQ(view.m_title, nullptr);
    ASSERT_EQ(view.m_picFrame, nullptr);

    view.initUI();
    ASSERT_NE(view.m_title, nullptr);
    ASSERT_NE(view.m_picFrame, nullptr);

    EXPECT_EQ(view.layout()->contentsMargins(), QMargins(35, 12, 35, 0));
    EXPECT_EQ(view.m_picFrame->size(), QSize(310, 110));

    EXPECT_EQ(view.m_title->width(), 310);
}

TEST(VideoView, ut_setTitle)
{
    VideoView view;
    EXPECT_NO_FATAL_FAILURE(view.setTitle("title"));

    view.initUI();
    view.setTitle("title");
    EXPECT_EQ(view.m_title->text(), QString("title"));
    EXPECT_TRUE(view.m_title->toolTip().isEmpty());

    QString longStr = "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
                      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss"
                      "ssssssssssssssssssssssssssssssssssssssssssssssssssssssssss";
    view.setTitle(longStr);
    EXPECT_EQ(view.m_title->text(), longStr);
    EXPECT_FALSE(view.m_title->toolTip().isEmpty());

    view.setTitle("title");
    EXPECT_TRUE(view.m_title->toolTip().isEmpty());
}

TEST(VideoView, ut_setThumbnail)
{
    VideoView view;
    view.initUI();
    EXPECT_TRUE(view.m_picFrame->m_pixmap.isNull());

    QPixmap pix(100, 100);
    pix.fill();
    view.setThumbnail(pix);
    EXPECT_FALSE(view.m_picFrame->m_pixmap.isNull());

    EXPECT_FALSE(view.m_picFrame->m_picLabel->pixmap()->isNull());
    EXPECT_EQ(view.m_picFrame->m_picLabel->size(), pix.size());
}

TEST(VideoView, ut_maxThumbnailSize)
{
    EXPECT_EQ(VideoView::maxThumbnailSize(), QSize(310, 110));
}
