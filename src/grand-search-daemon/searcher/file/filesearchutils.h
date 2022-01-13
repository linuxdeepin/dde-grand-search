/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef FILESEARCHUTILS_H
#define FILESEARCHUTILS_H

#include "global/matcheditem.h"

class FileSearchUtils
{
public:
    enum Group {
        Unknown = 0,
        File,
        Folder,
        Picture,
        Audio,
        Video,
        Document,
        GroupCount,
        GroupBegin = Unknown
    };

    enum SearchType {
        NormalSearch,   // 普通搜索
        SuffixSearch,   // 后缀搜索
        GrouopSearch    // 类目搜索
    };

    static GrandSearch::MatchedItem packItem(const QString &fileName, const QString &searcher, bool isRecentFile = false);
    static QString groupKey(Group group);
    static Group getGroupByName(const QString &fileName);
    static Group getGroupBySuffix(const QString &suffix);
    static SearchType checkSearchTypeAndToReg(QString &pattern);
private:
    static QString toSuffixReg(const QString &suffix, const QString &pattern);
    static QString toGroupReg(const QString &group, const QString &pattern);
};

#endif   // FILESEARCHUTILS_H
