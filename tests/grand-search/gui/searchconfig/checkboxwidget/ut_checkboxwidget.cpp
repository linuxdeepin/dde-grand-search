/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liqianga@uniontech.com>
 *
 * Maintainer: liuzhangjian<liqianga@uniontech.com>
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
#include "gui/searchconfig/checkboxwidget/checkboxwidget.h"
#include "gui/searchconfig/checkboxwidget/checkboxitem.h"
#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;

TEST(CheckBoxWidgetTest, addCheckBox)
{
    CheckBoxWidget w;
    EXPECT_EQ(0, w.addCheckBox("1"));
    EXPECT_EQ(1, w.addCheckBox("2"));
    EXPECT_EQ(2, w.addCheckBox("3"));
}

TEST(CheckBoxWidgetTest, setCheckBoxChecked)
{
    CheckBoxWidget w;
    w.addCheckBox("test");
    EXPECT_TRUE(w.setCheckBoxChecked("test", true));
}

TEST(CheckBoxWidgetTest, onCheckBoxChecked)
{
    CheckBoxWidget w;
    w.addCheckBox("test");

    stub_ext::StubExt st;
    st.set_lamda(&QObject::sender, [&]() {
        return w.m_checkBoxList.first();
    });

    EXPECT_NO_FATAL_FAILURE(w.onCheckBoxChecked(true));
}
