/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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

#include "gui/exhibition/preview/previewwidget.h"
#include "gui/exhibition/preview/pluginproxy.h"
#include "gui/exhibition/preview/generalwidget/detailwidget.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace GrandSearch;

TEST(PluginProxyTest, constructor)
{
    PreviewWidget w;
    PluginProxy *p = new PluginProxy(&w);

    ASSERT_TRUE(p);
}

TEST(PluginProxyTest, updateDetailInfo)
{
    PreviewWidget w;
    PluginProxy *p = new PluginProxy(&w);

    stub_ext::StubExt stu;

    bool ut_call_set = false;
    stu.set_lamda(&DetailWidget::setDetailInfoList, [&](){
       ut_call_set = true;
    });

    p->updateDetailInfo(nullptr);
    EXPECT_FALSE(ut_call_set);

    ut_call_set = false;
    w.m_preview = w.m_generalPreview;
    p->updateDetailInfo(w.m_generalPreview.get());
    EXPECT_TRUE(ut_call_set);

    QThread threadA;
    QThread threadB;
    stu.set_lamda(ADDR(QObject, thread), [&](QObject *obj){
        if (obj == qApp)
            return &threadA;
        return &threadB;
    });

    ut_call_set = false;
    p->updateDetailInfo(w.m_generalPreview.get());
    EXPECT_FALSE(ut_call_set);
}
