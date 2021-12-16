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

#include <QSignalSpy>

#define private public
#define protected public
#include "business/matchresult/matchcontroller.h"
#include "business/matchresult/matchcontroller_p.h"
#include "contacts/interface/daemongrandsearchinterface.h"

using namespace testing;

TEST(MatchController, constructor)
{
    MatchController *matchController = new MatchController;
    ASSERT_TRUE(matchController);
    ASSERT_TRUE(matchController->d_p);
    delete matchController;
}

TEST(MatchController, onMissionIdChanged)
{
    MatchController matchController;

    QString key("aaaa");
    matchController.onMissionIdChanged(key);

    EXPECT_EQ(key, matchController.d_p->m_missionId);
}

TEST(MatchControllerPrivate, constructor)
{
    bool isCall = false;
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(MatchControllerPrivate, initConnect), [&](){
        isCall = true;
    });

    MatchController matchController;
    ASSERT_TRUE(matchController.d_p);
    ASSERT_TRUE(matchController.d_p->m_daemonDbus);
    EXPECT_TRUE(isCall);
}

TEST(MatchControllerPrivate, initConnect)
{
    MatchController matchController;
    matchController.d_p->initConnect();

    bool isCall = false;
    QString reciveId;
    stub_ext::StubExt stu;
    stu.set_lamda(ADDR(MatchControllerPrivate, onMatched), [&](MatchControllerPrivate *obj, QString missionId){
        Q_UNUSED(obj)

        isCall = true;
        reciveId = missionId;
    });

    QString missionId("testId");
    matchController.d_p->m_daemonDbus->Matched(missionId);
    EXPECT_TRUE(isCall);
    EXPECT_EQ(reciveId, missionId);

    isCall = false;
    reciveId.clear();

    stu.set_lamda(ADDR(MatchControllerPrivate, onSearchCompleted), [&](MatchControllerPrivate *obj, QString missionId){
        Q_UNUSED(obj)

        isCall = true;
        reciveId = missionId;
    });

    matchController.d_p->m_daemonDbus->SearchCompleted(missionId);
    EXPECT_TRUE(isCall);
    EXPECT_EQ(reciveId, missionId);
}

TEST(MatchControllerPrivate, onMatched)
{
    stub_ext::StubExt stu;
    stu.set_lamda(&DaemonGrandSearchInterface::MatchedBuffer, [](){
        QDBusPendingReply<QByteArray> reply;;
        return reply;
    });

    MatchController matchController;

    QSignalSpy spy(&matchController, &MatchController::matchedResult);

    EXPECT_EQ(spy.count(), 0);
    matchController.d_p->m_missionId.clear();
    matchController.d_p->onMatched(QString());
    EXPECT_EQ(spy.count(), 0);

    QString missionId("testId");
    matchController.d_p->m_missionId = missionId;
    matchController.d_p->onMatched(missionId);
    EXPECT_EQ(spy.count(), 1);
}

TEST(MatchControllerPrivate, onSearchCompleted)
{
    bool reciveSig = false;
    MatchController matchController;
    QObject::connect(&matchController, &MatchController::searchCompleted, qApp, [&](){
        reciveSig = true;
    });

    matchController.d_p->m_missionId.clear();
    matchController.d_p->onSearchCompleted(QString());
    EXPECT_FALSE(reciveSig);

    QString missionId("testId");
    matchController.d_p->m_missionId = missionId;
    matchController.d_p->onSearchCompleted(missionId);
    EXPECT_TRUE(reciveSig);
}
