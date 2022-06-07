/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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

#include "utils/chineseletterhelper.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>
#include <QFile>

using namespace GrandSearch;
TEST(ChineseLetterHelperTest, ut_convertChinese2Pinyin)
{
    stub_ext::StubExt st;
    typedef bool (*fptr)(QFile *, QIODevice::OpenMode);
    fptr open_stub = (fptr)((bool (QFile::*)(QIODevice::OpenMode)) & QFile::open);
    st.set_lamda(open_stub, []() { return true; });
    st.set_lamda(&QFile::readAll, []() {
        return "0x6d4b:ce\n0x8bd5:shi";
    });

    QString firstPy;
    QString fullPy;
    Ch2PyIns->convertChinese2Pinyin("测试", firstPy, fullPy);
    EXPECT_EQ(firstPy, QString("cs"));
    EXPECT_EQ(fullPy, QString("ceshi"));
}
