/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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

#include "gui/searchconfig/customwidget.h"
#include "gui/searchconfig/switchwidget/switchwidget.h"
#include "business/config/searchconfig.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QFileInfo>
#include <QDir>

using namespace testing;
using namespace GrandSearch;

TEST(CustomWidgetTest, constructor)
{
    stub_ext::StubExt stu;

    bool ut_call_updateIcons = false;
    stu.set_lamda(&CustomWidget::updateIcons, [&]() {
        ut_call_updateIcons = true;
    });

    CustomWidget *w = new CustomWidget;

    EXPECT_TRUE(w);
    EXPECT_TRUE(ut_call_updateIcons);

    delete w;
}

TEST(CustomWidgetTest, onSwitchStateChanged)
{
    CustomWidget w;

    stub_ext::StubExt stu;

    SwitchWidget *switchWidget = new SwitchWidget;
    stu.set_lamda(&QObject::sender, [&]() {
        return switchWidget;
    });

    bool ut_call_setConfig = false;
    stu.set_lamda(&SearchConfig::setConfig, [&]() {
        ut_call_setConfig = true;
    });

    w.onSwitchStateChanged(true);

    EXPECT_TRUE(ut_call_setConfig);

    delete switchWidget;
}

TEST(CustomWidgetTest, updateIcons)
{
    CustomWidget w;

    stub_ext::StubExt stu;

    bool ut_call_setIcon = false;
    stu.set_lamda(&SwitchWidget::setIcon, [&]() {
        ut_call_setIcon = true;
    });

    w.updateIcons();
    EXPECT_TRUE(ut_call_setIcon);
}
