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
#define AC_GroupHash_App                "app"
#define AC_GroupHash_Folder             "folder"
#define AC_GroupHash_File               "file"

// 组-列表对象名
#define AC_GroupObjName_App             "Group_App"
#define AC_GroupObjName_Folder          "Group_Folder"
#define AC_GroupObjName_File            "Group_File"

// 组-中文词典翻译
#define AC_GroupName_App                QObject::tr("GroupApp")
#define AC_GroupName_Folder             QObject::tr("GroupFolder")
#define AC_GroupName_File               QObject::tr("GroupFile")
};

#endif // DATADEFINE_H
