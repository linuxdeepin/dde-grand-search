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

#include "maincontroller.h"
#include "maincontrollerprivate.h"

#include <QDebug>

MainControllerPrivate::MainControllerPrivate(MainController *parent)
    : QObject(parent)
    , q(parent)
{
}

MainControllerPrivate::~MainControllerPrivate()
{
    if (m_currentTask) {
        m_currentTask->stop();
        m_currentTask->deleteSelf();
        m_currentTask = nullptr;
    }
}

void MainControllerPrivate::buildWorker(TaskCommander *task)
{
    Q_ASSERT(task);
    Q_ASSERT(m_searchers);

    auto searchers = m_searchers->searchers();
    for (auto searcher : searchers) {
        Q_ASSERT(searcher);
        //判断搜索项是否激活，若未激活则先激活
        qDebug() << "searcher create worker" << searcher->name();
        if (searcher->isActive() || searcher->activate()) {
            if (auto worker = searcher->createWorker()) {
                task->join(worker);
            }
        }
    }
}

MainController::MainController(QObject *parent)
    : QObject(parent)
    , d(new MainControllerPrivate(this))
{
}

bool MainController::init()
{
    Q_ASSERT(d->m_searchers == nullptr);

    d->m_searchers = new SearcherGroup(this);
    return d->m_searchers->init();
}

bool MainController::newSearch(const QString &key)
{
    qInfo() << "new search, current task:" << d->m_currentTask << key.size();
    if (Q_UNLIKELY(key.isEmpty()))
        return false;

    //释放当前任务
    if (d->m_currentTask) {
        d->m_currentTask->stop();
        disconnect(d->m_currentTask, nullptr, this, nullptr);

        d->m_currentTask->deleteSelf();
        d->m_currentTask = nullptr;
    }

    auto task = new TaskCommander(key);
    qInfo() << "new task:" << task << task->taskID();
    Q_ASSERT(task);

    //直连，防止被事件循环打乱时序
    connect(task, &TaskCommander::matched, this, &MainController::matched, Qt::DirectConnection);
    connect(task, &TaskCommander::finished, this, &MainController::searchCompleted, Qt::DirectConnection);

    d->buildWorker(task);
    if (task->start()) {
        d->m_currentTask = task;
        return true;
    }

    qWarning() << "fail to start task" << task << task->taskID();
    task->deleteSelf();
    return false;
}

void MainController::terminate()
{
    qDebug() << __FUNCTION__;
    //只停止任务，不释放
    if (d->m_currentTask) {
        d->m_currentTask->stop();
        disconnect(d->m_currentTask, nullptr, this, nullptr);
    }
}

QByteArray MainController::getResults() const
{
    if (d->m_currentTask) {
        GrandSearch::MatchedItemMap items = d->m_currentTask->getResults();

        //序列化
        QByteArray bytes;
        QDataStream stream(&bytes, QIODevice::WriteOnly);
        stream << items;

        return bytes;
    }

    return QByteArray();
}

QByteArray MainController::readBuffer() const
{
    if (d->m_currentTask) {
        GrandSearch::MatchedItemMap items = d->m_currentTask->readBuffer();

        //序列化
        QByteArray bytes;
        QDataStream stream(&bytes, QIODevice::WriteOnly);
        stream << items;

        return bytes;
    }

    return QByteArray();
}

bool MainController::isEmptyBuffer() const
{
    if (d->m_currentTask) {
        return d->m_currentTask->isEmptyBuffer();
    }

    return true;
}

bool MainController::searcherAction(const QString &name, const QString &action, const QString &item)
{
    if (name.isEmpty() || action.isEmpty() || item.isEmpty()) {
        qWarning() << "input is empty.";
        return false;
    }

    if (auto searcher = d->m_searchers->searcher(name)) {
        return searcher->action(action, item);
    }

    qWarning() << "no such search:" << name;
    return false;
}
