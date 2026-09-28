// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "configuration/userpreference.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

GRANDSEARCH_USE_NAMESPACE

TEST(UserPreference, ut_constructor)
{
    QVariantHash datas;
    datas.insert("0", true);
    datas.insert("1", 1);
    UserPreference up(datas);

    EXPECT_EQ(up.group("2"), nullptr);

    {
        QVariant var;
        EXPECT_FALSE(up.innerValue("3", var));
        EXPECT_TRUE(up.innerValue("0", var));
        EXPECT_TRUE(var.toBool());

        EXPECT_TRUE(up.innerValue("1", var));
        EXPECT_EQ(var.toInt(), 1);
    }

    {
        EXPECT_FALSE(up.value("4", false));
        EXPECT_EQ(up.value("4", 999), 999);

        EXPECT_TRUE(up.value("0", false));
        EXPECT_EQ(up.value("1", 999), 1);
    }
}

TEST(UserPreference, ut_setValue)
{
    UserPreference up({});

    // setValue with empty name should be a no-op
    up.setValue("", 42);
    QVariant var;
    EXPECT_FALSE(up.innerValue("", var));

    // Overwrite existing key
    up.setValue("key1", true);
    EXPECT_TRUE(up.value("key1", false));
    up.setValue("key1", false);
    EXPECT_FALSE(up.value("key1", true));

    // Store a UserPreferencePointer
    UserPreferencePointer upp(new UserPreference({}));
    up.setValue("child", QVariant::fromValue(upp));
    EXPECT_EQ(up.group("child").get(), upp.get());
}

TEST(UserPreference, ut_group)
{
    UserPreference up({});
    EXPECT_EQ(up.group("nonexistent"), nullptr);

    UserPreferencePointer child(new UserPreference({}));
    up.setValue("child", QVariant::fromValue(child));
    EXPECT_EQ(up.group("child").get(), child.get());
    EXPECT_EQ(up.group("other"), nullptr);
}

TEST(UserPreference, ut_innerValue)
{
    QVariantHash datas;
    datas.insert("str", QString("hello"));
    datas.insert("int", 42);
    UserPreference up(datas);

    QVariant var;
    EXPECT_TRUE(up.innerValue("str", var));
    EXPECT_EQ(var.toString(), QString("hello"));

    EXPECT_TRUE(up.innerValue("int", var));
    EXPECT_EQ(var.toInt(), 42);

    EXPECT_FALSE(up.innerValue("missing", var));
}

TEST(UserPreference, ut_value)
{
    UserPreference up({{"flag", true}, {"count", 7}});

    EXPECT_TRUE(up.value("flag", false));
    EXPECT_EQ(up.value("count", 0), 7);

    // Missing key returns default
    EXPECT_FALSE(up.value("missing", false));
    EXPECT_EQ(up.value("missing", 999), 999);
}
