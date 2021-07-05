/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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

#include "../environments.h"

#include <QApplication>
#include <QDebug>

#include <unistd.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //设置应用信息
    app.setOrganizationName("deepin");
    app.setApplicationName("dde-grand-search");
    app.setApplicationDisplayName("DDE Grand Search");
    app.setApplicationVersion(VERSION);
    qInfo() << "starting" << app.applicationName() << app.applicationVersion() << getpid();

    app.exec();
}
