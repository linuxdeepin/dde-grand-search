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
#ifndef SEARCHHELPER_H
#define SEARCHHELPER_H

#include <QHash>

#define DOCUMENT_GROUP      "text"
#define PICTURE_GROUP       "img"
#define AUDIO_GROUP         "msc"
#define VIDEO_GROUP         "vdo"
#define FILE_GROUP          "file"
#define FOLDER_GROUP        "fld"
#define APPLICATION_GROUP   "app"
#define DEF_ALL_GROUPS \
static const QStringList allGroups {\
DOCUMENT_GROUP, PICTURE_GROUP,\
AUDIO_GROUP, VIDEO_GROUP,\
FILE_GROUP, FOLDER_GROUP,\
APPLICATION_GROUP\
}

// json各字段属性名
#define JSON_GROUP_ATTRIBUTE     "Group"
#define JSON_SUFFIX_ATTRIBUTE    "Suffix"
#define JSON_KEYWORD_ATTRIBUTE   "Keyword"

// 文档类目
#define DOCUMENT_SUFFIX     "pdf,txt,doc,docx,dot,dotx,ppt,pptx,pot,potx,xls,xlsx,xlt,xltx,wps,wpt,rtf,md,latex"
// 图片类目
#define PICTURE_SUFFIX      "jpg,jpeg,jpe,bmp,png,gif,svg,tif,tiff"
// 音频类目
#define AUDIO_SUFFIX        "au,snd,mid,mp3,aif,aifc,aiff,m3u,ra,ram,wav,cda,wma,ape,mp2,mpa"
// 视频类目
#define VIDEO_SUFFIX        "avi,mov,mp4,mpg,mpeg,qt,rm,rmvb,mkv,asx,asf,flv,3gp,mpe"
// 文件类目
#define FILE_SUFFIX         "zip,rar,z,deb,lib,iso,html,js"

#define searchHelper SearchHelper::instance()

class SearchHelper
{
public:
    static SearchHelper *instance();

    bool parseKeyword(const QString &keyword, QStringList &groupList, QStringList &suffixList, QStringList &keywordList);
    bool isSuffix(const QString &suffix);
    bool isGroupName(const QString &name);
    QStringList getSearcherByGroupName(const QString &name);
    QStringList getSuffixByGroupName(const QString &name);
    QString tropeInputSymbol(const QString &pattern);
    bool isSimplifiedChinese() const;

private:
    explicit SearchHelper();
    void initSuffixTable();
    void initGroupSuffixTable();
    void initGroupSearcherTable();

private:
    // 后缀表
    QStringList m_docSuffixTable;
    QStringList m_picSuffixTable;
    QStringList m_audioSuffixTable;
    QStringList m_videoSuffixTable;
    QStringList m_fileSuffixTable;

    // 类目-后缀映射表
    QHash<QString, QStringList> m_groupSuffixHash;
    // 类目-搜索项映射表
    QHash<QString, QStringList> m_groupSearcherHash;
};

#endif // SEARCHHELPER_H
