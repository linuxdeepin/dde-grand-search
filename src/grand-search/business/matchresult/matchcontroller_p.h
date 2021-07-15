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

class DaemonGrandSearchInterface;
class MatchControllerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit MatchControllerPrivate(MatchController *parent = nullptr);

public slots:
    void onMissionIdChanged(const QString &missionId);
    void onMatched(const QString &missonId);
    void onSearchCompleted(const QString &session);

public:
    void initConnect();

public:
    MatchController *q_p;

    QString m_missonId;

    DaemonGrandSearchInterface *m_daemonDbus = nullptr;
};

#endif // MATCHCONTROLLER_P_H
