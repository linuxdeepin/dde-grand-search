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

#include "gui/exhibition/preview/generalwidget/replicablelabel.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QEvent>
#include <QMouseEvent>
#include <QResizeEvent>

using namespace testing;

TEST(ReplicableLabelTest, constructor)
{
    ReplicableLabel *label = new ReplicableLabel;

    ASSERT_TRUE(label);

    Qt::ContextMenuPolicy policy = label->contextMenuPolicy();
    EXPECT_EQ(policy, Qt::NoContextMenu);

    delete label;
}

TEST(ReplicableLabelTest, setText)
{
    ReplicableLabel label;

    QString test("test");
    label.setText(test);

    QString getVal = label.m_fullText;
    EXPECT_EQ(getVal, test);
}

TEST(ReplicableLabelTest, setElideMode)
{
    ReplicableLabel label;

    Qt::TextElideMode mode = Qt::ElideLeft;

    label.setElideMode(mode);

    Qt::TextElideMode getValue = label.m_elideMode;

    EXPECT_EQ(mode, getValue);
}

TEST(ReplicableLabelTest, elideMode)
{
    ReplicableLabel label;

    Qt::TextElideMode mode = Qt::ElideLeft;

    label.m_elideMode = mode;

    Qt::TextElideMode getValue = label.elideMode();

    EXPECT_EQ(mode, getValue);
}

TEST(ReplicableLabelTest, event)
{
    ReplicableLabel label;

    QEvent event(QEvent::FocusOut);

    label.event(&event);

    EXPECT_EQ(label.m_pressed, false);
}

TEST(ReplicableLabelTest, mousePressEvent)
{
    ReplicableLabel label;

    QMouseEvent event(QEvent::MouseButtonPress, QPointF(), QPointF(), QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier, Qt::MouseEventSynthesizedByQt);

    label.mousePressEvent(&event);

    EXPECT_TRUE(label.m_pressed);
}

TEST(ReplicableLabelTest, mouseReleaseEvent)
{
    ReplicableLabel label;

    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(), QPointF(), QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier, Qt::MouseEventSynthesizedByQt);

    label.mouseReleaseEvent(&event);

    EXPECT_FALSE(label.m_pressed);
}

TEST(ReplicableLabelTest, mouseMoveEvent)
{
    ReplicableLabel label;

    label.m_pressed = true;

    QMouseEvent event(QEvent::MouseMove, QPointF(), QPointF(), QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier, Qt::MouseEventSynthesizedByQt);
    label.mouseMoveEvent(&event);

    EXPECT_FALSE(label.m_pressed);

    QString getVal = label.text();
    EXPECT_EQ(getVal, label.m_fullText);
}

TEST(ReplicableLabelTest, resizeEvent)
{
    ReplicableLabel label;

    QResizeEvent event(QSize(10, 10), QSize(20, 20));

    label.resizeEvent(&event);

    QString getVal = label.text();
    EXPECT_EQ(getVal, label.m_elideText);
}
