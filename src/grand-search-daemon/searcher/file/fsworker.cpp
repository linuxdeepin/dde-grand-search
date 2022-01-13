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
#include "utils/specialtools.h"
#include "global/builtinsearch.h"
#include "configuration/configer.h"

#include <QWaitCondition>

#define MAX_SEARCH_NUM 100
#define EMIT_INTERVAL 50

FsWorker::FsWorker(const QString &name, QObject *parent) : ProxyWorker(name, parent)
{
    initConfig();
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

    m_time.start();

    // 搜索最近使用文件
    if (!searchRecentFile())
        return false;

    if (!searchLocalFile())
        return false;

    //检查是否还有数据
    if (m_status.testAndSetRelease(Runing, Completed)) {
        //发送数据
        if (hasItem())
            emit unearthed(this);
    }

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
     for (int i = FileSearchUtils::GroupBegin; i < FileSearchUtils::GroupCount; ++i)
        if (!m_items[i].isEmpty())
            return true;

    return false;
}

GrandSearch::MatchedItemMap FsWorker::takeAll()
{
    QMutexLocker lk(&m_mtx);
    //添加分组
    GrandSearch::MatchedItemMap ret;
    for (int i = FileSearchUtils::GroupBegin; i < FileSearchUtils::GroupCount; ++i) {
        if (!m_items[i].isEmpty()) {
            GrandSearch::MatchedItems items = std::move(m_items[i]);
            Q_ASSERT(m_items[i].isEmpty());
            ret.insert(FileSearchUtils::groupKey(static_cast<FileSearchUtils::Group>(i)), items);
        }
    }
    lk.unlock();

    return ret;
}

void FsWorker::setFsearchApp(FsearchApplication *app)
{
    //搜索中不允许设置, 已有数据后不允许更新
    if (m_status.loadAcquire() == Runing || m_app)
        return;

    m_app = app;
}

void FsWorker::initConfig()
{
    // 获取支持的搜索类目
    auto config = Configer::instance()->group(GRANDSEARCH_CLASS_FILE_FSEARCH);
    if (config->value(GRANDSEARCH_GROUP_FOLDER, false))
        m_resultCountHash.insert(FileSearchUtils::Folder, 0);

    if (config->value(GRANDSEARCH_GROUP_FILE, false))
        m_resultCountHash.insert(FileSearchUtils::File, 0);

    if (config->value(GRANDSEARCH_GROUP_FILE_VIDEO, false))
        m_resultCountHash.insert(FileSearchUtils::Video, 0);

    if (config->value(GRANDSEARCH_GROUP_FILE_AUDIO, false))
        m_resultCountHash.insert(FileSearchUtils::Audio, 0);

    if (config->value(GRANDSEARCH_GROUP_FILE_PICTURE, false))
        m_resultCountHash.insert(FileSearchUtils::Picture, 0);

    if (config->value(GRANDSEARCH_GROUP_FILE_DOCUMNET, false))
        m_resultCountHash.insert(FileSearchUtils::Document, 0);
}

void FsWorker::tryNotify()
{
    //50ms推送一次
    int cur = m_time.elapsed();
    if (hasItem() && (cur - m_lastEmit) > EMIT_INTERVAL) {
        m_lastEmit = cur;
        qDebug() << "unearthed, current spend:" << cur;
        emit unearthed(this);
    }
}

int FsWorker::itemCount() const
{
    QMutexLocker lk(&m_mtx);
    int count = 0;
    for (int i = FileSearchUtils::GroupBegin; i < FileSearchUtils::GroupCount; ++i)
        count += m_items[i].size();

    return count;
}

bool FsWorker::isResultLimit()
{
    const auto &iter = std::find_if(m_resultCountHash.begin(), m_resultCountHash.end(), [](const int &num){
        return num <= MAX_SEARCH_NUM;
    });

    return iter == m_resultCountHash.end();
}

void FsWorker::callbackReceiveResults(void *data, void *sender)
{
    DatabaseSearchResult *result = static_cast<DatabaseSearchResult *>(data);
    FsWorker *self = static_cast<FsWorker *>(sender);
    Q_ASSERT(result && self);

    if (self->m_app->search == nullptr) {
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

            // 过滤文管设置的隐藏文件
            if (GrandSearch::SpecialTools::isHiddenFile(fileName, self->m_hiddenFilters, QDir::homePath()))
                continue;

            self->appendSearchResult(fileName);

            //推送
            self->tryNotify();

            if (self->isResultLimit())
                break;
        }
    }

    int leave = self->itemCount();
    qInfo() << "search completed, found items:" << self->m_resultCountHash
            << "total spend:" << self->m_time.elapsed()
            << "current items" << leave;

    self->m_conditionMtx.lock();
    self->m_waitCondition.wakeAll();
    self->m_conditionMtx.unlock();
}

bool FsWorker::appendSearchResult(const QString &fileName, bool isRecentFile)
{
    if (m_tmpSearchResults.contains(fileName))
        return false;

    auto group = FileSearchUtils::getGroupByName(fileName);
    Q_ASSERT(group >= FileSearchUtils::GroupBegin && group< FileSearchUtils::GroupCount);

    // 根据搜索类目配置判断是否需要进行添加
    if (!m_resultCountHash.contains(group)) {
        if (group == FileSearchUtils::Folder) {
            return false;
        }

        if (m_resultCountHash.contains(FileSearchUtils::File)) {
            group = FileSearchUtils::File;
        } else {
            return false;
        }
    }

    if (++m_resultCountHash[group] > MAX_SEARCH_NUM)
        return false;

    m_tmpSearchResults << fileName;
    const auto &item = FileSearchUtils::packItem(fileName, name(), isRecentFile);

    QMutexLocker lk(&m_mtx);
    m_items[group].append(item);
    // 文档、音频、视频、图片需添加到文件组中
    if (group != FileSearchUtils::File && m_resultCountHash.contains(FileSearchUtils::File)) {
        if (group != FileSearchUtils::Folder && m_resultCountHash[FileSearchUtils::File] <= MAX_SEARCH_NUM) {
            m_items[FileSearchUtils::File].append(item);
            ++m_resultCountHash[FileSearchUtils::File];
        }
    }

    return true;
}

bool FsWorker::searchRecentFile()
{
    // 搜索最近使用文件
    const auto &recentfiles = GrandSearch::SpecialTools::getRecentlyUsedFiles();
    for (const auto &file : recentfiles) {
        //中断
        if (m_status.loadAcquire() != ProxyWorker::Runing)
            return false;

        QFileInfo info(file);
        if (info.fileName().contains(m_context, Qt::CaseInsensitive)) {
            appendSearchResult(file, true);

            //推送
            tryNotify();

            if (isResultLimit())
                break;
        }
    }

    int leave = itemCount();
    qInfo() << "recently-used search found file items:" << m_resultCountHash
            << "total spend:" << m_time.elapsed()
            << "current items" << leave;

    return true;
}

bool FsWorker::searchLocalFile()
{
    db_search_results_clear(m_app->search);
    Database *db = m_app->db;
    if (!db_try_lock(db)) {
        return true;
    }

    if (m_app->search) {
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
