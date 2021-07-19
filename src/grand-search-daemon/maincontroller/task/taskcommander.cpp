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

#include "taskcommander.h"
#include "taskcommanderprivate.h"

#include <QDateTime>
#include <QtConcurrent>

TaskCommanderPrivate::TaskCommanderPrivate(TaskCommander *parent)
    : QObject(parent)
    , q(parent)
{

}

TaskCommanderPrivate::~TaskCommanderPrivate()
{

}

void TaskCommanderPrivate::merge(GrandSearch::MatchedItemMap &addTo, const GrandSearch::MatchedItemMap &addFrom)
{
    for (auto iter = addFrom.begin(); iter != addFrom.end(); ++iter) {
        GrandSearch::MatchedItemMap::iterator org = addTo.find(iter.key());
        if (org == addTo.end()) {
            addTo.insert(iter.key(), iter.value());
        } else {
            org.value().append(iter.value());
        }
    }
}

void TaskCommanderPrivate::working(ProxyWorker *worker)
{
    Q_ASSERT(worker);
    worker->working(nullptr);
}

void TaskCommanderPrivate::onUnearthed(ProxyWorker *worker)
{
    Q_ASSERT(worker);
    if (m_allWorkers.contains(worker) && worker->hasItem()) {
        auto results = worker->takeAll();
        QWriteLocker lk(&m_lock);
        bool emptyBuffer = m_buffer.isEmpty();

        merge(m_results, results);
        merge(m_buffer, results);

        //回到主线程发送信号
        if (emptyBuffer)
            QMetaObject::invokeMethod(q, "matched", Qt::QueuedConnection);
    }
}

void TaskCommanderPrivate::onFinished()
{
    //工作线程退出，若之前调用了deleteSelf那么在这里执行释放，否则发送结束信号
    if (m_asyncLine.isFinished() && m_syncLine.isFinished()) {
        if (m_deleted)
            this->deleteLater();
        else
            emit q->finished();
    }
}

TaskCommander::TaskCommander(const QString &content, QObject *parent)
    : QObject(parent)
    , d(new TaskCommanderPrivate(this))
{
    d->m_id = QString::number(QDateTime::currentMSecsSinceEpoch());
    d->m_content = content;
}

TaskCommander::~TaskCommander()
{
    qDebug() << "task is deleted" << d->m_id;
}

QString TaskCommander::taskID() const
{
    return d->m_id;
}

QString TaskCommander::content() const
{
    return d->m_content;
}

bool TaskCommander::start()
{
    qDebug() << "start" << this->taskID() << d->m_working
             << "async" << d->m_asyncWorkers.size() << "sync" << d->m_syncWorkers.size();
    if (d->m_working)
        return false;

    d->m_working = true;
    //所有异步搜索项在一个线程中依次执行
    if (!d->m_asyncWorkers.isEmpty()) {
        d->m_asyncLine.setFuture(QtConcurrent::run([this]() {
            QString taskID = d->m_id;
            for (auto worker : d->m_asyncWorkers) {
                if (!d->m_working )
                    return;

                worker->working(&taskID);
            }
        }));
        connect(&d->m_asyncLine, &QFutureWatcherBase::finished, d, &TaskCommanderPrivate::onFinished);
    }

    //所有同步搜索项在线程池中执行
    if (!d->m_syncWorkers.isEmpty()) {
        d->m_syncLine.setFuture(QtConcurrent::map(d->m_syncWorkers, TaskCommanderPrivate::working));
        connect(&d->m_syncLine, &QFutureWatcherBase::finished, d, &TaskCommanderPrivate::onFinished);
    }

    return true;
}

void TaskCommander::stop()
{
    qDebug() << "stop" << this->taskID();
    d->m_asyncLine.cancel();
    d->m_syncLine.cancel();

    for (auto worker : d->m_allWorkers) {
        Q_ASSERT(worker);
        worker->terminate();
    }

    d->m_working = false;
}

GrandSearch::MatchedItemMap TaskCommander::getResults() const
{
    QReadLocker lk(&d->m_lock);
    return d->m_results;
}

GrandSearch::MatchedItemMap TaskCommander::readBuffer() const
{
    QWriteLocker lk(&d->m_lock);
    decltype(d->m_buffer) ret;
    ret = std::move(d->m_buffer);
    return ret;
}

bool TaskCommander::isEmptyBuffer() const
{
    QReadLocker lk(&d->m_lock);
    return d->m_buffer.isEmpty();
}

bool TaskCommander::join(ProxyWorker *worker)
{
    //已经开启任务不允许添加搜索项
    if (d->m_working)
        return false;

    Q_ASSERT(worker);
    worker->setParent(d);
    worker->setContext(d->m_content);
    d->m_allWorkers.append(worker);

    if (worker->isAsync())
        d->m_asyncWorkers.append(worker);
    else
        d->m_syncWorkers.append(worker);

    //直连，在线程处理
    connect(worker, &ProxyWorker::unearthed, d, &TaskCommanderPrivate::onUnearthed, Qt::DirectConnection);
    return true;
}

void TaskCommander::deleteSelf()
{
    //工作线程未完全退出时不执行释放，待退出后再释放
    if (d->m_asyncLine.isFinished() && d->m_syncLine.isFinished())
        delete this;
    else
        d->m_deleted = true;
}

