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
#ifndef MATCHCONTROLLER_P_H
#define MATCHCONTROLLER_P_H

#include "matchcontroller.h"

#include <QAtomicInteger>

namespace GrandSearch {

class DaemonGrandSearchInterface;
class MatchControllerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit MatchControllerPrivate(MatchController *parent = nullptr);

public slots:
    void onMatched(const QString &missionId);
    void onSearchCompleted(const QString &missionId);
    void sendCacheItems();

public:
    void initConnect();

public:
    MatchController *q_p = nullptr;

    QString m_missionId;
    QString m_missionContent;

    QAtomicInteger<bool> m_missionIdChanged = true;
    MatchedItemMap m_cacheItems;
    bool m_enableBestMatch = true;
    int m_firstItemLimit = 30;
    int m_firstWaitTime = 500;
    int m_bestItemMaxCount = 4;
    QSharedPointer<QTimer> m_waitTimer = nullptr;

    DaemonGrandSearchInterface *m_daemonDbus = nullptr;
};

}

#endif // MATCHCONTROLLER_P_H
