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

    // 文档格式
    static QRegExp docReg("^((pdf)|(txt)|(doc)|(docx)|(dot)|(dotx)|(ppt)|(pptx)|(pot)|(potx)"
                          "|(xls)|(xlsx)|(xlt)|(xltx)|(wps)|(wpt)|(rtf)|(md)|(latex))$", Qt::CaseInsensitive);
    // 图片格式
    static QRegExp pictureReg("^((jpg)|(jpeg)|(jpe)|(bmp)|(png)|(gif)|(svg)|(tif)|(tiff))$", Qt::CaseInsensitive);
    // 视频格式
    static QRegExp videoReg("^((avi)|(mov)|(mp4)|(mpg)|(mpeg)|(qt)|(rm)|(rmvb)"
                            "|(mkv)|(asx)|(asf)|(flv)|(3gp)|(mpe))$", Qt::CaseInsensitive);
    // 音频格式
    static QRegExp musicReg("^((au)|(snd)|(mid)|(mp3)|(aif)|(aifc)|(aiff)|(m3u)|(ra)"
                            "|(ram)|(wav)|(cda)|(wma)|(ape)|(mp2)|(mpa))$", Qt::CaseInsensitive);
    // 文件格式
    static QRegExp fileReg("^((zip)|(rar)|(z)|(deb)|(lib)|(iso)|(html)|(js))$", Qt::CaseInsensitive);

    if (docReg.exactMatch(suffix)) {
        group = Document;
    } else if (pictureReg.exactMatch(suffix)) {
        group = Picture;
    } else if (videoReg.exactMatch(suffix)) {
        group = Video;
    } else if (musicReg.exactMatch(suffix)) {
        group = Audio;
    } else if (fileReg.exactMatch(suffix)) {
        group = File;
    }

    return group;
}

FileSearchUtils::SearchType FileSearchUtils::checkSearchTypeAndToRegexp(QString &pattern)
{
    if (!pattern.contains(':') || pattern == ':')
        return NormalSearch;

    QString tmpStr = pattern;
    // 对特殊字符进行转义
    pattern = tropeInputSymbol(pattern);
    int index = pattern.indexOf(':');
    QString suffix = pattern.mid(0, index);
    QString keyword = pattern.mid(index + 1);

    pattern = toSuffixRegexp(suffix, keyword);
    if (pattern.isEmpty()) {
        pattern = tmpStr;
        return NormalSearch;
    }
    return SuffixSearch;
}

QString FileSearchUtils::toSuffixRegexp(const QString &suffix, const QString &pattern)
{
    QString keyword = pattern;
    auto group = getGroupBySuffix(suffix);
    if (Unknown == group) {
        if (keyword.trimmed().isEmpty() || suffix.trimmed().isEmpty())
            return "";

        // 未知类型，冒号前后拆分搜索取并集
        QStringList keywords = keyword.split(':');
        keyword = suffix;
        for (const auto &word : keywords) {
            keyword.append("|" + word);
        }
        return keyword;
    } else {
        return QString(R"(%1.*\.%2$)").arg(keyword, suffix);
    }
}

QString FileSearchUtils::toGroupRegexp(const QString &group, const QString &pattern)
{
    // TODO 类目搜索
    Q_UNUSED(group)
    Q_UNUSED(pattern)

    return "";
}

QString FileSearchUtils::tropeInputSymbol(const QString &pattern)
{
    const int len = pattern.length();
    QString rx;
    rx.reserve(len + len / 16);
    int i = 0;
    const QChar *wc = pattern.unicode();

    while (i < len) {
        const QChar c = wc[i++];
        switch (c.unicode()) {
        case '*':
        case '?':
        case '\\':
        case '$':
        case '(':
        case ')':
        case '+':
        case '.':
        case '^':
        case '{':
        case '|':
        case '}':
        case '[':
        case ']':
            rx += QLatin1Char('\\');
            rx += c;
            break;
        default:
            rx += c;
            break;
        }
    }

    return rx;
}
