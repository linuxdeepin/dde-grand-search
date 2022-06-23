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
#ifndef PROXYWORKER_H
#define PROXYWORKER_H

#include "global/matcheditem.h"

#include <QObject>

namespace GrandSearch {

//! 异步的Worker必须在析构函数中保证资源安全释放，且析构函数不能阻塞太久

class ProxyWorker : public QObject
{
    Q_OBJECT
public:
    enum Status {Ready,Runing,Completed,Terminated};
public:
    explicit ProxyWorker(const QString &name, QObject *parent = nullptr);
    QString name() const;
    virtual void setContext(const QString &context) = 0;
    virtual bool isAsync() const = 0;
    virtual bool working(void *context) = 0;
    virtual void terminate() = 0;
    virtual Status status() = 0;
    virtual bool hasItem() const = 0;
    virtual MatchedItemMap takeAll() = 0;
signals:
    void unearthed(ProxyWorker *worker);
    void asyncFinished(ProxyWorker *worker); //异步搜索结束信号
protected:
    QString m_name;
};

}

#endif // PROXYWORKER_H
