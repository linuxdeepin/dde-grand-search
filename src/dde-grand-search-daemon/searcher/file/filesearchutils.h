// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILESEARCHUTILS_H
#define FILESEARCHUTILS_H

#include "global/matcheditem.h"

namespace GrandSearch {

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
        bool hasScope = false;              // 是否启用 /目录片段 范围限定
        QString keyword;                    // 搜索关键字
        QStringList boolKeywords;           // 空格分词（包含词）
        QStringList typeKeywords;           // 类目搜索分词
        QStringList suffixList;             // 搜索后缀表
        QStringList excludeKeywords;        // 排除词（条目自身名称不得包含）
        QStringList typeFilter;             // 类型过滤扩展名集合（doc:/pic:/ext:xxx 语法解析结果）
        QString folderFilter;               // 目录片段（路径某级目录需包含）
        QList<Group> groupList;             // 搜索类目表
    };

    static MatchedItem packItem(const QString &fileName, const QString &searcher, const QStringList &keywords = QStringList());
    static QString groupKey(Group group);
    static Group getGroupByName(const QString &fileName);
    static Group getGroupBySuffix(const QString &suffix);
    static Group getGroupByGroupName(const QString &groupName);
    static SearchInfo parseContent(const QString &content);
    static QVariantHash extraData(const QFileInfo &info, const QStringList &keywords);
    static bool matchScope(const SearchInfo &info, const QString &filePath);   // /目录片段 范围与排除词后置过滤
    static QStringList buildDFMSearchFileTypes(const QList<Group> &groupList);
    static bool isPinyin(const QString &str);
    static bool hasWildcard(const QString &str);
};

}

#endif   // FILESEARCHUTILS_H
