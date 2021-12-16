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

#include "searchplugin/pluginliaison_p.h"
#include "searchplugin/convertors/dataconvertor.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

// 因为#include <QTest>, qApp会变成static_cast<QGuiApplication *>(QCoreApplication::instance()
// 而main函数中定义的是QCoreApplication，此转换会导致报内存泄露，这里将qApp重新定义回QCoreApplication::instance()
#define qApp QCoreApplication::instance()

TEST(PluginLiaison, ut_init)
{
    stub_ext::StubExt st;
    bool sup = true;
    st.set_lamda(&DataConvertor::isSupported, [&sup]() {
        return sup;
    });

    PluginLiaison pl;
    ASSERT_EQ(pl.d->m_inteface, nullptr);

    EXPECT_FALSE(pl.init("","1","2","3","4"));
    ASSERT_EQ(pl.d->m_inteface, nullptr);

    EXPECT_FALSE(pl.init("1","","2","3","4"));
    ASSERT_EQ(pl.d->m_inteface, nullptr);

    EXPECT_FALSE(pl.init("1","2","","3","4"));
    ASSERT_EQ(pl.d->m_inteface, nullptr);

    EXPECT_FALSE(pl.init("1","2","3","","4"));
    ASSERT_EQ(pl.d->m_inteface, nullptr);

    EXPECT_FALSE(pl.init("1","2","3","4",""));
    ASSERT_EQ(pl.d->m_inteface, nullptr);

    //版本不支持
    sup = false;
    EXPECT_FALSE(pl.init("1","2","3","4","5"));
    ASSERT_EQ(pl.d->m_inteface, nullptr);

    sup = true;
    EXPECT_TRUE(pl.init("1","2","3","4","5"));
    EXPECT_NE(pl.d->m_inteface, nullptr);

    // 重复初始化
    EXPECT_FALSE(pl.init("1","2","3","4","5"));
}

TEST(PluginLiaison, ut_isVaild)
{
    stub_ext::StubExt st;
    bool sup = true;
    st.set_lamda(&QDBusAbstractInterface::isValid, [&sup]() {
        return sup;
    });

    PluginLiaison pl;
    EXPECT_FALSE(pl.isVaild());

    pl.d->m_inteface = new SearchPluginInterfaceV1("service", "address", "interface",
                                                QDBusConnection::sessionBus(), &pl);
    pl.d->m_ver.clear();
    EXPECT_FALSE(pl.isVaild());

    pl.d->m_ver = "1.0";
    EXPECT_TRUE(pl.isVaild());

    sup = false;
    EXPECT_FALSE(pl.isVaild());
}

TEST(PluginLiaison, ut_search)
{
    stub_ext::StubExt st;
    bool search = false;
    st.set_lamda(&SearchPluginInterfaceV1::Search, [&search]() {
        search = true;
        QDBusMessage msg = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/org/freedesktop/DBus",
                                                          "org.freedesktop.DBus.Properties", "Get");
        auto var = QVariant::fromValue(QString("test"));
        auto reply = msg.createReply(var);
        auto ret = QDBusPendingReply<QString>(reply);
        return ret;
    });

    st.set_lamda(&DataConvertor::convert, [](DataConvertor *,
                 const QString &version, const QString &type, void *in, void *out) {
        if (version != "test")
            return -1;
        EXPECT_EQ(type, PLUGININTERFACE_TYPE_SEARCH);
        return 0;
    });

    PluginLiaison pl;
    pl.d->m_inteface = new SearchPluginInterfaceV1("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus.Properties",
                                                QDBusConnection::sessionBus(), &pl);
    pl.d->m_inteface ->setTimeout(10);
    pl.d->m_ver = "1.0";
    EXPECT_FALSE(pl.search("task", "test"));
    EXPECT_FALSE(pl.d->m_searching.loadAcquire());
    EXPECT_FALSE(search);

    search = false;
    pl.d->m_ver = "test";
    EXPECT_TRUE(pl.search("task", "test"));
    EXPECT_TRUE(search);
    EXPECT_TRUE(pl.d->m_searching.load());
    ASSERT_NE(pl.d->m_replyWatcher, nullptr);

    EXPECT_EQ(pl.d->m_replyWatcher->thread(), qApp->thread());

    //todo dbus mmleak
    QTest::qWaitFor([&](){
        return pl.d->m_replyWatcher->isFinished();
    }, 50);

    EXPECT_TRUE(pl.d->m_replyWatcher->isError());

    search = false;
    EXPECT_FALSE(pl.search("task2", "test2"));
    EXPECT_FALSE(search);
}

TEST(PluginLiaison, ut_stop)
{
    stub_ext::StubExt st;
    bool stop = false;
    st.set_lamda(&SearchPluginInterfaceV1::Stop, [&stop]() {
        stop = true;
        return QDBusPendingReply<bool>(QDBusMessage::createError(QDBusError::NoReply, ""));
    });

    st.set_lamda(&DataConvertor::convert, [](DataConvertor *,
                 const QString &version, const QString &type, void *in, void *out) {
        if (version != "test")
            return -1;
        EXPECT_EQ(type, PLUGININTERFACE_TYPE_STOP);
        return 0;
    });

    PluginLiaison pl;
    pl.d->m_inteface = new SearchPluginInterfaceV1("service", "address", "interface",
                                                QDBusConnection::sessionBus(), &pl);
    pl.d->m_searching.store(false);

    EXPECT_TRUE(pl.stop("test"));

    pl.d->m_searching.store(true);
    EXPECT_TRUE(pl.stop(""));
    EXPECT_FALSE(pl.d->m_searching.load());

    pl.d->m_searching.store(true);

    pl.d->m_ver = "1.0";
    EXPECT_FALSE(pl.stop("task"));
    EXPECT_FALSE(pl.d->m_searching.loadAcquire());
    EXPECT_FALSE(stop);

    pl.d->m_ver = "test";
    stop = false;
    pl.d->m_searching.store(true);
    EXPECT_TRUE(pl.stop("task"));
    EXPECT_FALSE(pl.d->m_searching.loadAcquire());
    EXPECT_TRUE(stop);
}

TEST(PluginLiaison, ut_action)
{
    stub_ext::StubExt st;
    bool action = false;
    st.set_lamda(&SearchPluginInterfaceV1::Action, [&action]() {
        action = true;
        return QDBusPendingReply<bool>(QDBusMessage::createError(QDBusError::NoReply, ""));
    });

    st.set_lamda(&DataConvertor::convert, [](DataConvertor *,
                 const QString &version, const QString &type, void *in, void *out) {
        if (version != "test")
            return -1;
        EXPECT_EQ(type, PLUGININTERFACE_TYPE_ACTION);
        return 0;
    });

    PluginLiaison pl;
    EXPECT_FALSE(pl.action("", ""));
    EXPECT_FALSE(pl.action("test", ""));
    EXPECT_FALSE(pl.action("test", "test"));
    pl.d->m_inteface = new SearchPluginInterfaceV1("service", "address", "interface",
                                                QDBusConnection::sessionBus(), &pl);

    pl.d->m_ver = "1.0";
    EXPECT_FALSE(pl.action("test", "test"));
    EXPECT_FALSE(action);

    action = false;
    pl.d->m_ver = "test";
    EXPECT_TRUE(pl.action("test", "test"));
    EXPECT_TRUE(action);
}

TEST(PluginLiaison, ut_onServiceStarted)
{
    PluginLiaison pl;
    EXPECT_NO_FATAL_FAILURE(pl.d->onServiceStarted("demo", "", ""));

    pl.d->m_inteface = new SearchPluginInterfaceV1("service", "address", "interface",
                                                QDBusConnection::sessionBus(), &pl);
    bool registered = true;
    stub_ext::StubExt st;
    st.set_lamda(&QDBusConnectionInterface::isServiceRegistered, [&registered]()->QDBusReply<bool> {
        QDBusMessage msg;
        return msg.createReply(QVariant::fromValue(registered));
    });

    bool ready = false;
    pl.connect(&pl, &PluginLiaison::ready, &pl, [&ready](){ ready = true;}, Qt::DirectConnection);

    pl.d->onServiceStarted("demo", "", "");
    EXPECT_FALSE(ready);

    ready = false;
    registered = false;
    pl.d->onServiceStarted("service", "", "");
    EXPECT_FALSE(ready);

    ready = false;
    registered = true;
    pl.d->onServiceStarted("service", "", "");
    EXPECT_TRUE(ready);
}
