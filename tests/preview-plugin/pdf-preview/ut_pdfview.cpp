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
#include "pdf-preview/pdfview.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>
#include <QLabel>

class PDFViewTest : public testing::Test
{
public:
    PDFViewTest()
        : testing::Test()
    {
        stub_ext::StubExt st;
        st.set_lamda(&PDFView::initDoc, [](){ return; });
        st.set_lamda(&PDFView::initUI, [](){ return; });
        st.set_lamda(&PDFView::initConnections, [](){ return; });

        view.reset(new PDFView("test.pdf"));
    }

    QSharedPointer<PDFView> view;
};

TEST_F(PDFViewTest, ut_initDoc)
{
    stub_ext::StubExt st;
    st.set_lamda(Poppler::Document::load, [](){ return nullptr; });
    EXPECT_NO_FATAL_FAILURE(view->initDoc("test.pdf"));
}

TEST_F(PDFViewTest, ut_initUI_0)
{
    stub_ext::StubExt st;
    st.set_lamda(&PDFView::syncLoadFirstPage, [](){ return; });

    EXPECT_NO_FATAL_FAILURE(view->initUI());
}

TEST_F(PDFViewTest, ut_initUI_1)
{
    view->m_isBadDoc = true;
    EXPECT_NO_FATAL_FAILURE(view->initUI());
}

TEST_F(PDFViewTest, ut_initConnections)
{
    EXPECT_NO_FATAL_FAILURE(view->initConnections());
}

TEST_F(PDFViewTest, ut_onPageUpdated)
{
    view->m_pageLabel = new QLabel(view.data());
    QImage errImg(":/icons/file_damaged.svg");
    EXPECT_NO_FATAL_FAILURE(view->onPageUpdated(errImg));
}

TEST_F(PDFViewTest, ut_syncLoadFirstPage)
{
    stub_ext::StubExt st;
    Poppler::Page *(Poppler::Document::*page_addr)(int) const = &Poppler::Document::page;
    st.set_lamda(page_addr, [](){ return nullptr; });

    view->m_doc.reset(new Poppler::Document(nullptr));
    view->syncLoadFirstPage();
    view->m_future.waitForFinished();
    EXPECT_TRUE(view->m_isLoadFinished);
}
