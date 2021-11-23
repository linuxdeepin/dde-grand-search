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

#include "contacts/interface/grandsearchinterface.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;

namespace {
class GrandSearchInterfaceTest : public Test
{
public:
    GrandSearchInterfaceTest() : Test()
    {
        stu = new stub_ext::StubExt;
        stu->set_lamda(ADDR(QDBusAbstractInterface, asyncCallWithArgumentList), [&](){
            isCall = true;
            QDBusMessage message;
            return QDBusPendingCall::fromCompletedCall(message);
        });

        interface = new GrandSearchInterface;
    }
    ~GrandSearchInterfaceTest()
    {
        delete stu;
        delete interface;
    }

    stub_ext::StubExt *stu = nullptr;
    GrandSearchInterface *interface;
    bool isCall = false;
};

}

TEST_F(GrandSearchInterfaceTest, IsVisible)
{
    interface->IsVisible();

    EXPECT_TRUE(isCall);
}

TEST_F(GrandSearchInterfaceTest, SetVisible)
{
    interface->SetVisible(false);

    EXPECT_TRUE(isCall);
}
