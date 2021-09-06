/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef BUILTINSEARCH_H
#define BUILTINSEARCH_H

//定义内置搜索项的名称
#define GRANDSEARCH_CLASS_FILE_DEEPIN "com.deepin.dde-grand-search.file-deepin"
#define GRANDSEARCH_CLASS_FILE_FSEARCH "com.deepin.dde-grand-search.file-fsearch"
#define GRANDSEARCH_CLASS_APP_DESKTOP "com.deepin.dde-grand-search.app-desktop"
#define GRANDSEARCH_CLASS_SETTING_CONTROLCENTER "com.deepin.dde-grand-search.setting-controlcenter"
#define GRANDSEARCH_CLASS_WEB_STATICTEXT "com.deepin.dde-grand-search.web-statictext"

#define GRANDSEARCH_GROUP_APP                "com.deepin.dde-grand-search.group.application"
#define GRANDSEARCH_GROUP_SETTING            "com.deepin.dde-grand-search.group.setting-controlcenter"
#define GRANDSEARCH_GROUP_WEB                "com.deepin.dde-grand-search.group.web"

#define GRANDSEARCH_GROUP_FOLDER             "com.deepin.dde-grand-search.group.folder"

#define GRANDSEARCH_GROUP_FILE               "com.deepin.dde-grand-search.group.files"
#define GRANDSEARCH_GROUP_FILE_VIDEO         "com.deepin.dde-grand-search.group.files.video"
#define GRANDSEARCH_GROUP_FILE_AUDIO         "com.deepin.dde-grand-search.group.files.audio"
#define GRANDSEARCH_GROUP_FILE_PICTURE       "com.deepin.dde-grand-search.group.files.picture"
#define GRANDSEARCH_GROUP_FILE_DOCUMNET      "com.deepin.dde-grand-search.group.files.document"

#define GRANDSEARCH_PROPERTY_ITEM_LEVEL      "itemLevel"

#define GRANDSEARCH_PROPERTY_ITEM_LEVEL_FIRST   1
#define GRANDSEARCH_PROPERTY_ITEM_LEVEL_SECOND  2
#define GRANDSEARCH_PROPERTY_ITEM_LEVEL_THIRD   3

#define DEF_BUILTISEARCH_NAMES  \
static const QStringList predefBuiltinSearches { \
GRANDSEARCH_CLASS_FILE_DEEPIN, \
GRANDSEARCH_CLASS_FILE_FSEARCH, \
GRANDSEARCH_CLASS_APP_DESKTOP, \
GRANDSEARCH_CLASS_SETTING_CONTROLCENTER, \
GRANDSEARCH_CLASS_WEB_STATICTEXT \
}

#endif // BUILTINSEARCH_H
