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
#ifndef DESKTOPAPPWORKER_H
#define DESKTOPAPPWORKER_H

#include "searcher/proxyworker.h"

#include <QMutex>

namespace GrandSearch {

class DesktopAppWorker : public ProxyWorker
{
    Q_OBJECT
public:
    explicit DesktopAppWorker(const QString &name, QObject *parent = nullptr);
    void setContext(const QString &context) Q_DECL_OVERRIDE;
    bool isAsync() const Q_DECL_OVERRIDE;
    bool working(void *context) Q_DECL_OVERRIDE;
    void terminate() Q_DECL_OVERRIDE;
    Status status() Q_DECL_OVERRIDE;
    bool hasItem() const Q_DECL_OVERRIDE;
    MatchedItemMap takeAll() Q_DECL_OVERRIDE;
public:
    void setIndexTable(const QHash<QString, QList<QSharedPointer<MatchedItem>>> &table);
private:
    QString group() const;
    QString buildKeyword(const QString &keyword);
private:
    QAtomicInt m_status = Ready;
    QString m_context;
    QHash<QString, QList<QSharedPointer<MatchedItem>>> m_indexTable;

    //搜索结果
    mutable QMutex m_mtx;
    MatchedItems m_items;
};

}

#endif // DESKTOPAPPWORKER_H
