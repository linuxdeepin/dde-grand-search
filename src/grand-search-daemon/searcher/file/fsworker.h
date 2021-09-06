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
#ifndef FSWORKER_H
#define FSWORKER_H

#include "searcher/proxyworker.h"

extern "C" {
#include "fsearch.h"
}

class FsWorker : public ProxyWorker
{
    enum Group {Normal = 0, Folder, Picture, Audio, Video, Document, GroupCount, GroupBegin = Normal};
public:
    explicit FsWorker(const QString &name, QObject *parent = nullptr);

    void setContext(const QString &context) Q_DECL_OVERRIDE;
    bool isAsync() const Q_DECL_OVERRIDE;
    bool working(void *context) Q_DECL_OVERRIDE;
    void terminate() Q_DECL_OVERRIDE;
    Status status() Q_DECL_OVERRIDE;
    bool hasItem() const Q_DECL_OVERRIDE;
    GrandSearch::MatchedItemMap takeAll() Q_DECL_OVERRIDE;
    void setFsearchApp(FsearchApplication *app);
private:
    static void callbackReceiveResults(void *data, void *sender);
    bool appendSearchResult(const QString &fileName, bool isRecentFile = false);
    bool searchRecentFile();
    bool searchLocalFile();
    void tryNotify();
    int itemCount() const;
    QString groupKey(Group group) const;
    bool isResultLimit();
    Group getGroupByFileName(const QString &fileName);
private:
    FsearchApplication *m_app = nullptr;
    QAtomicInt m_status = Ready;
    QString m_context;

    //搜索结果
    mutable QMutex m_mtx;
    GrandSearch::MatchedItems m_items[GroupCount];  // 文件搜索
    QHash<Group, quint32> m_resultCountHash; // 记录各类型文件搜索结果数量

    QWaitCondition m_waitCondition;
    QMutex m_conditionMtx;
    QSet<QString> m_tmpSearchResults;     // 存储所有的搜索结果，用于去重
    QHash<QString, QSet<QString>> m_hiddenFilters;

    QTime m_time;       //搜索计时
    int m_lastEmit = 0;
};

#endif // FSWORKER_H
