// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QString>

// 系统剪贴板写入封装。
//
// 背景：本插件运行在 dde-grand-search-daemon（无头守护进程）中，
// 历史实现用 QGuiApplication::clipboard() 但因主程序是 QCoreApplication，
// clipboard() 永远返回 nullptr，导致点击复制 / UOS AI 传内容均失败。
//
// 策略（依次尝试，任一成功即返回 true）：
//   1) Qt clipboard（若程序为 QGuiApplication 且平台可用，X11 下可直接写）
//   2) deepin 剪贴板 DBus 服务（com.deepin.daemon.ClipboardManager 旧版 /
//      org.deepin.dde.Clipboard1 新版），跨进程写系统剪贴板，无需本进程有 GUI
//
// 全部失败则静默返回 false，绝不抛异常或段错误。
namespace Clipboard {

// 将文本写入系统剪贴板。返回是否成功发起写入。
bool setText(const QString &text);

} // namespace Clipboard

#endif // CLIPBOARD_H
