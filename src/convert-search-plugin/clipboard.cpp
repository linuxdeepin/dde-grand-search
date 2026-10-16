// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clipboard.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QProcess>
#include <QStandardPaths>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logClip, "convert.search.clipboard")

namespace Clipboard {
namespace {

// 尝试通过 xclip 命令写入 X11 剪贴板（QCoreApplication 进程无 GUI 时的通用路径）。
// xclip 会 fork 出持有选区所有权的子进程，需常驻而非等待退出。
bool setViaXClip(const QString &text)
{
    const QString xclip = QStandardPaths::findExecutable(QStringLiteral("xclip"));
    if (xclip.isEmpty())
        return false;
    QProcess proc;
    proc.start(xclip, {QStringLiteral("-selection"), QStringLiteral("clipboard"),
                       QStringLiteral("-i")});
    if (!proc.waitForStarted(1000))
        return false;
    proc.write(text.toUtf8());
    proc.closeWriteChannel();
    // xclip fork 后台持有选区；主进程退出码 0 即写入发起成功
    if (!proc.waitForFinished(2000) || proc.exitStatus() != QProcess::NormalExit
        || proc.exitCode() != 0)
        return false;
    qCInfo(logClip) << "Clipboard set via xclip";
    return true;
}

// 尝试通过 deepin/UOS 旧版剪贴板 DBus 服务写入文本（新版为面板 UI 接口，无写入方法）。
bool setViaDeepinDBus(const QString &text)
{
    static const char *kServices[] = {
        "com.deepin.daemon.ClipboardManager",
        nullptr
    };
    const char *kPath = "/com/deepin/daemon/ClipboardManager";

    for (int i = 0; kServices[i]; ++i) {
        const QString svc = QString::fromLatin1(kServices[i]);
        if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(svc))
            continue;
        QDBusInterface iface(svc, QString::fromLatin1(kPath), svc, QDBusConnection::sessionBus());
        if (!iface.isValid())
            continue;
        // SetClipboardData(uuid:string, mime:string, data:ay, info:a{sv})
        //   uuid 传空串由管理器分配；mime 用 text/plain；data 为 UTF-8 文本
        QVariantList args;
        args << QString()                                  // uuid
             << QStringLiteral("text/plain")               // mime
             << QByteArray(text.toUtf8())                  // data
             << QVariantMap();                             // info
        QDBusReply<void> reply = iface.callWithArgumentList(
            QDBus::Block, QStringLiteral("SetClipboardData"), args);
        if (reply.isValid()) {
            qCInfo(logClip) << "Clipboard set via DBus:" << svc;
            return true;
        }
        qCWarning(logClip) << "SetClipboardData failed on" << svc
                           << reply.error().message();
    }
    return false;
}

} // namespace

bool setText(const QString &text)
{
    if (text.isEmpty())
        return false;

    // 路径 1：Qt clipboard（进程为 QGuiApplication 时直接写系统剪贴板）
    if (qobject_cast<QGuiApplication *>(qApp)) {
        QClipboard *cb = qApp->clipboard();
        if (cb) {
            cb->setText(text);
            qCInfo(logClip) << "Clipboard set via Qt:" << text;
            return true;
        }
    }

    // 路径 2：xclip 命令（X11 会话通用，无需 GUI 环境变量以外的依赖）
    if (setViaXClip(text))
        return true;

    // 路径 3：deepin/UOS 旧版剪贴板 DBus 服务
    if (setViaDeepinDBus(text))
        return true;

    qCWarning(logClip) << "All clipboard paths failed for:" << text;
    return false;
}

} // namespace Clipboard
