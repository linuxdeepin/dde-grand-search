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
#include "extendworker.h"

#include "searchplugin/pluginliaison.h"

ExtendWorker::ExtendWorker(const QString &name, QObject *parent)
    : ProxyWorker(name, parent)
{
    m_delayTimer.setSingleShot(true);
    connect(&m_delayTimer, &QTimer::timeout, this, &ExtendWorker::tryWorking);
}

ExtendWorker::~ExtendWorker()
{

}

bool ExtendWorker::setService(const QString &service, const QString &address, const QString &interface, const QString &ver)
{
    if (Q_UNLIKELY(address.isEmpty()
            || service.isEmpty() || interface.isEmpty()
            || ver.isEmpty()))
        return false;

    if (m_liaison)
        return false;

    //创建通信模块
    auto liaison = new PluginLiaison(this);
    if (!liaison->init(service, address, interface, ver, name())) {
        qWarning() << "fail to create PluginLiaison: " << service << address << interface << ver;
        delete liaison;
        return false;
    }

    m_liaison = liaison;

    //插件返回数据解析完毕，使用队列切入主线程，信号会来自子线程，也会来自主线程
    connect(m_liaison, &PluginLiaison::searchFinished, this, &ExtendWorker::onWorkFinished,Qt::QueuedConnection);
    return true;
}

void ExtendWorker::setContext(const QString &context)
{
    m_context = context;
}

bool ExtendWorker::isAsync() const
{
    return true;
}

bool ExtendWorker::working(void *context)
{
    //准备状态切运行中，否则直接返回
    if (!m_status.testAndSetRelease(Ready, Runing))
        return false;

    QString id = *static_cast<QString *>(context);
    if (!m_liaison || id.isEmpty() || m_context.isEmpty()) {
        m_status.storeRelease(Completed);
        return false;
    }

    m_taskID = id;
    //直接调用接口
    if (m_liaison->isVaild()) {
        if (m_liaison->search(m_taskID, m_context)) {
            return true;
        } else {
            m_status.storeRelease(Completed);
            return false;
        }
    } else {
        delayWork();
    }

    return true;
}

void ExtendWorker::terminate()
{
    m_delayTimer.stop();
    m_status.storeRelease(Terminated);

    if (m_liaison)
        m_liaison->stop(m_taskID);
}

ProxyWorker::Status ExtendWorker::status()
{
    return static_cast<ProxyWorker::Status>(m_status.loadAcquire());
}

bool ExtendWorker::hasItem() const
{
    QMutexLocker lk(&m_mtx);
    return !m_items.isEmpty();
}

GrandSearch::MatchedItemMap ExtendWorker::takeAll()
{
    QMutexLocker lk(&m_mtx);
    GrandSearch::MatchedItemMap items = std::move(m_items);

    Q_ASSERT(m_items.isEmpty());
    lk.unlock();

    return items;
}

void ExtendWorker::delayWork()
{
    m_delayTimer.stop();
    m_retryCount++;

    //尝试大于3次，放弃
    if (m_retryCount > 3) {
        m_status.storeRelease(Completed);

        //发送结束
        emit asyncFinished(this);
        return;
    }

    //重试间隔
    static int RetryInterval[] = {1, 2000, 5000, 10000, INT32_MAX};

    //回到主线程开启定时器
    QMetaObject::invokeMethod(&m_delayTimer, "start",
                              Qt::QueuedConnection, Q_ARG(int, RetryInterval[m_retryCount]));
}

void ExtendWorker::tryWorking()
{
    //! 由timer触发，在主线程执行
    //插件接口是否有效
    if (m_liaison->isVaild()) {
        //有效直接调用
        if (!m_liaison->search(m_taskID, m_context)) {
            //发起搜索失败
            m_status.storeRelease(Completed);

            //发送结束信号
            emit asyncFinished(this);
            return;
        }
    } else {
        //无效，再次等待
        delayWork();
    }
}

void ExtendWorker::onWorkFinished(const GrandSearch::MatchedItemMap &ret)
{
    QMutexLocker lk(&m_mtx);
    m_items = ret;
    lk.unlock();

    if (!ret.isEmpty())
        emit unearthed(this);

    qDebug() << name() << "work finished" << ret.size();
    //异步搜索结束
    emit asyncFinished(this);
}
