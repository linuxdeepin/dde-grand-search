/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef QUERYCONTROLLER_P_H
#define QUERYCONTROLLER_P_H

#include "querycontroller.h"

#include <QObject>
#include <QTimer>

class DaemonGrandSearchInterface;
class QueryControllerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit QueryControllerPrivate(QueryController *parent = nullptr);

public slots:
    void onSearchTextChanged(const QString &txt);
    void keepAlive();

    void onSearchCompleted(const QString &missionId);

public:
    QueryController *q_p;
    QString m_missionId;
    QString m_searchText;
    QTimer *m_keepAliveTimer = nullptr;

    DaemonGrandSearchInterface *m_daemonDbus = nullptr;
};

#endif // QUERYCONTROLLER_P_H
