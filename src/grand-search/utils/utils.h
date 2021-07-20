/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     houchengqiu<houchengqiu@uniontech.com>
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

#ifndef SORTMATCHITEM_H
#define SORTMATCHITEM_H

#include "global/matcheditem.h"

class Utils
{
public:
    static bool sort(GrandSearch::MatchedItems &list, Qt::SortOrder order = Qt::AscendingOrder);

    static bool compareByString(const QString &str1, const QString &str2, Qt::SortOrder order = Qt::AscendingOrder);
    static bool startWithSymbol(const QString &text);
    static bool startWithHanzi(const QString &text);
    static bool startWithLatin(const QString &text);
    static bool startWithOtherLang(const QString &text);
};

#endif // SORTMATCHITEM_H
