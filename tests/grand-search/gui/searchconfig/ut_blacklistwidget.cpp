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
#include "global/grandsearch_global.h"
#include "gui/searchconfig/blacklistwidget.h"
#include "gui/searchconfig/blacklistview/blacklistview.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <DDialog>

#include <QFileDialog>
#include <QModelIndexList>
#include <QStandardPaths>

using namespace testing;
DWIDGET_USE_NAMESPACE
GRANDSEARCH_USE_NAMESPACE

TEST(BlackListWidgetTest, constructor)
{
    BlackListWidget *blackListWidget = new BlackListWidget();

    EXPECT_TRUE(blackListWidget->m_mainLayout);
    EXPECT_TRUE(blackListWidget->m_childHLayout);
    EXPECT_TRUE(blackListWidget->m_groupLabel);
    EXPECT_TRUE(blackListWidget->m_contentLabel);
    EXPECT_TRUE(blackListWidget->m_addButton);
    EXPECT_TRUE(blackListWidget->m_deleteButton);
    EXPECT_TRUE(blackListWidget->m_listWrapper);

    delete blackListWidget;
}

TEST(BlackListWidgetTest, addButtonClicked)
{
    BlackListWidget w;
    stub_ext::StubExt stu;
    QUrl url("/");
    stu.set_lamda(&QFileDialog::getExistingDirectoryUrl, [&](){
        return url;
    });

    stu.set_lamda(&QUrl::toLocalFile, [](){
        return QString("/home");
    });
    bool flag = false;
    stu.set_lamda(&BlackListWrapper::addRow, [&](){
        flag = true;
    });
    w.addButtonClicked();
    EXPECT_TRUE(flag);

    flag = false;
    url.clear();
    w.addButtonClicked();
    EXPECT_FALSE(flag);
}

TEST(BlackListWidgetTest, deleteButtonClicked)
{
    BlackListWidget w;

    stub_ext::StubExt stu;
    auto dialog = QDialog::Accepted;
    stu.set_lamda(VADDR(DDialog, exec), [&](){
        return dialog;
    });

    stu.set_lamda(&QItemSelectionModel::selectedRows, [](){
        QModelIndexList list;
        QStandardItemModel model;
        model.insertRows(0, 3, QModelIndex());
        list << model.index(1, 0, QModelIndex()) << model.index(3, 0, QModelIndex()) << model.index(2, 0, QModelIndex());
        return list;
    });

    bool ut_call_removeRows = false;
    stu.set_lamda(&BlackListWrapper::removeRows, [&](){
        ut_call_removeRows = true;
    });

    w.deleteButtonClicked();
    EXPECT_TRUE(ut_call_removeRows);

    ut_call_removeRows = false;
    dialog = QDialog::Rejected;
    w.deleteButtonClicked();
    EXPECT_FALSE(ut_call_removeRows);
}

TEST(BlackListWidgetTest, selectedChanged)
{
    stub_ext::StubExt stu;
    bool ut_call_setEnabled = false;
    stu.set_lamda(&DIconButton::setEnabled, [&](){
        ut_call_setEnabled = true;
    });

    BlackListWidget w;
    w.selectedChanged(QItemSelection(), QItemSelection());
    EXPECT_TRUE(ut_call_setEnabled);
}
