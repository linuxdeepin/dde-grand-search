// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "indexstatusmonitor.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QLoggingCategory>
#include <QVariantMap>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

using namespace GrandSearch;

namespace {
// 文管索引 D-Bus 服务（service/path/interface 同名，见文管 assets/dbus/*.xml）
inline constexpr char kTextIndexService[] { "org.deepin.Filemanager.TextIndex" };
inline constexpr char kTextIndexPath[] { "/org/deepin/Filemanager/TextIndex" };
inline constexpr char kOcrIndexService[] { "org.deepin.Filemanager.OcrIndex" };
inline constexpr char kOcrIndexPath[] { "/org/deepin/Filemanager/OcrIndex" };
}

IndexStatusMonitor::IndexStatusMonitor(QObject *parent)
    : QObject(parent)
{
    // 信号直连，接口创建后再补连由 ensureInterface 完成
}

IndexStatusMonitor::~IndexStatusMonitor()
{
}

QString IndexStatusMonitor::serviceName(Index idx)
{
    return idx == Index::Text ? QString::fromLatin1(kTextIndexService)
                              : QString::fromLatin1(kOcrIndexService);
}

QString IndexStatusMonitor::objectPath(Index idx)
{
    return idx == Index::Text ? QString::fromLatin1(kTextIndexPath)
                              : QString::fromLatin1(kOcrIndexPath);
}

bool IndexStatusMonitor::ensureInterface(Index idx)
{
    QDBusInterface *iface = interface(idx);
    if (iface && iface->isValid())
        return true;

    const QString service = serviceName(idx);
    QDBusConnectionInterface *busIface = QDBusConnection::sessionBus().interface();
    if (!busIface) {
        qCWarning(logGrandSearch) << "IndexStatusMonitor: no session bus interface";
        return false;
    }

    // 服务未注册时兜底拉起（正常由 dbus activation 拉起）
    if (!busIface->isServiceRegistered(service)) {
        auto reply = busIface->startService(service);
        if (!reply.isValid())
            qCWarning(logGrandSearch) << "IndexStatusMonitor: failed to start service" << service
                                      << ":" << reply.error().message();
    }

    // 接口失效重建时清理旧对象（避免服务反复重启时累积）
    if (QDBusInterface *stale = interface(idx)) {
        QDBusConnection::sessionBus().disconnect(
                service, objectPath(idx), service,
                QStringLiteral("IndexStatusChanged"),
                this,
                idx == Index::Text ? SLOT(onTextStatusChanged(QString, QString))
                                   : SLOT(onOcrStatusChanged(QString, QString)));
        stale->deleteLater();
    }

    iface = new QDBusInterface(service, objectPath(idx), service,
                               QDBusConnection::sessionBus(), this);
    if (!iface->isValid()) {
        qCWarning(logGrandSearch) << "IndexStatusMonitor: invalid interface for" << service
                                  << ":" << iface->lastError().message();
        delete iface;
        return false;
    }

    m_interfaces.insert(static_cast<int>(idx), iface);

    // 转发远端 IndexStatusChanged 信号为带索引类型的本地信号
    const bool connected = QDBusConnection::sessionBus().connect(
            service, objectPath(idx), service, QStringLiteral("IndexStatusChanged"),
            this,
            idx == Index::Text ? SLOT(onTextStatusChanged(QString, QString))
                               : SLOT(onOcrStatusChanged(QString, QString)));
    if (!connected)
        qCWarning(logGrandSearch) << "IndexStatusMonitor: failed to connect IndexStatusChanged for" << service;

    return true;
}

QDBusInterface *IndexStatusMonitor::interface(Index idx)
{
    return m_interfaces.value(static_cast<int>(idx)).data();
}

void IndexStatusMonitor::getIndexStatus()
{
    queryIndexStatus(Index::Text);
    queryIndexStatus(Index::Ocr);
}

void IndexStatusMonitor::queryIndexStatus(Index idx)
{
    if (!ensureInterface(idx)) {
        emit indexStatusResult(idx, QString(), QString(), false);
        return;
    }

    auto *watcher = new QDBusPendingCallWatcher(interface(idx)->asyncCall(QStringLiteral("GetIndexStatus")), this);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [this, idx](QDBusPendingCallWatcher *w) { handleStatusReply(idx, w); });
}

void IndexStatusMonitor::handleStatusReply(Index idx, QDBusPendingCallWatcher *watcher)
{
    watcher->deleteLater();
    QDBusPendingReply<QVariantMap> reply = *watcher;

    if (reply.isError()) {
        qCWarning(logGrandSearch) << "IndexStatusMonitor: GetIndexStatus failed for"
                                  << serviceName(idx) << ":" << reply.error().message();
        emit indexStatusResult(idx, QString(), QString(), false);
        return;
    }

    const QVariantMap status = reply.value();
    emit indexStatusResult(idx,
                           status.value(QStringLiteral("state")).toString(),
                           status.value(QStringLiteral("grade")).toString(),
                           true);
}

void IndexStatusMonitor::forceUpdateIndex(Index idx, const QStringList &paths, bool manual)
{
    if (!ensureInterface(idx)) {
        qCWarning(logGrandSearch) << "IndexStatusMonitor: cannot force update, interface unavailable for"
                                  << serviceName(idx);
        return;
    }

    QVariantMap options;
    options.insert(QStringLiteral("manual"), manual);
    interface(idx)->asyncCall(QStringLiteral("ForceUpdateIndex"), QVariant::fromValue(paths), options);
}

void IndexStatusMonitor::onTextStatusChanged(const QString &state, const QString &grade)
{
    emit indexStatusChanged(Index::Text, state, grade);
}

void IndexStatusMonitor::onOcrStatusChanged(const QString &state, const QString &grade)
{
    emit indexStatusChanged(Index::Ocr, state, grade);
}
