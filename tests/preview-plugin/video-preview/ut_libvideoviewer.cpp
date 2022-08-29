// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "video-preview/libvideoviewer.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QLibrary>
#include <QUrl>
#include <QImage>

using namespace GrandSearch;

TEST(LibVideoViewer, ut_initLibrary_load)
{
    LibVideoViewer viewer;

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

static bool movieCover = false;
static void getMovieCoverFunc(const QUrl &url, const QString &savePath, QImage *image){
    movieCover = true;
}

static bool movieInfo = false;
static void getMovieInfoFunc(const QUrl &url, QJsonObject *json){
    movieInfo = true;
}

TEST(LibVideoViewer, ut_initLibrary_resolve)
{
    LibVideoViewer viewer;

    stub_ext::StubExt stub;
    bool loaded = false;
    stub.set_lamda(&QLibrary::load, [&loaded](){
        loaded = true;
        return true;
    });

    int resolve = 0;
    stub.set_lamda((QFunctionPointer (QLibrary::*)(const char *))&QLibrary::resolve, [&resolve](QLibrary *self, const char *func)-> QFunctionPointer {
        if (QString(func) == QString("getMovieCover")) {
            resolve = resolve | 1;
            return (QFunctionPointer)&getMovieCoverFunc;
        } else if (QString(func) == QString("getMovieInfoByJson")) {
            resolve = resolve | 2;
            return (QFunctionPointer)&getMovieInfoFunc;
        }
        return (QFunctionPointer)nullptr;
    });

    ASSERT_TRUE(viewer.initLibrary());
    EXPECT_NE(viewer.m_imageLib, nullptr);

    EXPECT_EQ(resolve, 3);
    EXPECT_EQ(viewer.m_getMovieCoverFunc, &getMovieCoverFunc);
    EXPECT_EQ(viewer.m_getMovieInfoFunc, &getMovieInfoFunc);
}

TEST(LibVideoViewer, ut_getMovieInfo_null)
{
    LibVideoViewer viewer;
    stub_ext::StubExt stub;
    stub.set_lamda(&QLibrary::load, [](){
        return true;
    });

    ASSERT_EQ(viewer.m_getMovieInfoFunc, nullptr);
    QUrl url;
    QSize dimension;
    qint64 duration;
    movieInfo = false;
    EXPECT_FALSE(viewer.getMovieInfo(url, dimension, duration));
    EXPECT_FALSE(movieInfo);
}

TEST(LibVideoViewer, ut_getMovieInfo_ok)
{
    LibVideoViewer viewer;
    stub_ext::StubExt stub;
    stub.set_lamda(&QLibrary::load, [](){
        return true;
    });

    stub.set_lamda((QFunctionPointer (QLibrary::*)(const char *))&QLibrary::resolve, [](QLibrary *self, const char *func)-> QFunctionPointer {
        if (QString(func) == QString("getMovieCover")) {
            return (QFunctionPointer)&getMovieCoverFunc;
        } else if (QString(func) == QString("getMovieInfoByJson")) {
            return (QFunctionPointer)&getMovieInfoFunc;
        }
        return (QFunctionPointer)nullptr;
    });

    ASSERT_TRUE(viewer.initLibrary());
    ASSERT_EQ(viewer.m_getMovieInfoFunc, &getMovieInfoFunc);

    QUrl url;
    QSize dimension;
    qint64 duration;
    movieInfo = false;
    viewer.getMovieInfo(url, dimension, duration);
    EXPECT_TRUE(movieInfo);
}

TEST(LibVideoViewer, ut_getMovieCover_null)
{
    LibVideoViewer viewer;
    stub_ext::StubExt stub;
    stub.set_lamda(&QLibrary::load, [](){
        return true;
    });

    ASSERT_EQ(viewer.m_getMovieCoverFunc, nullptr);
    QUrl url;
    QImage img;
    movieCover = false;
    EXPECT_FALSE(viewer.getMovieCover(url, img));
    EXPECT_FALSE(movieCover);
}

TEST(LibVideoViewer, ut_getMovieCover_ok)
{
    LibVideoViewer viewer;
    stub_ext::StubExt stub;
    stub.set_lamda(&QLibrary::load, [](){
        return true;
    });

    stub.set_lamda((QFunctionPointer (QLibrary::*)(const char *))&QLibrary::resolve, [](QLibrary *self, const char *func)-> QFunctionPointer {
        if (QString(func) == QString("getMovieCover")) {
            return (QFunctionPointer)&getMovieCoverFunc;
        } else if (QString(func) == QString("getMovieInfoByJson")) {
            return (QFunctionPointer)&getMovieInfoFunc;
        }
        return (QFunctionPointer)nullptr;
    });

    ASSERT_TRUE(viewer.initLibrary());
    ASSERT_EQ(viewer.m_getMovieCoverFunc, &getMovieCoverFunc);

    QUrl url;
    QImage img;
    movieCover = false;
    EXPECT_TRUE(viewer.getMovieCover(url, img));
    EXPECT_TRUE(movieCover);
}
