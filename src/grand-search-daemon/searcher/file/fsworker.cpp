/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#include "fsworker.h"
#include "utils/utiltools.h"
#include "global/builtinsearch.h"

#include <QWaitCondition>

#define MAX_SEARCH_NUM 100

FsWorker::FsWorker(const QString &name, QObject *parent) : ProxyWorker(name, parent)
{
}

void FsWorker::setContext(const QString &context)
{
    if (context.isEmpty())
        qWarning() << "search key is empty.";
    m_context = context;
}

bool FsWorker::isAsync() const
{
    return false;
}

bool FsWorker::working(void *context)
{
    //准备状态切运行中，否则直接返回
    if (!m_status.testAndSetRelease(Ready, Runing))
        return false;

    Q_UNUSED(context);
    if (m_context.isEmpty() || !m_app) {
        m_status.storeRelease(Completed);
        return true;
    }

    db_search_results_clear(m_app->search);
    Database *db = m_app->db;
    if (!db_try_lock(db)) {
        m_status.storeRelease(Completed);
        return true;
    }

    if (m_app->search) {
        m_time.start();
        db_search_update(m_app->search,
                         db_get_entries(db),
                         db_get_num_entries(db),
                         UINT32_MAX,
                         FsearchFilter::FSEARCH_FILTER_NONE,
                         m_context.toStdString().c_str(),
                         m_app->config->hide_results_on_empty_search,
                         m_app->config->match_case,
                         m_app->config->enable_regex,
                         m_app->config->auto_search_in_path,
                         m_app->config->search_in_path);

        m_conditionMtx.lock();
        db_perform_search(m_app->search, callbackReceiveResults, m_app, this);
        m_waitCondition.wait(&m_conditionMtx);
        m_conditionMtx.unlock();
    }
    db_unlock(db);

    return true;
}

void FsWorker::terminate()
{
    m_status.storeRelease(Terminated);
}

ProxyWorker::Status FsWorker::status()
{
    return static_cast<ProxyWorker::Status>(m_status.loadAcquire());
}

bool FsWorker::hasItem() const
{
    QMutexLocker lk(&m_mtx);
    return !m_items.isEmpty();
}

GrandSearch::MatchedItemMap FsWorker::takeAll()
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

void FsWorker::setFsearchApp(FsearchApplication *app)
{
    //搜索中不允许设置, 已有数据后不允许更新
    if (m_status.loadAcquire() == Runing || m_app)
        return;

    m_app = app;
}

QString FsWorker::group() const
{
    return GRANDSEARCH_GROUP_FILE;
}

void FsWorker::callbackReceiveResults(void *data, void *sender)
{
    //计时
    int lastEmit = 0;

    DatabaseSearchResult *result = static_cast<DatabaseSearchResult *>(data);
    FsWorker *self = static_cast<FsWorker *>(sender);
    Q_ASSERT(result || self);

    if (self->m_app->search == nullptr) {
        self->m_status.storeRelease(Completed);
        self->m_conditionMtx.lock();
        self->m_waitCondition.wakeAll();
        self->m_conditionMtx.unlock();
        return;
    }

    uint32_t num_results = 0;
    if (result->results && result->results->len > 0) {
        num_results = result->results->len;
        for (uint32_t i = 0; i < num_results; ++i) {
            //中断
            if (self->m_status.loadAcquire() != Runing) {
                self->m_conditionMtx.lock();
                self->m_waitCondition.wakeAll();
                self->m_conditionMtx.unlock();
                return;
            }

            QString fileName;
            DatabaseSearchEntry *entry = static_cast<DatabaseSearchEntry *>(g_ptr_array_index(result->results, i));
            if (entry && entry->node) {
                auto *pNode = entry->node;
                while (pNode != nullptr) {
                    //中断
                    if (self->m_status.loadAcquire() != Runing) {
                        self->m_conditionMtx.lock();
                        self->m_waitCondition.wakeAll();
                        self->m_conditionMtx.unlock();
                        return;
                    }

                    if (pNode->name != nullptr) {
                        fileName.prepend(pNode->name);
                        if (pNode->parent && strcmp(pNode->name, "") != 0)
                            fileName.prepend("/");
                    }
                    pNode = pNode->parent;
                }
            }

            self->appendSearchResult(fileName);
            //50ms推送一次
            int cur = self->m_time.elapsed();
            if ((cur - lastEmit) > 50) {
                lastEmit = cur;
                qDebug() << "unearthed, current spend:" << cur;
                emit self->unearthed(self);
            }

            if (self->m_resultFileCount > MAX_SEARCH_NUM && self->m_resultFolderCount > MAX_SEARCH_NUM)
                break;
        }
    }

    self->m_status.storeRelease(Completed);

    int leave = 0;
    {
        QMutexLocker lk(&self->m_mtx);
        leave = self->m_items.count();
    }

    qInfo() << "search completed, found file items:" << self->m_resultFileCount
            << "folder items:" << self->m_resultFolderCount
            << "total spend:" << self->m_time.elapsed()
            << "current items" << leave;

    //发送数据
    if (leave > 0)
        emit self->unearthed(self);

    free(result);
    result = nullptr;

    self->m_conditionMtx.lock();
    self->m_waitCondition.wakeAll();
    self->m_conditionMtx.unlock();
}

void FsWorker::appendSearchResult(const QString &fileName)
{
    QFileInfo file(fileName);
    if (file.isDir()) {
        if (++m_resultFolderCount > MAX_SEARCH_NUM)
            return;
    } else if (++m_resultFileCount > MAX_SEARCH_NUM) {
        return;
    }

    QMimeType mimeType = GrandSearch::UtilTools::getMimeType(file);
    GrandSearch::MatchedItem item;
    item.item = fileName;
    item.name = file.fileName();
    item.type = mimeType.name();
    item.icon = mimeType.iconName();
    item.searcher = name();

    QMutexLocker lk(&m_mtx);
    m_items << item;
}
