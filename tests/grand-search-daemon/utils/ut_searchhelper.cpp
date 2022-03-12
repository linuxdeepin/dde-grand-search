/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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

#include "global/searchhelper.h"

#include <gtest/gtest.h>

TEST(SearchHelperTest, ut_parseKeyword)
{
    QStringList groupList, suffixList, keywordList;
    EXPECT_FALSE(searchHelper->parseKeyword("test", groupList, suffixList, keywordList));
    EXPECT_TRUE(searchHelper->parseKeyword("text::png:b", groupList, suffixList, keywordList));
}

TEST(SearchHelperTest, ut_isSuffix)
{
    EXPECT_FALSE(searchHelper->isSuffix(""));
    EXPECT_TRUE(searchHelper->isSuffix("zip"));
}

TEST(SearchHelperTest, ut_isGroupName)
{
    EXPECT_FALSE(searchHelper->isGroupName(""));
    EXPECT_TRUE(searchHelper->isGroupName("file"));
}

TEST(SearchHelperTest, ut_getSearcherByGroupName)
{
    EXPECT_TRUE(searchHelper->getSearcherByGroupName("test").isEmpty());
    EXPECT_FALSE(searchHelper->getSearcherByGroupName("file").isEmpty());
}

TEST(SearchHelperTest, ut_getSuffixByGroupName)
{
    EXPECT_TRUE(searchHelper->getSuffixByGroupName("test").isEmpty());
    EXPECT_FALSE(searchHelper->getSuffixByGroupName("file").isEmpty());
}

TEST(SearchHelperTest, ut_tropeInputSymbol)
{
    auto str = searchHelper->tropeInputSymbol("test*");
    EXPECT_EQ(str, "test\\*");
}
