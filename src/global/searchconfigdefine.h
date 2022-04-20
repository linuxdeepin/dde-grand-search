/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
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
#ifndef SEARCHCONFIGDEFINE_H
#define SEARCHCONFIGDEFINE_H

// 全局搜索后端的名称
#define GRANDSEARCH_DAEMON_NAME "dde-grand-search-daemon"
#define GRANDSEARCH_NAME "dde-grand-search"

// 搜索配置版本
#define GRANDSEARCH_VERSION_GROUP           "Version_Group"
#define GRANDSEARCH_VERSION_CONFIG          "version.config"
#define GRANDSEARCH_VERSION_CONFIG_CURRENT  "0.1"

// 搜索范围
#define GRANDSEARCH_SEARCH_GROUP            "Search_Group"

// 拖尾
#define GRANDSEARCH_TAILER_GROUP            "Tailer_File_Group"
#define GRANDSEARCH_TAILER_PARENTDIR        "tailer.file.parentDir"
#define GRANDSEARCH_TAILER_TIMEMODEFIED     "tailer.file.timeModified"

// 搜索体验计划
#define GRANDSEARCH_PLAN_GROUP              "Plan_Group"
#define GRANDSEARCH_PLAN_EXPERIENCE         "plan.experience"

// 路径黑名单
#define GRANDSEARCH_BLACKLIST_GROUP         "Blacklist_Group"
#define GRANDSEARCH_BLACKLIST_PATH          "Blacklist.path"

// 个性化
#define GRANDSEARCH_CUSTOM_GROUP                        "Custom_Group"
#define GRANDSEARCH_CUSTOM_BESTMATCH                    "custom.bestMatch"
#define GRANDSEARCH_CUSTOM_BESTMATCH_FIRSTITEMLIMIT     "custom.bestMatch.firstItemLimit"
#define GRANDSEARCH_CUSTOM_BESTMATCH_FIRSTWAITTIME      "custom.bestMatch.firstWaitTime"
#define GRANDSEARCH_CUSTOM_BESTMATCH_MAXCOUNT           "custom.bestMatch.maxCount"

// 配置切换界面参数
#define SWITCHWIDGETWIDTH       502
#define SWITCHWIDGETHEIGHT      36
#define SWITCHWIDGETICONSIZE    36
#define CHECKBOXITEMWIDTH       502
#define CHECKBOXITEMHEIGHT      36

#endif // SEARCHCONFIGDEFINE_H
