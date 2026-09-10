// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "searchplugin/interface/searchplugininterfacev1.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>
#include <QDBusConnection>

GRANDSEARCH_USE_NAMESPACE

TEST(SearchPluginInterfaceV1, ut_constructor)
{
    auto *iface = new SearchPluginInterfaceV1(
        "org.deepin.dde.GrandSearch1",
        "/org/deepin/dde/GrandSearch1",
        "org.deepin.dde.GrandSearch1",
        QDBusConnection::sessionBus(), nullptr);
    EXPECT_NE(iface, nullptr);
    EXPECT_NO_FATAL_FAILURE(delete iface);
}

TEST(SearchPluginInterfaceV1, ut_destructor)
{
    auto *iface = new SearchPluginInterfaceV1(
        "org.deepin.dde.GrandSearch1",
        "/org/deepin/dde/GrandSearch1",
        "org.deepin.dde.GrandSearch1",
        QDBusConnection::sessionBus(), nullptr);
    EXPECT_NO_FATAL_FAILURE(delete iface);
}

TEST(SearchPluginInterfaceV1, ut_Search)
{
    SearchPluginInterfaceV1 iface(
        "org.deepin.dde.GrandSearch1",
        "/org/deepin/dde/GrandSearch1",
        "org.deepin.dde.GrandSearch1",
        QDBusConnection::sessionBus(), nullptr);

    QDBusPendingReply<QString> reply = iface.Search("test");
    // Without a real DBus service the reply will be in error state,
    // but the call itself should not crash.
    EXPECT_NO_FATAL_FAILURE(reply.waitForFinished());
}

TEST(SearchPluginInterfaceV1, ut_Stop)
{
    SearchPluginInterfaceV1 iface(
        "org.deepin.dde.GrandSearch1",
        "/org/deepin/dde/GrandSearch1",
        "org.deepin.dde.GrandSearch1",
        QDBusConnection::sessionBus(), nullptr);

    QDBusPendingReply<bool> reply = iface.Stop("test");
    EXPECT_NO_FATAL_FAILURE(reply.waitForFinished());
}

TEST(SearchPluginInterfaceV1, ut_Action)
{
    SearchPluginInterfaceV1 iface(
        "org.deepin.dde.GrandSearch1",
        "/org/deepin/dde/GrandSearch1",
        "org.deepin.dde.GrandSearch1",
        QDBusConnection::sessionBus(), nullptr);

    QDBusPendingReply<bool> reply = iface.Action("test");
    EXPECT_NO_FATAL_FAILURE(reply.waitForFinished());
}
