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

#include "global/grandsearch_global.h"
#include "gui/mainwindow.h"

#include "stubext.h"

#include <gtest/gtest.h>

#define private public
#define protected public
#include "contacts/services/grandsearchservice.h"
#include "contacts/services/grandsearchservice_p.h"

using namespace testing;
GRANDSEARCH_USE_NAMESPACE

TEST(GrandSearchServiceTest, constructor)
{
    MainWindow w;
    GrandSearchService *service = new GrandSearchService(&w);

    ASSERT_TRUE(service);
    ASSERT_TRUE(service->d_p);
    EXPECT_EQ(service->d_p->m_mainWindow, &w);

    delete service;
}

TEST(GrandSearchServiceTest, IsVisible)
{
    MainWindow w;
    GrandSearchService service(&w);

    bool actual = service.IsVisible();

    EXPECT_EQ(actual, service.d_p->m_mainWindow->isVisible());
}

TEST(GrandSearchServiceTest, SetVisible)
{
    MainWindow w;
    GrandSearchService service(&w);

    // 测试状态相等直接返回
    bool visible = service.d_p->m_mainWindow->isVisible();
    service.SetVisible(visible);

    // 测试状态不等进行设置
    stub_ext::StubExt stu;

    bool isCallClose = false;
    stu.set_lamda(ADDR(QWidget, close), [&](){
        isCallClose = true;
        return isCallClose;
    });

    bool testVisible = false;
    stu.set_lamda(ADDR(QWidget, isVisible), [&](){
        return testVisible;
    });

    typedef void (*fptr)(QWidget*, bool);
    fptr QWidget_setVisible = (fptr)(&QWidget::setVisible);

    bool isCallSetVisible = false;
    stu.set_lamda(QWidget_setVisible, [&](){
        isCallSetVisible = true;
    });

    // 设置为显示
    visible = true;
    service.SetVisible(visible);
    EXPECT_FALSE(isCallClose);
    EXPECT_TRUE(isCallSetVisible);

    // 设置为隐藏
    testVisible =true;
    visible = false;
    isCallSetVisible = false;
    service.SetVisible(visible);
    EXPECT_TRUE(isCallClose);
    EXPECT_FALSE(isCallSetVisible);
}
