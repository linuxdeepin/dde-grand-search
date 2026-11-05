// SPDX-FileCopyrightText: 2026 testerxydw
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#ifdef ENABLE_SANITIZER
#include <sanitizer/asan_interface.h>
#endif

#include <QCoreApplication>
#include <QLocale>

static void noMessageHandler(QtMsgType, const QMessageLogContext &, const QString &)
{
}

int main(int argc, char *argv[])
{
    // 插件为无界面 DBus 进程，测试同样不需要 GUI
    QCoreApplication app(argc, argv);

    // 结果文案经 QLocale() 格式化（小数点、千分位），固定为 C locale 保证断言稳定
    QLocale::setDefault(QLocale::c());

    testing::InitGoogleTest(&argc, argv);

    qInstallMessageHandler(noMessageHandler);
    int ret = RUN_ALL_TESTS();

#ifdef ENABLE_SANITIZER
    __sanitizer_set_report_path("../../asan_convert_search_plugin.log");
#endif
    return ret;
}
