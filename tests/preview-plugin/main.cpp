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

#include <gtest/gtest.h>

#ifdef ENABLE_SANITIZER
#include <sanitizer/asan_interface.h>
#endif

#include <QApplication>
#include <QAccessible>

#include <unistd.h>

static void noMessageHandler(QtMsgType type, const QMessageLogContext &context,
                                   const QString &message)
{
    return;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    testing::InitGoogleTest(&argc,argv);

    //不打印应用的输出日志
    qInstallMessageHandler(noMessageHandler);
    int ret = RUN_ALL_TESTS();

    // Qt的Guipplication，再开启事件循环后会自动创建一个QAccessibleInterface，但没有释放。
    // 这里手动释放
    if (auto ifs = QAccessible::queryAccessibleInterface(&app)) {
        auto id = QAccessible::uniqueId(ifs);
        QAccessible::deleteAccessibleInterface(id);
    }
    QAccessible::cleanup();

    // must be here
#ifdef ENABLE_SANITIZER
    __sanitizer_set_report_path("../../asan_preview_plugin.log");
#endif
    return ret;
}
