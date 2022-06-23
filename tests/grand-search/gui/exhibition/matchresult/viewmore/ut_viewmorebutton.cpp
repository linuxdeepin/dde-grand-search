/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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

#include "global/grandsearch_global.h"
#include "gui/exhibition/matchresult/viewmore/viewmorebutton.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <DGuiApplicationHelper>
#include <DStyle>

#include <QPaintEvent>
#include <QStyleOptionToolButton>

GRANDSEARCH_USE_NAMESPACE
DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

TEST(ViewMoreButtonTest, setSelected)
{
    ViewMoreButton btn("test");
    btn.setSelected(false);
    EXPECT_FALSE(btn.m_bIsSelected);

    stub_ext::StubExt st;
    st.set_lamda((void(ViewMoreButton::*)())ADDR(ViewMoreButton, update), [](){
        return ;
    });
    btn.setSelected(true);
    EXPECT_TRUE(btn.m_bIsSelected);
}

TEST(ViewMoreButtonTest, isSelected)
{
    ViewMoreButton btn("test");
    bool selected = btn.isSelected();
    EXPECT_EQ(btn.m_bIsSelected, selected);
}

TEST(ViewMoreButtonTest, paintEvent)
{
    ViewMoreButton btn("test");
    QPaintEvent event((QRect()));

    stub_ext::StubExt st;
    st.set_lamda(&DGuiApplicationHelper::themeType, [&](){
        return DGuiApplicationHelper::DarkType;
    });

    st.set_lamda(&ViewMoreButton::initStyleOption, [](QToolButton *, QStyleOptionToolButton *opt){
        opt->state |= QStyle::State_Active;
    });

    // normal, not selected
    st.set_lamda(&DStyle::getState, [](){ return DStyle::SS_NormalState; });
    EXPECT_NO_FATAL_FAILURE(btn.paintEvent(&event));
    // normal, selected
    btn.m_bIsSelected = true;
    EXPECT_NO_FATAL_FAILURE(btn.paintEvent(&event));

    // hover
    st.reset(&DStyle::getState);
    st.set_lamda(&DStyle::getState, [](){ return DStyle::SS_HoverState; });
    EXPECT_NO_FATAL_FAILURE(btn.paintEvent(&event));

    // press
    st.reset(&DStyle::getState);
    st.set_lamda(&DStyle::getState, [](){ return DStyle::SS_PressState; });
    EXPECT_NO_FATAL_FAILURE(btn.paintEvent(&event));
}
