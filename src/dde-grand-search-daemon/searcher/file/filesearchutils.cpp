// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filesearchutils.h"
#include "global/builtinsearch.h"
#include "global/commontools.h"
#include "utils/specialtools.h"
#include "global/searchhelper.h"
#include "global/searchconfigdefine.h"
#include "configuration/configer.h"

#include <dfm-search/dsearch_global.h>

#include <DDesktopEntry>

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;
DCORE_USE_NAMESPACE

MatchedItem FileSearchUtils::packItem(const QString &fileName, const QString &searcher, const QStringList &keywords)
{
    qCDebug(logDaemon) << "Packing file item - File:" << fileName << "Searcher:" << searcher;
    QFileInfo fileInfo(fileName);
    GrandSearch::MatchedItem item;
    item.item = fileName;
    item.searcher = searcher;
    item.extra = QVariant::fromValue(extraData(fileInfo, keywords));

    // .desktop 文件（含符号链接指向 .desktop）：解析出本地化应用名称和应用图标
    QString desktopPath = fileName;
    if (fileInfo.suffix().compare("desktop", Qt::CaseInsensitive) != 0 && fileInfo.isSymLink()) {
        QString target = fileInfo.symLinkTarget();
        if (QFileInfo(target).suffix().compare("desktop", Qt::CaseInsensitive) == 0)
            desktopPath = target;
    }

    if (desktopPath != fileName || fileInfo.suffix().compare("desktop", Qt::CaseInsensitive) == 0) {
        DDesktopEntry desktopEntry(desktopPath);
        QString displayName = desktopEntry.ddeDisplayName();
        if (!displayName.isEmpty()) {
            item.name = displayName;
            item.icon = desktopEntry.stringValue("Icon", "Desktop Entry", "application-x-desktop");
            item.type = "application/x-desktop";

            qCDebug(logDaemon) << "Desktop file parsed - Name:" << item.name
                               << "Type:" << item.type << "Icon:" << item.icon;
            return item;
        }
    }

    // 普通文件：使用 MIME 类型信息
    QMimeType mimeType = SpecialTools::getMimeType(fileInfo);
    item.name = fileInfo.fileName();
    item.type = mimeType.name();
    item.icon = mimeType.iconName();

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
        // 文档独立分组（docx/txt/md/pdf 等归「文档」类，普通文件归「文件」类）
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

// 常用文件类型过滤宏（参照 Listary）：doc: / pic: / video: / audio: / zip: / exe: / pdf:
// 返回宏对应的扩展名集合（小写、不含前导点）
static QStringList typeFilterExtensions(const QString &name)
{
    const QString lower = name.toLower();
    if (lower == "doc")
        return { "docx", "doc", "wps", "wpt", "uot", "uos", "uop", "uof", "uott", "uost", "uopt", "odt",
                 "rtf", "pdf", "txt", "md", "xls", "xlsx", "ppt", "pptx", "dps", "et", "ett", "dpt", "ofd" };
    if (lower == "pic")
        return { "jpg", "jpeg", "png", "gif", "bmp", "webp", "svg", "tif", "tiff", "ico", "heic" };
    if (lower == "video")
        return { "mp4", "mkv", "avi", "mov", "wmv", "flv", "webm", "m4v", "ts", "mpg", "mpeg", "rmvb" };
    if (lower == "audio")
        return { "mp3", "wav", "flac", "aac", "ogg", "m4a", "wma", "ape", "opus" };
    if (lower == "zip")
        return { "zip", "rar", "7z", "tar", "gz", "bz2", "xz", "zst", "iso" };
    if (lower == "exe")
        return { "exe", "msi", "appimage", "deb", "rpm" };
    if (lower == "pdf")
        return { "pdf" };
    return {};
}

// 识别类型过滤 token（doc: / pic: / ext:.mp4 等）：命中返回 true 并把扩展名填入 extensions
// 仅识别半角冒号 ':'（类型筛选统一用英文冒号）
static bool resolveTypeFilterToken(const QString &token, QStringList &extensions)
{
    const int colon = token.indexOf(QLatin1Char(':'));
    if (colon <= 0)
        return false;
    const QString name = token.left(colon).trimmed().toLower();
    if (name == "ext") {
        // ext:.mp4 / ext:mp4,txt / ext:.mp4,.avi —— 冒号后为扩展名列表（可带点、逗号分隔）
        const QStringList parts = token.mid(colon + 1).split(',', Qt::SkipEmptyParts);
        for (QString p : parts) {
            QString e = p.trimmed().toLower();
            if (e.startsWith('.'))
                e = e.mid(1);
            if (!e.isEmpty() && !extensions.contains(e))
                extensions << e;
        }
        return !extensions.isEmpty();
    }
    // 非 ext 类型：token 必须恰好是 “类型名+冒号”（doc: / pic: / video: ...）
    if (token.size() != colon + 1)
        return false;
    const QStringList exts = typeFilterExtensions(name);
    if (exts.isEmpty())
        return false;
    for (const QString &e : exts)
        if (!extensions.contains(e))
            extensions << e;
    return true;
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
        // 归一化全角空格后统一处理（content 为 const 引用，先复制再原地替换）
        QString normalized = content;
        normalized.replace(QChar(0x3000), QLatin1Char(' '));
        normalized = normalized.simplified();

        // 目录片段范围限定语法：/目录片段 包含词1 包含词2 -排除词1 -排除词2
        if (normalized.startsWith('/')) {
            const QString rest = normalized.mid(1);
            const int space = rest.indexOf(' ');
            if (space > 0) {
                // 目录片段 = 第一个空格前的文本；剩余按空格分词，区分包含词、排除词与类型过滤词
                info.folderFilter = rest.left(space);
                const QStringList tokens = rest.mid(space + 1).simplified().split(' ', Qt::SkipEmptyParts);
                QStringList includeKw;
                for (const QString &t : tokens) {
                    if (t.startsWith('-') && t.size() > 1)
                        info.excludeKeywords << t.mid(1);
                    else if (resolveTypeFilterToken(t, info.typeFilter))
                        continue;
                    else
                        includeKw << t;
                }
                info.boolKeywords = includeKw;
                info.hasScope = true;
            } else if (space < 0) {
                // 仅目录片段，无关键词："/工作总结"
                info.folderFilter = rest.simplified();
                info.hasScope = !info.folderFilter.isEmpty();
            } else {
                // "/ 关键词" —— 目录片段为空，回退普通多关键字搜索
                info.boolKeywords = normalized.mid(1).simplified().split(' ', Qt::SkipEmptyParts);
            }
        } else {
            // 普通多关键字搜索同样支持排除词与类型过滤：-开头的词为排除词，
            // xxx: 结尾的词为类型过滤（doc:/pic:/video:/audio:/zip:/exe:/pdf:/ext:自定义），其余为包含词（AND）
            const QStringList tokens = normalized.split(' ', Qt::SkipEmptyParts);
            QStringList includeKw;
            for (const QString &t : tokens) {
                if (t.startsWith('-') && t.size() > 1)
                    info.excludeKeywords << t.mid(1);
                else if (resolveTypeFilterToken(t, info.typeFilter))
                    continue;
                else
                    includeKw << t;
            }
            info.boolKeywords = includeKw;
        }
        qCDebug(logDaemon) << "Simple search parsed - Keyword:" << info.keyword
                           << "Bool keywords:" << info.boolKeywords.size()
                           << "Scope:" << info.hasScope << info.folderFilter
                           << "Excludes:" << info.excludeKeywords.size()
                           << "TypeFilter:" << info.typeFilter.size();
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

        keywordList.append(key);
        qCDebug(logDaemon) << "Added keyword to search:" << key;
    }

    info.keyword = content;
    info.typeKeywords = keywordList;
    return info;
}

bool FileSearchUtils::matchScope(const SearchInfo &info, const QString &filePath)
{
    const QStringList parts = filePath.split('/', Qt::SkipEmptyParts);

    // 1) 目录片段：仅在启用 /目录片段 范围限定时检查，路径的某一级上级目录（非条目自身）需包含该片段
    if (info.hasScope && !info.folderFilter.isEmpty()) {
        bool matched = false;
        for (int i = 0; i + 1 < parts.size(); ++i) {
            if (parts.at(i).contains(info.folderFilter, Qt::CaseInsensitive)) {
                matched = true;
                break;
            }
        }
        if (!matched)
            return false;
    }

    // 2) 排除词：条目自身名称（basename）不得包含任一排除词
    if (!info.excludeKeywords.isEmpty()) {
        const QString basename = parts.isEmpty() ? filePath : parts.last();
        for (const QString &ex : info.excludeKeywords) {
            if (basename.contains(ex, Qt::CaseInsensitive))
                return false;
        }
    }

    // 3) 类型过滤（doc:/pic:/video:/audio:/zip:/exe:/pdf:/ext:xxx）：
    //    目录不参与类型浏览；文件扩展名必须命中过滤集合（小写、无前导点）。
    //    此处为统一的后置过滤，避免依赖 dfm-search 库层
    //    “Boolean 查询 + fileExtensions”组合在不同库版本上的行为差异
    //    （部分版本组合后返回 0 结果，如 “doc: 8月 会议” 搜不到任何文件）。
    if (!info.typeFilter.isEmpty()) {
        const QFileInfo fileInfo(filePath);
        if (fileInfo.isDir())
            return false;
        const QString suffix = fileInfo.suffix().toLower();
        if (!info.typeFilter.contains(suffix))
            return false;
    }

    return true;
}

QVariantHash FileSearchUtils::extraData(const QFileInfo &info, const QStringList &keywords)
{
    qCDebug(logDaemon) << "Generating tailer data for file:" << info.absoluteFilePath();

    QVariantHash hash;
    QStringList datas;
    auto config = Configer::instance()->group(GRANDSEARCH_TAILER_GROUP);

    // 修改时间
    auto timeModified = CommonTools::getFileModifiedTime(info.absoluteFilePath()).toString("yyyy-MM-dd hh:mm");
    hash.insert(GRANDSEARCH_PROPERTY_ITEM_MODIFIED_TIME, timeModified);
    qCDebug(logDaemon) << "Added modification time:" << timeModified;

    // 上级目录为可配置项
    if (config->value(GRANDSEARCH_TAILER_PARENTDIR, false)) {
        datas.append(info.absolutePath());
        qCDebug(logDaemon) << "Added parent directory to tailer:" << info.absolutePath();
    }

    // Inject keywords for highlighting
    if (!keywords.isEmpty())
        hash.insert(GRANDSEARCH_PROPERTY_ITEM_KEYWORDS, keywords);

    hash.insert(GRANDSEARCH_PROPERTY_ITEM_TAILER, datas);
    return hash;
}

QStringList FileSearchUtils::buildDFMSearchFileTypes(const QList<Group> &groupList)
{
    qCDebug(logDaemon) << "Building DFM search file types - Groups:" << groupList.size();

    static const QMap<Group, QString> kGroupToTypeMap = {
        { Group::Folder, "dir" },
        { Group::Picture, "pic" },
        { Group::Audio, "audio" },
        { Group::Video, "video" },
        { Group::Document, "doc" }
    };

    const bool containFile = groupList.contains(Group::File);
    const bool containFolder = groupList.contains(Group::Folder);
    QStringList types;

    if (containFile) {
        qCDebug(logDaemon) << "File group detected - Adding all file types";
        types = kGroupToTypeMap.values();
        types << "app"
              << "archive"
              << "other";
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

    return DFMSEARCH::Global::isPinyinAcronymSequence(str);
}

bool FileSearchUtils::hasWildcard(const QString &str)
{
    return str.contains('*') || str.contains('?');
}
