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
#ifndef FILENAMEWORKER_P_H
#define FILENAMEWORKER_P_H

#include "anything_interface.h"
#include "searcher/proxyworker.h"

class FileNameWorker;
class FileNameWorkerPrivate
{
    enum Group {Normal = 0, Folder, Picture, Audio, Video, Document, GroupCount, GroupBegin = Normal};
public:
    explicit FileNameWorkerPrivate(FileNameWorker *qq);
    void initConfig();
    void initAnything();
    QFileInfoList traverseDirAndFile(const QString &path);
    bool sortFileName(const QFileInfo &info1, const QFileInfo &info2);
    bool appendSearchResult(const QString &fileName, bool isRecentFile = false);

    bool searchRecentFile();
    bool searchUserPath();
    bool searchByAnything();

    void tryNotify();
    int itemCount() const;
    QString groupKey(Group group) const;
    // 判断所有类目的搜索结果是否达到限制数量
    bool isResultLimit();
    Group getGroupByFileName(const QString &fileName);
public:
    FileNameWorker *q_ptr = nullptr;
    QAtomicInt m_status = ProxyWorker::Ready;
    QString m_searchPath;
    QString m_context;                  // 搜索关键字
    QHash<Group, quint32> m_resultCountHash; // 记录各类型文件搜索结果数量

    mutable QMutex m_mutex;
    GrandSearch::MatchedItems m_items[GroupCount];  // 文件搜索
    ComDeepinAnythingInterface *m_anythingInterface = nullptr;
    QStringList m_searchDirList;
    QSet<QString> m_tmpSearchResults;     // 存储所有的搜索结果，用于去重
    bool m_hasAddDataPrefix = false;
    QHash<QString, QSet<QString>> m_hiddenFilters;

    //计时
    QTime m_time;
    int m_lastEmit = 0;

    Q_DECLARE_PUBLIC(FileNameWorker)
};

#endif // FILENAMEWORKER_P_H
