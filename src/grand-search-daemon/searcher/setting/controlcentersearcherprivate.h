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
#ifndef CONTROLCENTERSEARCHERPRIVATE_H
#define CONTROLCENTERSEARCHERPRIVATE_H

#include "controlcentersearcher.h"
#include "global/matcheditem.h"

#include <QFuture>
#include <QtConcurrent>

typedef QSharedPointer<GrandSearch::MatchedItem> SettingItemPointer;

class ControlCenterSearcherPrivate
{
    friend class ControlCenterSearcher;
public:
    explicit ControlCenterSearcherPrivate(ControlCenterSearcher *parent);
private:
    static void createIndex(ControlCenterSearcherPrivate *);
private:
    ControlCenterSearcher *q;

    bool m_inited = false;
    volatile bool m_creating = false;
    QFuture<void> m_creatingIndex;

    //索引表
    QReadWriteLock m_lock;
    QHash<QString, QList<SettingItemPointer>> m_indexTable;
};

#endif // CONTROLCENTERSEARCHERPRIVATE_H
