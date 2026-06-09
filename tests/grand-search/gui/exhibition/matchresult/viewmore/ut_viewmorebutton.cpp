// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    btn.show();
    QPaintEvent event(QRect(0, 0, 100, 30));

    stub_ext::StubExt st;
    st.set_lamda(&DGuiApplicationHelper::themeType, [&](){
        return DGuiApplicationHelper::DarkType;
    });

    // Test basic paint without stubbing initStyleOption (which is inherited and can't be safely stubbed)
    // normal, not selected
    EXPECT_NO_FATAL_FAILURE(btn.paintEvent(&event));
    // normal, selected
    btn.m_bIsSelected = true;
    EXPECT_NO_FATAL_FAILURE(btn.paintEvent(&event));

    // Test with light theme
    st.reset(&DGuiApplicationHelper::themeType);
    st.set_lamda(&DGuiApplicationHelper::themeType, [&](){
        return DGuiApplicationHelper::LightType;
    });
    btn.m_bIsSelected = false;
    EXPECT_NO_FATAL_FAILURE(btn.paintEvent(&event));
    btn.m_bIsSelected = true;
    EXPECT_NO_FATAL_FAILURE(btn.paintEvent(&event));
}
