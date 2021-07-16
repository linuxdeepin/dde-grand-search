/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     houchengqiu <houchengqiu@uniontech.com>
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

#ifndef DATADEFINE_H
#define DATADEFINE_H

#include <QString>
#include <QObject>

namespace GrandSearch {

// 组-hash值
#define GroupHash_App                "Application"
#define GroupHash_Folder             "Folder"
#define GroupHash_File               "File"

// 组-列表对象名
#define GroupObjName_App             "Group_App"
#define GroupObjName_Folder          "Group_Folder"
#define GroupObjName_File            "Group_File"

// 组-中文词典翻译
#define GroupName_App                QObject::tr("GroupApp")
#define GroupName_Folder             QObject::tr("GroupFolder")
#define GroupName_File               QObject::tr("GroupFile")

static const uint DelayReponseTime          = 50;       // 输入延迟搜索时间
static const uint KeepAliveTime             = 3000;     // 搜索过程中，调用后端心跳函数间隔时间

static const uint MainWindowWidth           = 760;      // 主界面宽度
static const uint MainWindowHeight          = 68;       // 主界面高度
static const uint MainWindowExpandHeight    = 520;      // 主界面高度
static const uint EntraceWidgetWidth        = 740;      // 搜索界面宽度度
static const uint EntraceWidgetHeight       = 48;       // 搜索界面高度
static const uint MatchItemWidth            = 740;      // 搜索结果项的宽度
static const uint MatchItemHeight           = 36;       // 搜索结果项的高度
static const uint WidgetMargins             = 10;       // 界面边距

};

#endif // DATADEFINE_H
