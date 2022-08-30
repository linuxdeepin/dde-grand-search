// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/searchconfig/blacklistview/deletedialog.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;

TEST(DeleteDialogTest, constructor)
{
    DeleteDialog d;
    EXPECT_EQ(d.getButtonIndexByText(QString("Cancel")), 0);
    EXPECT_EQ(d.getButtonIndexByText(QString("Confirm")), 1);
}
