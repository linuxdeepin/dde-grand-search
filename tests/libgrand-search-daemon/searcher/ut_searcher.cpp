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

#include "global/grandsearch_global.h"
#include "searcher/searcher.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

GRANDSEARCH_USE_NAMESPACE

class TestSearcher : public Searcher
{
public:
    TestSearcher(QObject *parent = nullptr) : Searcher(parent) {}
    QString name() const override { return ""; }
    bool isActive() const override { return false; }
    ProxyWorker *createWorker() const override { return nullptr; }
    bool action (const QString &, const QString &) override { return false; }
};

TEST(SearcherTest, ut_activate)
{
    TestSearcher ts;
    EXPECT_FALSE(ts.activate());
}
