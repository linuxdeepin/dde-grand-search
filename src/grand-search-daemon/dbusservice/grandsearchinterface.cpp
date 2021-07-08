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

#include "grandsearchinterface.h"
#include "grandsearchinterfaceprivate.h"
#include "maincontroller/maincontroller.h"

GrandSearchInterfacePrivate::GrandSearchInterfacePrivate(GrandSearchInterface *parent) : q(parent)
{

}

GrandSearchInterfacePrivate::~GrandSearchInterfacePrivate()
{
    if (m_main) {
        delete m_main;
        m_main = nullptr;
    }
}

GrandSearchInterface::GrandSearchInterface(QObject *parent)
    : QObject(parent)
    , d(new GrandSearchInterfacePrivate(this))
{

}

GrandSearchInterface::~GrandSearchInterface()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

bool GrandSearchInterface::init()
{
    if (d->m_main)
        return false;

    d->m_main = new MainController;
    return d->m_main->init();
}

bool GrandSearchInterface::Search(const QString &session, const QString &key)
{
    return false;
}

void GrandSearchInterface::Terminate()
{

}

QString GrandSearchInterface::MatchingResults(const QString &session)
{

}

QString GrandSearchInterface::MatchingBuffer(const QString &session)
{

}

bool GrandSearchInterface::OpenWithPlugin(const QString &item)
{
    return false;
}

bool GrandSearchInterface::Configure(const QMap<QString, QString> &)
{
    return false;
}

QMap<QString, QString> GrandSearchInterface::Configuration() const
{

}

bool GrandSearchInterface::SetFeedBackStrategy(const QMap<QString, QString> &)
{
    return false;
}

QMap<QString, QString> GrandSearchInterface::FeedBackStrategy() const
{

}

bool GrandSearchInterface::KeepAlive(const QString &session)
{
    return true;
}
