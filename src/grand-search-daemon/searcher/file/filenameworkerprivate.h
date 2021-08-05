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
#ifndef FILENAMEWORKERPRIVATE_H
#define FILENAMEWORKERPRIVATE_H

#include "anything_interface.h"
#include "searcher/proxyworker.h"

class FileNameWorker;
class FileNameWorkerPrivate
{
public:
    explicit FileNameWorkerPrivate(FileNameWorker *qq);
    QFileInfoList traverseDirAndFile(const QString &path);
    bool sortFileName(const QFileInfo &info1, const QFileInfo &info2);
    void appendSearchResult(const QString &fileName);

public:
    FileNameWorker *q_ptr = nullptr;
    QAtomicInt m_status = ProxyWorker::Ready;
    QString m_searchPath;
    QString m_context;                  // 搜索关键字

    mutable QMutex m_mutex;
    GrandSearch::MatchedItems m_items;  // 搜索结果
    ComDeepinAnythingInterface *m_anythingInterface = nullptr;

    Q_DECLARE_PUBLIC(FileNameWorker)
};

#endif // FILENAMEWORKERPRIVATE_H
