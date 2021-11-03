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

#include "extendsearcher.h"
#include "extendsearcher_p.h"
#include "extendworker.h"
#include "searchplugin/pluginliaison.h"

#include <QDBusConnectionInterface>
#include <QDBusInterface>

ExtendSearcherPrivate::ExtendSearcherPrivate(ExtendSearcher *parent)
    : QObject(parent)
{

}

ExtendSearcher::ExtendSearcher(const QString &name, QObject *parent)
    : Searcher(parent)
    , d(new ExtendSearcherPrivate(this))
{
    d->m_name = name;
}

void ExtendSearcher::setService(const QString &service, const QString &address, const QString &interface, const QString &ver)
{
    if (Q_UNLIKELY(address.isEmpty()
            || service.isEmpty() || interface.isEmpty()
            || ver.isEmpty()))
        return ;

    d->m_service = service;
    d->m_address = address;
    d->m_interface = interface;
    d->m_version = ver;
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
    //受管理的插件通过进程激活
    if (d->m_activatable == InnerActivation) {
        bool ret = false;
        emit activateRequest(name(), ret);
        return ret;
    } else if (d->m_activatable == Trigger) {
        // 激活DBus服务
        auto msg = QDBusMessage::createMethodCall(d->m_service, d->m_address,
                                                  "org.freedesktop.DBus.Peer", "Ping");
        QDBusConnection::sessionBus().asyncCall(msg, 5);
        return true;
    }

    return false;
}

ProxyWorker *ExtendSearcher::createWorker() const
{
    ExtendWorker *worker = new ExtendWorker(name());
    if (worker->setService(d->m_service, d->m_address, d->m_interface, d->m_version))
        return worker;

    delete worker;
    qWarning() << "ExtendWorker: fial to set service";
    return nullptr;
}

bool ExtendSearcher::action(const QString &action, const QString &item)
{
    PluginLiaison cont;
    if (cont.init(d->m_service, d->m_address, d->m_interface, d->m_version , d->m_name)) {
        if (cont.action(action, item))
            return true;
        else
            qWarning() << "action: invaild action:" << action;
    } else {
        qWarning() << "action: invaild searcher" << name();
    }

    return false;
}
