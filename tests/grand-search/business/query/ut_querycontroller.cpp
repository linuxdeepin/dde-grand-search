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

#include "stubext.h"

#include <gtest/gtest.h>

#include <QDBusPendingReply>

#define private public
#define protected public
#include "business/query/querycontroller.h"
#include "business/query/querycontroller_p.h"
#include "contacts/interface/daemongrandsearchinterface.h"

using namespace testing;

TEST(QueryController, constructor)
{
    QueryController *queryController = new QueryController;
    ASSERT_TRUE(queryController);
    ASSERT_TRUE(queryController->d_p);
    delete queryController;
}

TEST(QueryController, onSearchTextChanged)
{
    QueryController queryController;
    ASSERT_TRUE(queryController.d_p);

    bool reciveSigMissionChanged = false;
    QObject::connect(&queryController, &QueryController::missionChanged, qApp, [&](){
        reciveSigMissionChanged = true;
    });

    QString txt("test");
    queryController.d_p->m_searchText = txt;
    queryController.onSearchTextChanged(txt);
    EXPECT_FALSE(reciveSigMissionChanged);

    reciveSigMissionChanged = false;
    txt.clear();
    queryController.onSearchTextChanged(txt);
    EXPECT_TRUE(reciveSigMissionChanged);
    EXPECT_TRUE(queryController.d_p->m_searchText.isEmpty());
    EXPECT_TRUE(queryController.d_p->m_missionId.isEmpty());

    reciveSigMissionChanged = false;

    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(DaemonGrandSearchInterface, Search), [&](){
        return QDBusPendingReply<bool>();
    });

    txt = "testSearch";
    queryController.onSearchTextChanged(txt);
    EXPECT_TRUE(reciveSigMissionChanged);
    EXPECT_FALSE(queryController.d_p->m_missionId.isEmpty());
}

TEST(QueryController, onTerminateSearch)
{
    QueryController queryController;
    ASSERT_TRUE(queryController.d_p);

    bool isCall = false;
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(DaemonGrandSearchInterface, Terminate), [&](){
        isCall = true;
        return QDBusPendingReply<>();
    });

    queryController.d_p->m_missionId.clear();
    queryController.onTerminateSearch();
    EXPECT_FALSE(isCall);

    queryController.d_p->m_missionId = "testId";
    queryController.onTerminateSearch();
    EXPECT_TRUE(isCall);
}

TEST(QueryController, getMissionID)
{
    QueryController queryController;
    ASSERT_TRUE(queryController.d_p);

    QString missionId("testId");
    queryController.d_p->m_missionId = missionId;
    QString actual = queryController.getMissionID();
    EXPECT_EQ(actual, missionId);
}

TEST(QueryController, isEmptySearchText)
{
    QueryController queryController;
    ASSERT_TRUE(queryController.d_p);
}

TEST(QueryControllerPrivate, keepAlive)
{
    QueryController queryController;
    ASSERT_TRUE(queryController.d_p);

    bool isCall = false;
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(DaemonGrandSearchInterface, KeepAlive), [&](){
        isCall = true;
        return QDBusPendingReply<>();
    });

    queryController.d_p->keepAlive();
    EXPECT_TRUE(isCall);
}

TEST(QueryControllerPrivate, onSearchCompleted)
{
    QueryController queryController;
    ASSERT_TRUE(queryController.d_p);

    QString missionId("testId");
    queryController.d_p->m_missionId = missionId;
    queryController.d_p->onSearchCompleted(missionId);
    EXPECT_FALSE(queryController.d_p->m_keepAliveTimer->isActive());
}
