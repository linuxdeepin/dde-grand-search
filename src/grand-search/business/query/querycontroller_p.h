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

namespace GrandSearch {

class DaemonGrandSearchInterface;
class QueryControllerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit QueryControllerPrivate(QueryController *parent = nullptr);

public slots:
    // 发起新的搜索后，在搜索完成或结束前，周期调用后端心跳接口
    void keepAlive();

    // 接收后端搜索完成信号，停止心跳发送
    void onSearchCompleted(const QString &missionId);

public:
    QueryController *q_p = nullptr;
    QString m_missionId;
    QString m_searchText;
    QTimer *m_keepAliveTimer = nullptr;

    DaemonGrandSearchInterface *m_daemonDbus = nullptr;
};

}

#endif // QUERYCONTROLLER_P_H
