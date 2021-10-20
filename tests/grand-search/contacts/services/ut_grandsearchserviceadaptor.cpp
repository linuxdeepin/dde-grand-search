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

#include "contacts/services/grandsearchserviceadaptor.h"
#include "contacts/services/grandsearchservice.h"
#include "gui/mainwindow.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;

TEST(GrandSearchServiceAdaptorTest, constructor)
{
    MainWindow w;
    GrandSearchService service(&w);
    GrandSearchServiceAdaptor *adaptor = new GrandSearchServiceAdaptor(&service);

    ASSERT_TRUE(adaptor);

    delete adaptor;
}

TEST(GrandSearchServiceAdaptorTest, IsVisible)
{
    MainWindow w;
    GrandSearchService service(&w);
    GrandSearchServiceAdaptor adaptor(&service);

    stub_ext::StubExt stu;
    bool isCall = false;
    stu.set_lamda(ADDR(GrandSearchService, IsVisible), [&](){
        isCall = true;
        return isCall;
    });

    adaptor.IsVisible();
    EXPECT_TRUE(isCall);
}

TEST(GrandSearchServiceAdaptorTest, SetVisible)
{
    MainWindow w;
    GrandSearchService service(&w);
    GrandSearchServiceAdaptor adaptor(&service);

    stub_ext::StubExt stu;
    bool isCall = false;
    stu.set_lamda(ADDR(GrandSearchService, SetVisible), [&](){
        isCall = true;
    });

    adaptor.SetVisible(isCall);
    EXPECT_TRUE(isCall);
}
