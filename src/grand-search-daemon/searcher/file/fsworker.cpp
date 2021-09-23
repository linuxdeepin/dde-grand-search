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
     for (int i = GroupBegin; i < GroupCount; ++i)
        if (!m_items[i].isEmpty())
            return true;

    return false;
}

GrandSearch::MatchedItemMap FsWorker::takeAll()
{
    QMutexLocker lk(&m_mtx);
    //添加分组
    GrandSearch::MatchedItemMap ret;
    for (int i = GroupBegin; i < GroupCount; ++i) {
        if (!m_items[i].isEmpty()) {
            GrandSearch::MatchedItems items = std::move(m_items[i]);
            Q_ASSERT(m_items[i].isEmpty());
            ret.insert(groupKey(static_cast<FsWorker::Group>(i)), items);
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
        m_resultCountHash.insert(Folder, 0);

    if (config->value(GRANDSEARCH_GROUP_FILE, false))
        m_resultCountHash.insert(Normal, 0);

    if (config->value(GRANDSEARCH_GROUP_FILE_VIDEO, false))
        m_resultCountHash.insert(Video, 0);

    if (config->value(GRANDSEARCH_GROUP_FILE_AUDIO, false))
        m_resultCountHash.insert(Audio, 0);

    if (config->value(GRANDSEARCH_GROUP_FILE_PICTURE, false))
        m_resultCountHash.insert(Picture, 0);

    if (config->value(GRANDSEARCH_GROUP_FILE_DOCUMNET, false))
        m_resultCountHash.insert(Document, 0);
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
    for (int i = GroupBegin; i < GroupCount; ++i)
        count += m_items[i].size();

    return count;
}

QString FsWorker::groupKey(FsWorker::Group group) const
{
    switch (group) {
    case Folder:
        return GRANDSEARCH_GROUP_FOLDER;
    case Picture:
        return GRANDSEARCH_GROUP_FILE_PICTURE;
    case Audio:
        return GRANDSEARCH_GROUP_FILE_AUDIO;
    case Video:
        return GRANDSEARCH_GROUP_FILE_VIDEO;
    case Document:
        return GRANDSEARCH_GROUP_FILE_DOCUMNET;
    default:
        break;
    }
    return GRANDSEARCH_GROUP_FILE;
}

bool FsWorker::isResultLimit()
{
    const auto &iter = std::find_if(m_resultCountHash.begin(), m_resultCountHash.end(), [](const int &num){
        return num <= MAX_SEARCH_NUM;
    });

    return iter == m_resultCountHash.end();
}

FsWorker::Group FsWorker::getGroupByFileName(const QString &fileName)
{
    Group group = Normal;
    QFileInfo fileInfo(fileName);

    if (fileInfo.isDir()) {
        group = Folder;
    } else {
        // 文档格式
        static QRegExp docReg("^((pdf)|(txt)|(doc)|(docx)|(dot)|(dotx)|(ppt)|(pptx)|(pot)|(potx)"
                              "|(xls)|(xlsx)|(xlt)|(xltx)|(wps)|(wpt)|(rtf)|(md)|(latex))$");
        // 图片格式
        static QRegExp pictureReg("^((jpg)|(jpeg)|(jpe)|(bmp)|(png)|(gif)|(svg)|(tif)|(tiff))$");
        // 视频格式
        static QRegExp videoReg("^((avi)|(mov)|(mp4)|(mp2)|(mpa)|(mpg)|(mpeg)|(qt)|(rm)|(rmvb)"
                                "|(mkv)|(asx)|(asf)|(flv)|(3gp)|(mpe))$");
        // 音频格式
        static QRegExp musicReg("^((au)|(snd)|(mid)|(mp3)|(aif)|(aifc)|(aiff)|(m3u)|(ra)"
                                "|(ram)|(wav)|(cda)|(wma)|(ape))$");
        const auto &suffix = fileInfo.suffix();

        if (docReg.exactMatch(suffix)) {
            group = Document;
        } else if (pictureReg.exactMatch(suffix)) {
            group = Picture;
        } else if (videoReg.exactMatch(suffix)) {
            group = Video;
        } else if (musicReg.exactMatch(suffix)) {
            group = Audio;
        }
    }

    return group;
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

    auto group = getGroupByFileName(fileName);
    Q_ASSERT(group >= GroupBegin && group< GroupCount);

    // 根据搜索类目配置判断是否需要进行添加
    if (!m_resultCountHash.contains(group)) {
        if (group == Folder) {
            return false;
        }

        if (m_resultCountHash.contains(Normal)) {
            group = Normal;
        } else {
            return false;
        }
    }

    if (++m_resultCountHash[group] > MAX_SEARCH_NUM)
        return false;

    QFileInfo file(fileName);
    m_tmpSearchResults << fileName;
    QMimeType mimeType = GrandSearch::SpecialTools::getMimeType(file);
    GrandSearch::MatchedItem item;
    item.item = fileName;
    item.name = file.fileName();
    item.type = mimeType.name();
    item.icon = mimeType.iconName();
    item.searcher = name();

    // 最近使用文件需要置顶显示
    if (isRecentFile) {
        QVariantHash showLevelHash({{GRANDSEARCH_PROPERTY_ITEM_LEVEL, GRANDSEARCH_PROPERTY_ITEM_LEVEL_FIRST}});
        item.extra = QVariant::fromValue(showLevelHash);
    }

    QMutexLocker lk(&m_mtx);
    m_items[group].append(item);
    // 文档、音频、视频、图片需添加到文件组中
    if (group != Normal && m_resultCountHash.contains(Normal)) {
        if (group != Folder && m_resultCountHash[Normal] <= MAX_SEARCH_NUM) {
            m_items[Normal].append(item);
            ++m_resultCountHash[Normal];
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
