// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extendsearcher.h"
#include "extendsearcher_p.h"
#include "extendworker.h"
#include "searchplugin/pluginliaison.h"

#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

ExtendSearcherPrivate::ExtendSearcherPrivate(ExtendSearcher *parent)
    : QObject(parent)
{
}

ExtendSearcher::ExtendSearcher(const QString &name, QObject *parent)
    : Searcher(parent),
      d(new ExtendSearcherPrivate(this))
{
    d->m_name = name;
    qCDebug(logDaemon) << "Initializing extend searcher:" << name;
}

void ExtendSearcher::setService(const QString &service, const QString &address, const QString &interface, const QString &ver)
{
    if (Q_UNLIKELY(address.isEmpty()
                   || service.isEmpty() || interface.isEmpty()
                   || ver.isEmpty())) {
        qCWarning(logDaemon) << "Invalid service configuration - Service:" << service
                             << "Address:" << address
                             << "Interface:" << interface << "Version:" << ver;
        return;
    }

    d->m_service = service;
    d->m_address = address;
    d->m_interface = interface;
    d->m_version = ver;
    qCDebug(logDaemon) << "Service configured for searcher:" << d->m_name
                       << "- Service:" << service
                       << "Address:" << address
                       << "Interface:" << interface << "Version:" << ver;
}

void ExtendSearcher::setActivatable(Activatable act)
{
    d->m_activatable = act;
}

QString ExtendSearcher::name() const
{
    return d->m_name;
}

bool ExtendSearcher::isActive() const
{
    if (d->m_service.isEmpty())
        return false;

    QDBusConnectionInterface *cif = QDBusConnection::sessionBus().interface();
    Q_ASSERT(cif);

    return cif->isServiceRegistered(d->m_service);
}

bool ExtendSearcher::activate()
{
    // 受管理的插件通过进程激活
    if (d->m_activatable == InnerActivation) {
        bool ret = false;
        qCDebug(logDaemon) << "Requesting inner activation for searcher:" << d->m_name;
        emit activateRequest(name(), ret);
        return ret;
    } else if (d->m_activatable == Trigger) {
        // 激活DBus服务
        qCDebug(logDaemon) << "Triggering DBus service activation for:" << d->m_service;
        auto msg = QDBusMessage::createMethodCall(d->m_service, d->m_address,
                                                  "org.freedesktop.DBus.Peer", "Ping");
        QDBusConnection::sessionBus().asyncCall(msg, 5);
        return true;
    }

    qCWarning(logDaemon) << "No activation method available for searcher:" << d->m_name;
    return false;
}

ProxyWorker *ExtendSearcher::createWorker() const
{
    ExtendWorker *worker = new ExtendWorker(name());
    if (worker->setService(d->m_service, d->m_address, d->m_interface, d->m_version))
        return worker;

    delete worker;
    qCWarning(logDaemon) << "Failed to create worker - Service configuration invalid:"
                         << "Service:" << d->m_service
                         << "Address:" << d->m_address
                         << "Interface:" << d->m_interface
                         << "Version:" << d->m_version;
    return nullptr;
}

bool ExtendSearcher::action(const QString &action, const QString &item)
{
    PluginLiaison cont;
    if (cont.init(d->m_service, d->m_address, d->m_interface, d->m_version, d->m_name)) {
        if (cont.action(action, item))
            return true;
        else
            qCWarning(logDaemon) << "Invalid action requested - Action:" << action
                                 << "Item:" << item
                                 << "Searcher:" << d->m_name;
    } else {
        qCWarning(logDaemon) << "Failed to initialize plugin liaison - Searcher:" << d->m_name
                             << "Service:" << d->m_service
                             << "Address:" << d->m_address
                             << "Interface:" << d->m_interface
                             << "Version:" << d->m_version;
    }

    return false;
}
