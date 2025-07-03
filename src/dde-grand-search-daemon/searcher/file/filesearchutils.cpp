// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filesearchutils.h"
#include "global/builtinsearch.h"
#include "utils/specialtools.h"
#include "global/searchhelper.h"
#include "global/searchconfigdefine.h"
#include "configuration/configer.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

MatchedItem FileSearchUtils::packItem(const QString &fileName, const QString &searcher)
{
    qCDebug(logDaemon) << "Packing file item - File:" << fileName << "Searcher:" << searcher;
    QFileInfo fileInfo(fileName);
    QMimeType mimeType = SpecialTools::getMimeType(fileInfo);
    GrandSearch::MatchedItem item;
    item.item = fileName;
    item.name = fileInfo.fileName();
    item.type = mimeType.name();
    item.icon = mimeType.iconName();
    item.searcher = searcher;
    item.extra = QVariant::fromValue(tailerData(fileInfo));

    qCDebug(logDaemon) << "Item packed successfully - Name:" << item.name
                       << "Type:" << item.type << "Icon:" << item.icon;
    return item;
}

QString FileSearchUtils::groupKey(FileSearchUtils::Group group)
{
    QString key;
    switch (group) {
    case Folder:
        key = GRANDSEARCH_GROUP_FOLDER;
        break;
    case Picture:
        key = GRANDSEARCH_GROUP_FILE_PICTURE;
        break;
    case Audio:
        key = GRANDSEARCH_GROUP_FILE_AUDIO;
        break;
    case Video:
        key = GRANDSEARCH_GROUP_FILE_VIDEO;
        break;
    case Document:
        key = GRANDSEARCH_GROUP_FILE_DOCUMNET;
        break;
    default:
        key = GRANDSEARCH_GROUP_FILE;
        break;
    }

    qCDebug(logDaemon) << "Group key resolved - Group:" << group << "Key:" << key;
    return key;
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
    if (suffix.isEmpty()) {
        qCDebug(logDaemon) << "Empty suffix provided - Returning Unknown group";
        return group;
    }

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

FileSearchUtils::Group FileSearchUtils::getGroupByGroupName(const QString &groupName)
{
    Group group = Unknown;
    if (groupName.compare(FOLDER_GROUP, Qt::CaseInsensitive) == 0) {
        group = Folder;
    } else if (groupName.compare(FILE_GROUP, Qt::CaseInsensitive) == 0) {
        group = File;
    } else if (groupName.compare(VIDEO_GROUP, Qt::CaseInsensitive) == 0) {
        group = Video;
    } else if (groupName.compare(AUDIO_GROUP, Qt::CaseInsensitive) == 0) {
        group = Audio;
    } else if (groupName.compare(PICTURE_GROUP, Qt::CaseInsensitive) == 0) {
        group = Picture;
    } else if (groupName.compare(DOCUMENT_GROUP, Qt::CaseInsensitive) == 0) {
        group = Document;
    }

    qCDebug(logDaemon) << "Group name resolved - Name:" << groupName << "Group:" << group;
    return group;
}

FileSearchUtils::SearchInfo FileSearchUtils::parseContent(const QString &content)
{
    qCDebug(logDaemon) << "Parsing search content - Length:" << content.length();

    SearchInfo info;
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(content.toLocal8Bit(), &error);
    if (error.error != QJsonParseError::NoError || doc.isEmpty()) {
        qCDebug(logDaemon) << "Content is not JSON or parse failed - Treating as simple keyword search";
        info.keyword = content;
        info.boolKeywords = info.keyword.simplified().split(" ");
        qCDebug(logDaemon) << "Simple search parsed - Keyword:" << info.keyword
                           << "Bool keywords count:" << info.boolKeywords.size();
        return info;
    }

    qCDebug(logDaemon) << "JSON content detected - Parsing advanced search parameters";
    QStringList keywordList;
    QJsonObject obj = doc.object();

    // 类目
    QJsonArray groupArr = obj[JSON_GROUP_ATTRIBUTE].toArray();
    qCDebug(logDaemon) << "Processing group attributes - Count:" << groupArr.count();
    for (int i = 0; i < groupArr.count(); ++i) {
        const QString &group = groupArr[i].toString();
        if (group.isEmpty()) {
            qCDebug(logDaemon) << "Skipping empty group at index:" << i;
            continue;
        }

        auto groupType = getGroupByGroupName(group);
        if (groupType != Unknown) {
            info.groupList << groupType;
            qCDebug(logDaemon) << "Added group to search - Group:" << group << "Type:" << groupType;
        } else {
            qCWarning(logDaemon) << "Unknown group type ignored:" << group;
        }
    }

    // 后缀
    QJsonArray suffixArr = obj[JSON_SUFFIX_ATTRIBUTE].toArray();
    qCDebug(logDaemon) << "Processing suffix attributes - Count:" << suffixArr.count();
    for (int i = 0; i < suffixArr.count(); ++i) {
        const QString &suffix = suffixArr[i].toString();
        if (suffix.isEmpty()) {
            qCDebug(logDaemon) << "Skipping empty suffix at index:" << i;
            continue;
        }
        info.suffixList << suffix;
        qCDebug(logDaemon) << "Added suffix to search:" << suffix;
    }

    // 类目/后缀表不为空，说明当前搜索方式为组合搜索
    if (!info.groupList.isEmpty() || !info.suffixList.isEmpty()) {
        info.isCombinationSearch = true;
        qCDebug(logDaemon) << "Combination search detected - Groups:" << info.groupList.size()
                           << "Suffixes:" << info.suffixList.size();
    }

    // 搜索关键字
    QJsonArray keywordArr = obj[JSON_KEYWORD_ATTRIBUTE].toArray();
    qCDebug(logDaemon) << "Processing keyword attributes - Count:" << keywordArr.count();
    for (int i = 0; i < keywordArr.count(); ++i) {
        const QString &key = keywordArr[i].toString();
        if (key.isEmpty()) {
            qCDebug(logDaemon) << "Skipping empty keyword at index:" << i;
            continue;
        }

        keywordList.append(searchHelper->tropeInputSymbol(key));
        qCDebug(logDaemon) << "Added keyword to search:" << key;
    }

    info.keyword = QString(R"((%1).*)").arg(keywordList.join('|'));
    info.typeKeywords = keywordList;
    return info;
}

bool FileSearchUtils::fileShouldVisible(const QString &fileName, Group &group, const FileSearchUtils::SearchInfo &info)
{
    // 对组合搜索到的结果进行过滤
    if (info.isCombinationSearch) {
        if (!info.groupList.contains(group)) {
            QFileInfo fileInfo(fileName);
            if (fileInfo.isDir())
                return false;

            const auto &suffix = fileInfo.suffix();
            if (suffix.isEmpty() || !info.suffixList.contains(suffix, Qt::CaseInsensitive)) {
                if (info.groupList.contains(File)) {
                    group = File;
                    return true;
                }

                return false;
            }
        }
    }

    return true;
}

bool FileSearchUtils::filterByBlacklist(const QString &fileName)
{
    qCDebug(logDaemon) << "Checking blacklist filter for file:" << fileName;

    // 过滤黑名单中的结果
    // add "/" to the end of fileName to distinguish partially identical path
    QString filePath = fileName + "/";
    auto config = Configer::instance()->group(GRANDSEARCH_BLACKLIST_GROUP);
    auto blacklist = config->value(GRANDSEARCH_BLACKLIST_PATH, QStringList());
    if (blacklist.isEmpty()) {
        qCDebug(logDaemon) << "Blacklist is empty - File not filtered:" << fileName;
        return false;
    }

    qCDebug(logDaemon) << "Checking against blacklist - Entries:" << blacklist.size();
    for (const auto &path : blacklist) {
        if (filePath.startsWith(path)) {
            qCDebug(logDaemon) << "File filtered by blacklist - File:" << fileName
                               << "Blacklist path:" << path;
            return true;
        }
    }

    qCDebug(logDaemon) << "File passed blacklist filter:" << fileName;
    return false;
}

QVariantHash FileSearchUtils::tailerData(const QFileInfo &info)
{
    qCDebug(logDaemon) << "Generating tailer data for file:" << info.absoluteFilePath();

    QVariantHash hash;
    QStringList datas;
    auto config = Configer::instance()->group(GRANDSEARCH_TAILER_GROUP);
    if (config->value(GRANDSEARCH_TAILER_PARENTDIR, false)) {
        datas.append(info.absolutePath());
        qCDebug(logDaemon) << "Added parent directory to tailer:" << info.absolutePath();
    }

    if (config->value(GRANDSEARCH_TAILER_TIMEMODEFIED, false)) {
        auto timeModified = info.lastModified().toString("yyyy-MM-dd hh:mm ") + QObject::tr("modified");
        datas.append(timeModified);
        qCDebug(logDaemon) << "Added modification time to tailer:" << timeModified;
    }

    if (!datas.isEmpty()) {
        hash.insert(GRANDSEARCH_PROPERTY_ITEM_TAILER, datas);
        qCDebug(logDaemon) << "Tailer data generated - Entries:" << datas.size();
    } else {
        qCDebug(logDaemon) << "No tailer data configured";
    }

    return hash;
}

QStringList FileSearchUtils::buildDFMSearchFileTypes(const QList<Group> &groupList)
{
    qCDebug(logDaemon) << "Building DFM search file types - Groups:" << groupList.size();

    static const QMap<Group, QString> kGroupToTypeMap = {
        {Group::Folder, "dir"},
        {Group::Picture, "pic"},
        {Group::Audio, "audio"},
        {Group::Video, "video"},
        {Group::Document, "doc"}
    };

    const bool containFile = groupList.contains(Group::File);
    const bool containFolder = groupList.contains(Group::Folder);
    QStringList types;

    if (containFile) {
        qCDebug(logDaemon) << "File group detected - Adding all file types";
        types = kGroupToTypeMap.values();
        types << "app" << "archive" << "other";
        if (!containFolder) {
            types.removeOne(kGroupToTypeMap.value(Group::Folder));
            qCDebug(logDaemon) << "Folder group not included - Removed directory type";
        }
    } else {
        qCDebug(logDaemon) << "Specific groups only - Adding selected types";
        for (const auto &group : groupList) {
            if (kGroupToTypeMap.contains(group))
                types.append(kGroupToTypeMap.value(group));
        }
    }
    qCDebug(logDaemon) << "DFM search file types built - Types:" << types.size() << types;
    return types;
}

bool FileSearchUtils::isPinyin(const QString &str)
{
    if (str.isEmpty()) {
        qCDebug(logDaemon) << "Empty string provided for pinyin check";
        return false;
    }

    static QRegularExpression regex(R"(^[a-zA-Z]+$)");
    return regex.match(str).hasMatch();
}

bool FileSearchUtils::hasWildcard(const QString &str)
{
    return str.contains('*') || str.contains('?');
}

