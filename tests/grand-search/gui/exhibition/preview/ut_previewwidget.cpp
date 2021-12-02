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
#include "gui/exhibition/preview/generalpreviewplugin.h"
#include "gui/exhibition/preview/previewpluginmanager.h"
#include "utils/utils.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QClipboard>

using namespace testing;
using namespace GrandSearch;

TEST(PreviewWidgetTest, constructor)
{
    PreviewWidget *w = new PreviewWidget;

    ASSERT_TRUE(w);
    ASSERT_TRUE(w->m_generalPreview);
    ASSERT_TRUE(w->m_detailInfoWidget);
    ASSERT_TRUE(w->m_generalToolBar);

    delete w;
}

TEST(PreviewWidgetTest, previewItem)
{
    PreviewWidget w;

    stub_ext::StubExt stu;

    PreviewPlugin *ut_previewPlugin = nullptr;
    stu.set_lamda((PreviewPlugin*(PreviewPluginManager::*)(const MatchedItem &))ADDR(PreviewPluginManager, getPreviewPlugin), [&](){
        return ut_previewPlugin;
    });

    w.m_preview = nullptr;

    MatchedItem item;
    bool result = w.previewItem(item);
    EXPECT_TRUE(result);
}

TEST(PreviewWidgetTest, clearLayoutWidgets)
{
    PreviewWidget w;

    w.m_preview = w.m_generalPreview;

    stub_ext::StubExt stu;

    typedef QWidget *(*fptr)(GeneralPreviewPlugin*);
    fptr ut_contentWidget = (fptr)(&GeneralPreviewPlugin::contentWidget);

    QWidget *contentWidget = new QWidget;
    stu.set_lamda(ut_contentWidget, [&](){
        return contentWidget;
    });

    fptr ut_toolBarWidget = (fptr)(&GeneralPreviewPlugin::toolBarWidget);

    QWidget *toolBarWidget = new QWidget;
    stu.set_lamda(ut_toolBarWidget, [&](){
        return toolBarWidget;
    });

    w.clearLayoutWidgets();

    EXPECT_EQ(contentWidget->parent(), nullptr);
    EXPECT_EQ(toolBarWidget->parent(), nullptr);

    delete contentWidget;
    delete toolBarWidget;
}

TEST(PreviewWidgetTest, onOpenClicked)
{
    PreviewWidget w;

    stub_ext::StubExt stu;

    bool ut_call_openFile = false;
    stu.set_lamda(ADDR(Utils, openFile), [&](){
        ut_call_openFile = true;
        return ut_call_openFile;
    });

    w.onOpenClicked();
    EXPECT_TRUE(ut_call_openFile);
}

TEST(PreviewWidgetTest, onOpenpathClicked)
{
    PreviewWidget w;

    stub_ext::StubExt stu;

    bool ut_call_openFile = false;
    stu.set_lamda(ADDR(Utils, openInFileManager), [&](){
        ut_call_openFile = true;
        return ut_call_openFile;
    });

    w.onOpenpathClicked();
    EXPECT_TRUE(ut_call_openFile);
}

TEST(PreviewWidgetTest, onCopypathClicked)
{
    PreviewWidget w;

    stub_ext::StubExt stu;

    bool ut_call_setText = false;
    stu.set_lamda(ADDR(QClipboard, setText), [&](){
        ut_call_setText = true;
    });

    w.onCopypathClicked();
    EXPECT_TRUE(ut_call_setText);
}
