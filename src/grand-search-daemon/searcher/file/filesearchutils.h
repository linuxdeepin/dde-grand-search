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

    // 通过搜索内容获取到的搜索信息
    struct SearchInfo
    {
        bool isCombinationSearch = false;   // 是否是组合搜索
        QString keyword;                    // 搜索关键字
        QStringList suffixList;             // 搜索后缀表
        QList<Group> groupList;             // 搜索类目表
    };

    static GrandSearch::MatchedItem packItem(const QString &fileName, const QString &searcher);
    static QString groupKey(Group group);
    static Group getGroupByName(const QString &fileName);
    static Group getGroupBySuffix(const QString &suffix);
    static Group getGroupByGroupName(const QString &groupName);
    static SearchInfo parseContent(const QString &content);
    static bool fileShouldVisible(const QString &fileName, Group &group, const SearchInfo &info);
    static QVariantHash tailerData(const QFileInfo &info);
};

#endif   // FILESEARCHUTILS_H
