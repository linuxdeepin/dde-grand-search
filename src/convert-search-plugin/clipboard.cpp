// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clipboard.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logClip, "convert.search.clipboard")

namespace Clipboard {
namespace {

// 尝试通过 deepin 剪贴板 DBus 服务写入文本。
// 兼容旧版服务名与新版服务名；不依赖未证实的私有签名，
// 仅调用 deepin 剪贴板管理器公开接口 SetClipboardData。
bool setViaDeepinDBus(const QString &text)
{
    // 候选服务名（旧 UOS/deepin 与 新版 dde-clipboard 6.x）
    static const char *kServices[] = {
        "com.deepin.daemon.ClipboardManager",
        "org.deepin.dde.Clipboard1",
        nullptr
    };
    const char *kPath = "/com/deepin/daemon/ClipboardManager";
    const char *kNewPath = "/org/deepin/dde/Clipboard1";

    for (int i = 0; kServices[i]; ++i) {
        const QString svc = QString::fromLatin1(kServices[i]);
        if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(svc))
            continue;
        // 新版路径与旧版不同，按服务名选路径
        const QString path = (svc == "org.deepin.dde.Clipboard1")
                                 ? QString::fromLatin1(kNewPath)
                                 : QString::fromLatin1(kPath);
        QDBusInterface iface(svc, path, svc, QDBusConnection::sessionBus());
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

    // 路径 1：Qt clipboard（需 QGuiApplication，X11 会话下可直接写系统剪贴板）
    if (qobject_cast<QGuiApplication *>(qApp)) {
        QClipboard *cb = qApp->clipboard();
        if (cb) {
            cb->setText(text);
            qCInfo(logClip) << "Clipboard set via Qt:" << text;
            return true;
        }
    }

    // 路径 2：deepin 剪贴板 DBus 服务（跨进程，无需本进程 GUI）
    if (setViaDeepinDBus(text))
        return true;

    qCWarning(logClip) << "All clipboard paths failed for:" << text;
    return false;
}

} // namespace Clipboard
