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

#include "gui/exhibition/preview/generalwidget/generaltoolbar.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <DGuiApplicationHelper>

#include <QFont>

using namespace testing;
DGUI_USE_NAMESPACE;

TEST(GeneralToolBarTest, constructor)
{
    stub_ext::StubExt stu;

    stu.set_lamda(ADDR(QFont, pixelSize), [&](){
        return 12;
    });

    GeneralToolBar *w = new GeneralToolBar;

    ASSERT_TRUE(w);
    ASSERT_TRUE(w->m_openBtn);
    ASSERT_TRUE(w->m_openPathBtn);
    ASSERT_TRUE(w->m_copyPathBtn);

    Qt::ToolButtonStyle style = w->m_openBtn->toolButtonStyle();
    EXPECT_EQ(style, Qt::ToolButtonTextBesideIcon);

    delete w;
}

TEST(GeneralToolBarTest, constructor_max)
{
    stub_ext::StubExt stu;

    DGuiApplicationHelper::ColorType ut_themeType = DGuiApplicationHelper::DarkType;
    stu.set_lamda(ADDR(DGuiApplicationHelper, themeType), [&](){
        return ut_themeType;
    });

    stu.set_lamda(ADDR(QFont, pixelSize), [&](){
        return 20;
    });

    GeneralToolBar w;

    Qt::ToolButtonStyle style = w.m_openBtn->toolButtonStyle();
    EXPECT_EQ(style, Qt::ToolButtonIconOnly);
}
