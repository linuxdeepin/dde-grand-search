/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhaohanxi<zhaohanxi@uniontech.com>
 *
 * Maintainer: zhaohanxi<zhaohanxi@uniontech.com>
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
#include "gui/searchconfig/searchenginewidget.h"
#include "business/config/searchconfig.h"
#include "global/searchhelper.h"
#include "gui/searchconfig/comboboxwidget/comboboxwidget.h"
#include "global/searchconfigdefine.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace GrandSearch;

TEST(SearchEngineWidgetTest, constructor)
{
    stub_ext::StubExt stu;
    QString choice("");
    stu.set_lamda(&SearchConfig::getConfig, [&](){
        return choice;
    });

    bool language(true);
    stu.set_lamda(&SearchHelper::isSimplifiedChinese, [&](){
        return language;
    });

    SearchEngineWidget *w = new SearchEngineWidget;
    EXPECT_TRUE(w);
    EXPECT_TRUE(w->m_mainLayout);
    EXPECT_TRUE(w->m_groupLabel);
    EXPECT_TRUE(w->m_contentLabel);
    EXPECT_TRUE(w->m_comboboxWidget);

    language = false;
    choice = "Google";

    SearchEngineWidget *w1 = new SearchEngineWidget;
    EXPECT_TRUE(w1);
    EXPECT_TRUE(w1->m_mainLayout);
    EXPECT_TRUE(w1->m_groupLabel);
    EXPECT_TRUE(w1->m_contentLabel);
    EXPECT_TRUE(w1->m_comboboxWidget);

    delete w;
    delete w1;
}

TEST(SearchEngineWidgetTest, checkedChangedIndex)
{
    stub_ext::StubExt stu;

    bool config = false;
    stu.set_lamda(&SearchConfig::setConfig, [&](){
        config = true;
    });

    bool language = true;
    stu.set_lamda(&SearchHelper::isSimplifiedChinese, [&](){
        return language;
    });

    ComboboxWidget *comboboxWidget = new ComboboxWidget;
    stu.set_lamda(&QObject::sender, [&]() {
        return comboboxWidget;
    });

    SearchEngineWidget searchEngineWidget;
    searchEngineWidget.checkedChangedIndex(0);
    EXPECT_TRUE(config);

    config = false;
    language = false;
    searchEngineWidget.checkedChangedIndex(1);
    EXPECT_TRUE(config);

    delete comboboxWidget;
}

TEST(SearchEngineWidgetTest, getIndex)
{
    stub_ext::StubExt stu;

    bool language = true;
    stu.set_lamda(&SearchHelper::isSimplifiedChinese, [&](){
        return language;
    });

    SearchEngineWidget s;
    int result = s.getIndex(GRANDSEARCH_WEB_SEARCHENGINE_GOOGLE);
    EXPECT_EQ(result, 3);

    language = false;
    result = s.getIndex(GRANDSEARCH_WEB_SEARCHENGINE_GOOGLE);
    EXPECT_EQ(result, 0);
}
