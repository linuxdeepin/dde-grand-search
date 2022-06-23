/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liqianga@uniontech.com>
 *
 * Maintainer: liuzhangjian<liqianga@uniontech.com>
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

#include "global/grandsearch_global.h"
#include "gui/searchconfig/tailerwidget.h"
#include "business/config/searchconfig.h"
#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
GRANDSEARCH_USE_NAMESPACE

TEST(TailerWidgetTest, onCheckBoxStateChanged)
{
    stub_ext::StubExt st;
    st.set_lamda(&SearchConfig::setConfig, []() { return; });

    TailerWidget w;
    EXPECT_NO_FATAL_FAILURE(w.onCheckBoxStateChanged(0, true));
    EXPECT_NO_FATAL_FAILURE(w.onCheckBoxStateChanged(1, true));
}
