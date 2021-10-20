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

#include "contacts/interface/daemongrandsearchinterface.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;

namespace {
class DaemonGrandSearchInterfaceTest : public Test
{
public:
    DaemonGrandSearchInterfaceTest() : Test()
    {
        stu = new stub_ext::StubExt;
        stu->set_lamda(ADDR(QDBusAbstractInterface, asyncCallWithArgumentList), [&](){
            isCall = true;
            QDBusMessage message;
            return QDBusPendingCall::fromCompletedCall(message);
        });

        interface = new DaemonGrandSearchInterface;
    }
    ~DaemonGrandSearchInterfaceTest()
    {
        delete stu;
        delete interface;
    }

    stub_ext::StubExt *stu = nullptr;
    DaemonGrandSearchInterface *interface;
    bool isCall = false;
};

}

TEST_F(DaemonGrandSearchInterfaceTest, init)
{
    ASSERT_TRUE(interface);
}

TEST_F(DaemonGrandSearchInterfaceTest, Configuration)
{
    interface->Configuration();
    EXPECT_TRUE(isCall);
}
TEST_F(DaemonGrandSearchInterfaceTest, Configure)
{
    QVariantMap data;
    data.insert("testKey", "tsetValue");
    interface->Configure(data);
    EXPECT_TRUE(isCall);
}
TEST_F(DaemonGrandSearchInterfaceTest, FeedBackStrategy)
{
    interface->FeedBackStrategy();
    EXPECT_TRUE(isCall);
}
TEST_F(DaemonGrandSearchInterfaceTest, KeepAlive)
{
    QString missionId("testId");
    interface->KeepAlive(missionId);
    EXPECT_TRUE(isCall);
}
TEST_F(DaemonGrandSearchInterfaceTest, MatchedBuffer)
{
    QString missionId("testId");
    interface->MatchedBuffer(missionId);
    EXPECT_TRUE(isCall);
}
TEST_F(DaemonGrandSearchInterfaceTest, MatchedResults)
{
    QString missionId("testId");
    interface->MatchedResults(missionId);
    EXPECT_TRUE(isCall);
}
TEST_F(DaemonGrandSearchInterfaceTest, OpenWithPlugin)
{
    QString searcher("testSearcher");
    QString item("testItem");
    interface->OpenWithPlugin(searcher, item);
    EXPECT_TRUE(isCall);
}
TEST_F(DaemonGrandSearchInterfaceTest, Search)
{
    QString missionId("testId");
    QString key("testKey");
    interface->Search(missionId, key);
    EXPECT_TRUE(isCall);
}
TEST_F(DaemonGrandSearchInterfaceTest, SetFeedBackStrategy)
{
    QVariantMap data;
    data.insert("testKey", "tsetValue");
    interface->SetFeedBackStrategy(data);
    EXPECT_TRUE(isCall);
}
TEST_F(DaemonGrandSearchInterfaceTest, Terminate)
{
    interface->Terminate();
    EXPECT_TRUE(isCall);
}
