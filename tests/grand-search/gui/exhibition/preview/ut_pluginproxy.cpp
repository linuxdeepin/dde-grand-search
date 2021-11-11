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

