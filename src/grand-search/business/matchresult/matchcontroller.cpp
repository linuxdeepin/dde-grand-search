/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *             houchengqiu<houchengqiu@uniontech.com>
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
#include "matchcontroller_p.h"
#include "matchcontroller.h"
#include "../query/querycontroller.h"
#include "contacts/interface/daemongrandsearchinterface.h"

using namespace GrandSearch;

class MatchControllerGlobal : public MatchController {};
Q_GLOBAL_STATIC(MatchControllerGlobal, matchControllerGlobal)

MatchControllerPrivate::MatchControllerPrivate(MatchController *parent)
    : q_p(parent)
{
    m_daemonDbus = new DaemonGrandSearchInterface(this);

    initConnect();
}

void MatchControllerPrivate::initConnect()
{
    connect(QueryController::instance(), &QueryController::missionIdChanged, this, &MatchControllerPrivate::onMissionIdChanged);

    connect(m_daemonDbus, &DaemonGrandSearchInterface::Matched, this, &MatchControllerPrivate::onMatched);
}

void MatchControllerPrivate::onMissionIdChanged(const QString &missionId)
{
    m_missonId = missionId;
}

void MatchControllerPrivate::onMatched(const QString &missonId)
{
    if (missonId != m_missonId || m_missonId.isEmpty())
        return;

    //获取数据解析
    qDebug() << QString("m_daemonDbus->MatchedBuffer begin missonId:%1").arg(m_missonId);
    QByteArray matchedBytes = m_daemonDbus->MatchedBuffer(m_missonId);
    qDebug() << QString("m_daemonDbus->MatchedBuffer end   missonId:%1").arg(m_missonId);

    QDataStream stream(&matchedBytes, QIODevice::ReadOnly);

    MatchedItemMap items;
    stream >> items;

    emit q_p->matchedResult(items);

    qDebug() << QString("matched %1 groups.").arg(items.size());
}

void MatchControllerPrivate::onSearchCompleted(const QString &session)
{
    if (session != m_missonId || m_missonId.isEmpty())
        return;

    //通知界面刷新，若界面有数据不做处理，否则就刷新

}

MatchController::MatchController(QObject *parent)
    : QObject(parent)
    , d_p(new MatchControllerPrivate(this))
{

}

MatchController::~MatchController()
{

}

MatchController *MatchController::instance()
{
    return matchControllerGlobal;
}

