/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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

#include "audio-preview/libaudioviewer.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QLibrary>
#include <QUrl>
#include <QImage>

using namespace GrandSearch;

TEST(LibAudioViewer, ut_initLibrary_load)
{
    LibAudioViewer viewer;

    QLibrary *fake = reinterpret_cast<QLibrary *>(0x1);
    viewer.m_imageLib = fake;

    stub_ext::StubExt stub;
    QString libName;
    bool loaded = false;
    bool retLoad = false;
    stub.set_lamda(&QLibrary::load, [&loaded, &libName, &retLoad](QLibrary *lib){
        loaded = true;
        libName = lib->fileName();
        return retLoad;
    });

    EXPECT_TRUE(viewer.initLibrary());
    EXPECT_FALSE(loaded);
    EXPECT_EQ(viewer.m_imageLib, fake);

    viewer.m_imageLib = nullptr;
    libName.clear();
    loaded = false;
    retLoad = false;
    EXPECT_FALSE(viewer.initLibrary());
    EXPECT_EQ(viewer.m_imageLib, nullptr);
    EXPECT_TRUE(loaded);
    EXPECT_EQ(libName, QString("libimageviewer.so"));
}

static bool movieInfo = false;
static void getMovieInfoFunc(const QUrl &url, QJsonObject *json){
    movieInfo = true;
}

TEST(LibAudioViewer, ut_initLibrary_resolve)
{
    LibAudioViewer viewer;

    stub_ext::StubExt stub;
    bool loaded = false;
    stub.set_lamda(&QLibrary::load, [&loaded](){
        loaded = true;
        return true;
    });

    bool resolve = false;
    stub.set_lamda((QFunctionPointer (QLibrary::*)(const char *))&QLibrary::resolve, [&resolve](QLibrary *self, const char *func)-> QFunctionPointer {
        if (QString(func) == QString("getMovieInfoByJson")) {
            resolve = true;
            return (QFunctionPointer)&getMovieInfoFunc;
        }
        return (QFunctionPointer)nullptr;
    });

    ASSERT_TRUE(viewer.initLibrary());
    EXPECT_NE(viewer.m_imageLib, nullptr);

    EXPECT_TRUE(resolve);
    EXPECT_EQ(viewer.m_getMovieInfoFunc, &getMovieInfoFunc);
}

TEST(LibAudioViewer, ut_getMovieInfo_null)
{
    LibAudioViewer viewer;
    stub_ext::StubExt stub;
    stub.set_lamda(&QLibrary::load, [](){
        return true;
    });

    ASSERT_EQ(viewer.m_getMovieInfoFunc, nullptr);
    QUrl url;
    qint64 duration;
    movieInfo = false;
    EXPECT_FALSE(viewer.getDuration(url, duration));
    EXPECT_FALSE(movieInfo);
}

TEST(LibAudioViewer, ut_getMovieInfo_ok)
{
    LibAudioViewer viewer;
    stub_ext::StubExt stub;
    stub.set_lamda(&QLibrary::load, [](){
        return true;
    });

    stub.set_lamda((QFunctionPointer (QLibrary::*)(const char *))&QLibrary::resolve, [](QLibrary *self, const char *func)-> QFunctionPointer {
        if (QString(func) == QString("getMovieInfoByJson")) {
            return (QFunctionPointer)&getMovieInfoFunc;
        }
        return (QFunctionPointer)nullptr;
    });

    ASSERT_TRUE(viewer.initLibrary());
    ASSERT_EQ(viewer.m_getMovieInfoFunc, &getMovieInfoFunc);

    QUrl url;
    qint64 duration;
    movieInfo = false;
    viewer.getDuration(url, duration);
    EXPECT_TRUE(movieInfo);
}

