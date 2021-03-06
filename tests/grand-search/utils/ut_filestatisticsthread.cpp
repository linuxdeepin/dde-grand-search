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

#include "utils/filestatisticsthread.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QTimer>
#include <QtTest>
#include <QThread>

using namespace testing;

TEST(FileStatisticsThreadest, constructor)
{
    FileStatisticsThread *fst = new FileStatisticsThread;

    EXPECT_TRUE(fst);
    EXPECT_TRUE(fst->m_notifyDataTimer);

    fst->m_notifyDataTimer->start(1);

    QTest::qWait(10);

    delete fst;
}

TEST(FileStatisticsThreadest, start)
{
    FileStatisticsThread fst;

    stub_ext::StubExt stu;

    bool ut_isRunning = true;
    stu.set_lamda(ADDR(QThread, isRunning), [&](){
        return ut_isRunning;
    });

    QString sourceFile;

    fst.m_sourceFile = "/usr/bin/";
    fst.start(sourceFile);
    EXPECT_FALSE(fst.m_sourceFile.isEmpty());

    ut_isRunning = false;
    fst.start(sourceFile);
    EXPECT_FALSE(fst.m_sourceFile.isEmpty());

    bool ut_call_start = false;
    stu.set_lamda(&QThread::start, [&]() {
        ut_call_start = true;
    });

    sourceFile = "/home/";
    fst.start(sourceFile);
    EXPECT_EQ(fst.m_sourceFile, sourceFile);
    EXPECT_TRUE(ut_call_start);
}

TEST(FileStatisticsThreadest, stop)
{
    FileStatisticsThread fst;

    stub_ext::StubExt stu;

    bool ut_setState = false;
    stu.set_lamda(ADDR(FileStatisticsThread, setState), [&](){
        ut_setState = true;
    });

    fst.stop();
    EXPECT_TRUE(ut_setState);
}

TEST(FileStatisticsThreadest, run)
{
    FileStatisticsThread *fst = new FileStatisticsThread;

    stub_ext::StubExt stu;

    fst->start("/home/");

    QTest::qWait(500);
    fst->stop();
    QTest::qWait(500);

    EXPECT_EQ(fst->m_state, FileStatisticsThread::StoppedState);

    fst->deleteLater();
}

TEST(FileStatisticsThreadest, setState)
{
    FileStatisticsThread fst;

    fst.m_state = FileStatisticsThread::StoppedState;

    fst.setState(FileStatisticsThread::RunningState);
    EXPECT_EQ(fst.m_state, FileStatisticsThread::RunningState);

    fst.setState(FileStatisticsThread::StoppedState);
    EXPECT_EQ(fst.m_state, FileStatisticsThread::StoppedState);
}

TEST(FileStatisticsThreadest, stateCheck)
{
    FileStatisticsThread fst;

    fst.m_state = FileStatisticsThread::StoppedState;

    bool result = fst.stateCheck();
    EXPECT_FALSE(result);

    fst.m_state = FileStatisticsThread::RunningState;
    result = fst.stateCheck();
    EXPECT_TRUE(result);
}

TEST(FileStatisticsThreadest, processFile)
{
    FileStatisticsThread fst;

    QString file("/home/");
    QQueue<QString> dirQueue;

    fst.processFile(file, dirQueue);
    EXPECT_FALSE(dirQueue.isEmpty());

    file = "/usr/bin/apt";
    dirQueue.clear();

    fst.processFile(file, dirQueue);
    EXPECT_TRUE(dirQueue.isEmpty());
    EXPECT_NE(fst.m_totalSize, 0);
}
