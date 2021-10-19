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
#ifndef EXTENDWORKER_H
#define EXTENDWORKER_H

#include "searcher/proxyworker.h"

class PluginLiaison;
class ExtendWorker : public ProxyWorker
{
public:
    explicit ExtendWorker(const QString &name, QObject *parent = nullptr);
    ~ExtendWorker();
    bool setService(const QString &service, const QString &address,
                    const QString &interface, const QString &ver);
    void setContext(const QString &context) Q_DECL_OVERRIDE;
    bool isAsync() const Q_DECL_OVERRIDE;
    bool working(void *context) Q_DECL_OVERRIDE;
    void terminate() Q_DECL_OVERRIDE;
    Status status() Q_DECL_OVERRIDE;
    bool hasItem() const Q_DECL_OVERRIDE;
    GrandSearch::MatchedItemMap takeAll() Q_DECL_OVERRIDE;
protected slots:
    void tryWorking();
    void onWorkFinished(const GrandSearch::MatchedItemMap &ret);
private:
    enum CallState{NotCall, WaitCall, Called};
private:
    QAtomicInt m_status = Ready;
    QString m_context;
    QString m_taskID;

    PluginLiaison *m_liaison = nullptr;
    QMutex m_callMtx;
    volatile CallState m_callSerach = NotCall;
    QTimer m_timeout;

    //搜索结果
    mutable QMutex m_mtx;
    GrandSearch::MatchedItemMap m_items;
};

#endif // EXTENDWORKER_H
