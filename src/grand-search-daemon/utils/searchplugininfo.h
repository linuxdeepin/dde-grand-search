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
#ifndef SEARCHPLUGININFO_H
#define SEARCHPLUGININFO_H

#include <QString>

namespace GrandSearch {

struct SearchPluginInfo
{
    enum Mode {Auto, Manual};   //运行模式，auto为由本程序控制启动，Manual为由外部控制
    enum Priority {High = 0, Middle, Low};  //优先级

    QString name;       //插件名，推荐使用com.company.app.xxxx 的格式
    QString ifsVersion; //dbus接口协议
    QString from;       //conf文件
    QString exec;       //插件可执行程序路径
    QString servie;     //dbus服务
    QString address;    //dbus地址
    QString interface;  //dbus接口

    Mode mode = Manual ;    //默认为Manual
    Priority priority = Low;    //仅在mode为auto时有效
};

}

#endif // SEARCHPLUGININFO_H
