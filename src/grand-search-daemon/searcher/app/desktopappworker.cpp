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
#include "desktopappworker.h"

DesktopAppWorker::DesktopAppWorker(const QString &name, QObject *parent) : ProxyWorker(name, parent)
{

}

void DesktopAppWorker::setContext(const QString &context)
{
    if (context.isEmpty())
        qWarning() << "search key is empty.";
    m_context = context;
}

bool DesktopAppWorker::isAsync() const
{
    //同步搜索，需分配线程资源
    return false;
}

bool DesktopAppWorker::working(void *context)
{
    //准备状态切运行中，否则直接返回
    if (!m_status.testAndSetRelease(Ready, Runing))
        return false;

    Q_UNUSED(context);
    if (m_context.isEmpty() || m_indexTable.isEmpty()) {
        m_status.storeRelease(Completed);
        return true;
    }

    //计时
    QTime time;
    time.start();
    int lastEmit = 0;

    //遍历查找
    QHash<GrandSearch::MatchedItem *, bool> found;
    for (auto iter = m_indexTable.begin(); iter != m_indexTable.end(); ++iter) {
        //中断
        if (m_status.loadAcquire() != Runing)
            return false;

        //匹配
        if (iter.key().contains(m_context, Qt::CaseInsensitive)) {
            //遍历该关键词匹配的项目检查是否已经被添加
            for (const QSharedPointer<GrandSearch::MatchedItem> &item : iter.value()) {
                //检查是否已经添加过
                if (found.contains(item.data()))
                    continue;

                //中断
                if (m_status.loadAcquire() != Runing)
                    return false;

                //加入搜索结果中
                {
                    QMutexLocker lk(&m_mtx);
                    m_items.append(*item);
                    found.insert(item.data(), true);

                    //数据从无到有发送通知
                    //50ms推送一次
                    int cur = time.elapsed();
                    if ((cur - lastEmit) > 50) {
                        lastEmit = cur;
                        qDebug() << "unearthed, current spend:" << cur;

                        //发送信号前必须解锁
                        lk.unlock();
                        emit unearthed(this);
                    }
                }
            }
        }
    }

    m_status.storeRelease(Completed);

    int leave = 0;
    {
        QMutexLocker lk(&m_mtx);
        leave = m_items.size();
    }

    qInfo() << "search completed, found items:" << found.size() << "total spend:" << time.elapsed()
            << "current items" << leave;
    //还有数据再发一次
    if (leave > 0)
        emit unearthed(this);

    return true;
}

void DesktopAppWorker::terminate()
{
    m_status.storeRelease(Terminated);
}

ProxyWorker::Status DesktopAppWorker::status()
{
    return static_cast<ProxyWorker::Status>(m_status.loadAcquire());
}

bool DesktopAppWorker::hasItem() const
{
    QMutexLocker lk(&m_mtx);
    return !m_items.isEmpty();
}

GrandSearch::MatchedItemMap DesktopAppWorker::takeAll()
{
    QMutexLocker lk(&m_mtx);
    GrandSearch::MatchedItems items = std::move(m_items);

    Q_ASSERT(m_items.isEmpty());
    lk.unlock();

    //添加分组
    GrandSearch::MatchedItemMap ret;
    ret.insert(group(),items);

    return ret;
}

void DesktopAppWorker::setIndexTable(const QHash<QString, QList<QSharedPointer<GrandSearch::MatchedItem> > > &table)
{
    qDebug() << "index table count" << table.size();
    //搜索中不允许设置索引表, 已有数据后不允许更新
    if (m_status.loadAcquire() == Runing || !m_indexTable.isEmpty())
        return;

    m_indexTable = table;
}

QString DesktopAppWorker::group() const
{
    return tr("Application");
}
