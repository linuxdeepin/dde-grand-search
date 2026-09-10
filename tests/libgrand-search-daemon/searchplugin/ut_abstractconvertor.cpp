// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "searchplugin/convertors/abstractconvertor.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

GRANDSEARCH_USE_NAMESPACE

// Concrete subclass for testing the abstract base
class TestConvertor : public AbstractConvertor
{
public:
    TestConvertor() = default;
    ~TestConvertor() override = default;
};

TEST(AbstractConvertor, ut_constructor)
{
    TestConvertor tc;
    EXPECT_NO_FATAL_FAILURE({});
}

TEST(AbstractConvertor, ut_destructor)
{
    auto *tc = new TestConvertor;
    EXPECT_NO_FATAL_FAILURE(delete tc);
}

TEST(AbstractConvertor, ut_version)
{
    TestConvertor tc;
    EXPECT_EQ(tc.version(), QString(""));
}

TEST(AbstractConvertor, ut_interfaces)
{
    TestConvertor tc;
    auto ifs = tc.interfaces();
    EXPECT_TRUE(ifs.isEmpty());
}
