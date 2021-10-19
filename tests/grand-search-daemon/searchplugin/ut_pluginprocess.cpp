/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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

#include "searchplugin/process/pluginprocess.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>
#include <QProcess>

TEST(PluginLiaison, ut_addProgram)
{
    PluginProcess pp;
    EXPECT_TRUE(pp.addProgram("test", "/usr/bin/test -t test"));
    ASSERT_EQ(pp.m_processes.size(), 1);
    EXPECT_TRUE(pp.m_programs.contains("test"));

    auto pcs = pp.m_processes.value("test");
    ASSERT_NE(pcs, nullptr);
    EXPECT_EQ(pcs->program(), QString("/usr/bin/test"));
    QStringList args{"-t","test"};
    EXPECT_EQ(pcs->arguments(), args);

    stub_ext::StubExt st;
    bool stachanged = false;
    st.set_lamda(&PluginProcess::processStateChanged, [&stachanged]() {
        stachanged = true;
    });

    EXPECT_FALSE(pp.addProgram("test", "/usr/bin/test2 -t test2"));

    emit pcs->stateChanged(QProcess::Running, QProcess::QPrivateSignal());
    //队列
    EXPECT_FALSE(stachanged);

    stachanged = false;
    QTest::qWaitFor([&stachanged](){return stachanged;}, 30);
    EXPECT_TRUE(stachanged);
}

TEST(PluginLiaison, ut_setWatched)
{
    PluginProcess pp;
    pp.setWatched("test", true);
    EXPECT_TRUE(pp.m_watch.contains("test"));

    pp.setWatched("test", false);
    EXPECT_FALSE(pp.m_watch.contains("test"));
}

TEST(PluginLiaison, ut_clear)
{
    PluginProcess pp;
    pp.addProgram("test", "/usr/bin/test -t test");
    pp.setWatched("test", true);

    EXPECT_FALSE(pp.m_watch.isEmpty());
    EXPECT_FALSE(pp.m_processes.isEmpty());
    EXPECT_FALSE(pp.m_programs.isEmpty());

    pp.clear();
    EXPECT_TRUE(pp.m_watch.isEmpty());
    EXPECT_TRUE(pp.m_processes.isEmpty());
    EXPECT_TRUE(pp.m_programs.isEmpty());
}

TEST(PluginLiaison, ut_timerEvent)
{
    QTimerEvent e(-1);
    stub_ext::StubExt st;
    QProcess *p1 = (QProcess *)1;
    st.set_lamda(&PluginProcess::checkStability, [&p1](PluginProcess *, QProcess *p) {
        p1 = p;
    });

    QProcess *p2 = (QProcess *)2;
    st.set_lamda(&PluginProcess::removeChecklist, [&p2](PluginProcess *, QProcess *p) {
        p2 = p;
    });
    PluginProcess pp;
    pp.timerEvent(&e);
    EXPECT_EQ((long)p1, 1);
    EXPECT_EQ((long)p2, 2);
    QProcess pcs;
    pp.m_checklist.insert(&pcs, -1);

    pp.timerEvent(&e);
    EXPECT_EQ(p1, &pcs);
    EXPECT_EQ(p2, &pcs);
}

TEST(PluginLiaison, ut_startProgram)
{
    PluginProcess pp;
    pp.addProgram("test", "/usr/bin/test -t test");

    bool started = false;
    stub_ext::StubExt st;
    auto startFunc = (void (QProcess::*)(QProcess::OpenMode))(&QProcess::start);
    st.set_lamda(startFunc, [&started]() {
        started = true;
    });

    for (int i = 0; i < 4; ++i) {
        started = false;
        bool ret = pp.startProgram("test");
        if (i > 2) { //超过三次启动失败
            EXPECT_FALSE(ret);
            EXPECT_FALSE(started);
        } else {
            EXPECT_TRUE(ret);
            EXPECT_TRUE(started);
        }
        //启动次数
        EXPECT_EQ(pp.m_restartCount.value(pp.m_processes.value("test")), i + 1);
    }
}

TEST(PluginLiaison, ut_terminate)
{
    PluginProcess pp;
    pp.addProgram("test", "/usr/bin/test -t test");

    bool terminate = false;
    stub_ext::StubExt st;
    st.set_lamda(&QProcess::terminate, [&terminate]() {
        terminate = true;
    });

    QProcess::ProcessState state = QProcess::Running;
    st.set_lamda(&QProcess::state, [&state]() {
        return state;
    });

    QProcess *pcs = nullptr;
    st.set_lamda(&PluginProcess::removeChecklist, [&pcs](PluginProcess *,QProcess *p ) {
        pcs = p;
    });

    pp.m_restartCount.insert(pp.m_processes.value("test"), 1);

    pp.terminate("test2");
    EXPECT_FALSE(terminate);
    EXPECT_EQ(pp.m_restartCount.value(pp.m_processes.value("test")), 1);
    EXPECT_EQ(pcs, nullptr);

    state = QProcess::NotRunning;
    pcs = nullptr;
    terminate = false;
    pp.m_restartCount.insert(pp.m_processes.value("test"), 2);
    pp.terminate("test");
    EXPECT_FALSE(terminate);
    EXPECT_EQ(pp.m_restartCount.value(pp.m_processes.value("test")), 2);
    EXPECT_EQ(pcs, nullptr);

    state = QProcess::Running;
    pcs = nullptr;
    terminate = false;
    pp.m_restartCount.insert(pp.m_processes.value("test"), 3);
    pp.terminate("test");
    EXPECT_TRUE(terminate);
    EXPECT_TRUE(pp.m_restartCount.isEmpty());
    EXPECT_EQ(pcs, pp.m_processes.value("test"));
}

TEST(PluginLiaison, ut_processStateChanged_queue)
{
    PluginProcess pp;
    pp.addProgram("test", "/usr/bin/test -t test");
    stub_ext::StubExt st;
    bool called = false;
    st.set_lamda(&PluginProcess::processStateChanged, [&called]() {
        called = true;
    });

    auto pcs = pp.m_processes.value("test");
    emit pcs->stateChanged(QProcess::Running, QProcess::QPrivateSignal());
    EXPECT_FALSE(called);

    //必须队列响应
    called = false;
    QTest::qWaitFor([&called](){return called;}, 30);
    EXPECT_TRUE(called);
}

TEST(PluginLiaison, ut_processStateChanged)
{
    PluginProcess pp;
    pp.addProgram("test", "/usr/bin/test -t test");
    stub_ext::StubExt st;
    QProcess::ProcessState state = QProcess::Running;
    bool called;
    st.set_lamda(&QProcess::state, [&state, &called]() {
        called = true;
        return state;
    });

    QProcess *pcs = nullptr;
    st.set_lamda(&PluginProcess::addChecklist, [&pcs](PluginProcess *,QProcess *p) {
        pcs = p;
    });

    QString name;
    st.set_lamda(&PluginProcess::startProgram, [&name](PluginProcess *,const QString &p) {
        name = p;
        return true;
    });

    auto p = pp.m_processes.value("test");
    emit p->stateChanged(QProcess::Running, QProcess::QPrivateSignal());
    QTest::qWaitFor([&called](){return called;}, 30);
    EXPECT_TRUE(called);
    EXPECT_EQ(pcs, p);
    EXPECT_TRUE(name.isEmpty());

    name.clear();
    pcs = nullptr;
    called = false;
    state = QProcess::NotRunning;
    emit p->stateChanged(QProcess::Running, QProcess::QPrivateSignal());
    QTest::qWaitFor([&called](){return called;}, 30);
    EXPECT_TRUE(called);
    EXPECT_EQ(pcs, nullptr);
    EXPECT_TRUE(name.isEmpty());

    name.clear();
    pcs = nullptr;
    called = false;
    pp.m_watch.insert("test", true);
    emit p->stateChanged(QProcess::Running, QProcess::QPrivateSignal());
    QTest::qWaitFor([&called](){return called;}, 30);
    EXPECT_TRUE(called);
    EXPECT_EQ(pcs, nullptr);
    EXPECT_EQ(name, QString("test"));
}

TEST(PluginLiaison, ut_removeChecklist)
{
    PluginProcess pp;
    stub_ext::StubExt st;
    int tid = -1;
    st.set_lamda(&PluginProcess::killTimer, [&tid](QObject *, int id) {
        tid = id;
    });

    QProcess pcs;
    pp.removeChecklist(&pcs);
    EXPECT_EQ(tid, -1);

    tid = -1;
    pp.m_checklist.insert(&pcs, 0);
    pp.removeChecklist(&pcs);
    EXPECT_EQ(tid, -1);
    EXPECT_TRUE(pp.m_checklist.isEmpty());

    tid = -1;
    pp.m_checklist.insert(&pcs, 4396);
    pp.removeChecklist(&pcs);
    EXPECT_EQ(tid, 4396);
    EXPECT_TRUE(pp.m_checklist.isEmpty());
}

TEST(PluginLiaison, ut_addChecklist)
{
    PluginProcess pp;
    QProcess pcs;
    stub_ext::StubExt st;
    QProcess *rp = nullptr;
    st.set_lamda(&PluginProcess::removeChecklist, [&rp](PluginProcess *,QProcess *p) {
        rp = p;
    });

    pp.addChecklist(&pcs);
    auto id = pp.m_checklist.value(&pcs, -1);
    EXPECT_EQ(rp, &pcs);
    ASSERT_GT(id, 0);
}

TEST(PluginLiaison, ut_checkStability)
{
    PluginProcess pp;
    QProcess pcs;
    stub_ext::StubExt st;
    QProcess::ProcessState state = QProcess::NotRunning;
    bool called;
    st.set_lamda(&QProcess::state, [&state, &called]() {
        called = true;
        return state;
    });
    pp.m_restartCount.insert(&pcs, 1);
    pp.checkStability(&pcs);
    EXPECT_EQ(pp.m_restartCount.value(&pcs), 1);

    state = QProcess::Running;
    pp.checkStability(&pcs);
    EXPECT_TRUE(pp.m_restartCount.isEmpty());
}
