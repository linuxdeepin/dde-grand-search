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
#include "filesearchutils.h"
#include "global/builtinsearch.h"
#include "utils/specialtools.h"
#include "utils/searchhelper.h"

GrandSearch::MatchedItem FileSearchUtils::packItem(const QString &fileName, const QString &searcher, bool isRecentFile)
{
    QFileInfo fileInfo(fileName);
    QMimeType mimeType = GrandSearch::SpecialTools::getMimeType(fileInfo);
    GrandSearch::MatchedItem item;
    item.item = fileName;
    item.name = fileInfo.fileName();
    item.type = mimeType.name();
    item.icon = mimeType.iconName();
    item.searcher = searcher;

    // 最近使用文件需要置顶显示
    if (isRecentFile) {
        QVariantHash showLevelHash({{GRANDSEARCH_PROPERTY_ITEM_LEVEL, GRANDSEARCH_PROPERTY_ITEM_LEVEL_FIRST}});
        item.extra = QVariant::fromValue(showLevelHash);
    }

    return item;
}

QString FileSearchUtils::groupKey(FileSearchUtils::Group group)
{
    switch (group) {
    case Folder:
        return GRANDSEARCH_GROUP_FOLDER;
    case Picture:
        return GRANDSEARCH_GROUP_FILE_PICTURE;
    case Audio:
        return GRANDSEARCH_GROUP_FILE_AUDIO;
    case Video:
        return GRANDSEARCH_GROUP_FILE_VIDEO;
    case Document:
        return GRANDSEARCH_GROUP_FILE_DOCUMNET;
    default:
        break;
    }
    return GRANDSEARCH_GROUP_FILE;
}

FileSearchUtils::Group FileSearchUtils::getGroupByName(const QString &fileName)
{
    Group group = Unknown;
    QFileInfo fileInfo(fileName);

    if (fileInfo.isDir()) {
        group = Folder;
    } else {
        group = getGroupBySuffix(fileInfo.suffix());
    }

    return group;
}

FileSearchUtils::Group FileSearchUtils::getGroupBySuffix(const QString &suffix)
{
    Group group = Unknown;
    if (suffix.isEmpty())
        return group;

    if (searchHelper->getSuffixByGroupName(DOCUMENT_GROUP).contains(suffix, Qt::CaseInsensitive)) {
        group = Document;
    } else if (searchHelper->getSuffixByGroupName(PICTURE_GROUP).contains(suffix, Qt::CaseInsensitive)) {
        group = Picture;
    } else if (searchHelper->getSuffixByGroupName(VIDEO_GROUP).contains(suffix, Qt::CaseInsensitive)) {
        group = Video;
    } else if (searchHelper->getSuffixByGroupName(AUDIO_GROUP).contains(suffix, Qt::CaseInsensitive)) {
        group = Audio;
    } else if (searchHelper->getSuffixByGroupName(FILE_GROUP).contains(suffix, Qt::CaseInsensitive)) {
        group = File;
    }

    return group;
}

QString FileSearchUtils::buildKeyword(const QString &context, QStringList &suffixContainList, bool &isContainFolder)
{
    suffixContainList.clear();
    isContainFolder = false;
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(context.toLocal8Bit(), &error);
    if (error.error != QJsonParseError::NoError || doc.isEmpty())
        return searchHelper->tropeInputSymbol(context);

    QStringList keywordList;
    QJsonObject obj = doc.object();
    // 类目
    QJsonArray groupArr = obj[JSON_GROUP_ATTRIBUTE].toArray();
    for (int i = 0; i < groupArr.count(); ++i) {
        const QString &group = groupArr[i].toString();
        if (group.isEmpty())
            continue;

        // 文件夹类目
        if (group.compare(FOLDER_GROUP, Qt::CaseInsensitive) == 0) {
            isContainFolder = true;
            continue;
        }

        // 将类目转换为对应的后缀
        const auto &list = searchHelper->getSuffixByGroupName(group);
        if (list.isEmpty())
            continue;
        suffixContainList.append(list);
    }

    // 后缀
    QJsonArray suffixArr = obj[JSON_SUFFIX_ATTRIBUTE].toArray();
    for (int i = 0; i < suffixArr.count(); ++i) {
        const QString &suffix = suffixArr[i].toString();
        if (suffix.isEmpty())
            continue;
        suffixContainList.append(suffix);
    }

    // 搜索关键字
    QJsonArray keywordArr = obj[JSON_KEYWORD_ATTRIBUTE].toArray();
    for (int i = 0; i < keywordArr.count(); ++i) {
        const QString &key = keywordArr[i].toString();
        if (key.isEmpty())
            continue;
        keywordList.append(searchHelper->tropeInputSymbol(key));
    }

    return QString(R"((%1).*)").arg(keywordList.join('|'));
}
