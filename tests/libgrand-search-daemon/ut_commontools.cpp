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

#include "global/commontools.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

using namespace GrandSearch::CommonTools;

TEST(CommonToolsTest, fstabBindInfo)
{
    stub_ext::StubExt st;
    st.set_lamda(stat, [](const char *, struct stat *info) {
        info->st_mtime = 1;
        return 0;
    });

    struct fstab fs;
    fs.fs_mntops = "defaults,bind";
    fs.fs_spec = "/data/home";
    fs.fs_file = "/home";
    bool first = true;
    st.set_lamda(getfsent, [&]() -> fstab * {
        if (first) {
            first = false;
            return &fs;
        }

        return nullptr;
    });

    const auto &table = fstabBindInfo();
    EXPECT_TRUE(!table.isEmpty());
    EXPECT_FALSE(first);
}

TEST(CommonToolsTest, bindPathTransform_1)
{
    QString bindPath = bindPathTransform("/", true);
    EXPECT_EQ("/", bindPath);

    stub_ext::StubExt st;
    st.set_lamda(fstabBindInfo, [] { return QMap<QString, QString>(); });
    bindPath = bindPathTransform("/home", true);
    EXPECT_EQ("/home", bindPath);
}

TEST(CommonToolsTest, bindPathTransform_2)
{
    stub_ext::StubExt st;
    st.set_lamda(fstabBindInfo, [] {
        QMap<QString, QString> table { { "/data/home", "/home" } };
        return table;
    });

    auto bindPath = bindPathTransform("/home", true);
    EXPECT_EQ("/data/home", bindPath);

    bindPath = bindPathTransform("/data/home", false);
    EXPECT_EQ("/home", bindPath);
}
