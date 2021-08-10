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
#include "filenameworker.h"
#include "filenameworkerprivate.h"
#include "anything_interface.h"
#include "global/builtinsearch.h"
#include "utils/utiltools.h"

#include <QStandardPaths>

#define MAX_SEARCH_NUM 100
#define EMIT_INTERVAL 50

FileNameWorkerPrivate::FileNameWorkerPrivate(FileNameWorker *qq)
    : q_ptr(qq)
{
    // 搜索目录为user
    QStringList homePaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (!homePaths.isEmpty())
        m_searchPath = homePaths.first();

    m_anythingInterface = new ComDeepinAnythingInterface("com.deepin.anything",
                                                         "/com/deepin/anything",
                                                         QDBusConnection::systemBus(),
                                                         qq);
    m_anythingInterface->setTimeout(1000);
}

QFileInfoList FileNameWorkerPrivate::traverseDirAndFile(const QString &path)
{
    QDir dir(path);
    if (!dir.exists())
        return {};

    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    auto result = dir.entryInfoList();
    // 排序
    qSort(result.begin(), result.end(), [](const QFileInfo &info1, const QFileInfo &info2) {
        static QStringList sortList{"Desktop", "Music", "Downloads", "Documents", "Pictures", "Videos"};
        int index1 = sortList.indexOf(info1.fileName());
        int index2 = sortList.indexOf(info2.fileName());

        if (index1 == -1)
            return false;
        else if (index2 == -1)
            return true;

        return index1 < index2;
    });

    return result;
}

void FileNameWorkerPrivate::appendSearchResult(const QString &fileName)
{
    Q_Q(FileNameWorker);

    if (m_tmpSearchResults.contains(fileName))
        return;

    QFileInfo file(fileName);
    if (file.isDir()) {
        if (++m_resultFolderCount > MAX_SEARCH_NUM)
            return;
    } else if (++m_resultFileCount > MAX_SEARCH_NUM) {
        return;
    }

    m_tmpSearchResults << fileName;
    QMimeType mimeType = GrandSearch::UtilTools::getMimeType(file);
    GrandSearch::MatchedItem item;
    item.item = fileName;
    item.name = file.fileName();
    item.type = mimeType.name();
    item.icon = mimeType.iconName();
    item.searcher = q->name();

    QMutexLocker lk(&m_mutex);
    m_items << item;
}

bool FileNameWorkerPrivate::searchRecentFile()
{
    Q_Q(FileNameWorker);

    // 搜索最近使用文件
    const auto &recentfiles = GrandSearch::UtilTools::getRecentlyUsedFiles();
    for (const auto &file : recentfiles) {
        //中断
        if (m_status.loadAcquire() != ProxyWorker::Runing)
            return false;

        QFileInfo info(file);
        if (info.fileName().contains(m_context, Qt::CaseInsensitive)) {
            appendSearchResult(file);

            //推送
            tryNotify();

            if (m_resultFileCount > MAX_SEARCH_NUM && m_resultFolderCount > MAX_SEARCH_NUM)
                break;
        }
    }

    int leave = itemCount();
    qInfo() << "recently-used search found file items:" << m_resultFileCount
            << "folder items:" << m_resultFolderCount
            << "total spend:" << m_time.elapsed()
            << "current items" << leave;

    return true;
}

bool FileNameWorkerPrivate::searchUserPath()
{
    Q_Q(FileNameWorker);

    QFileInfoList fileInfoList = traverseDirAndFile(m_searchPath);
    // 先对user目录下进行搜索
    for (const auto &info : fileInfoList) {
        //中断
        if (m_status.loadAcquire() != ProxyWorker::Runing)
            return false;

        if (info.isDir())
            m_searchDirList << info.absoluteFilePath();

        if (info.fileName().contains(m_context, Qt::CaseInsensitive)) {
            appendSearchResult(info.absoluteFilePath());

            //推送
            tryNotify();

            if (m_resultFileCount > MAX_SEARCH_NUM && m_resultFolderCount > MAX_SEARCH_NUM)
                break;
        }
    }

    int leave = itemCount();
    qInfo() << "user path search found file items:" << m_resultFileCount
            << "folder items:" << m_resultFolderCount
            << "total spend:" << m_time.elapsed()
            << "current items" << leave;

    return true;
}

bool FileNameWorkerPrivate::searchByAnything()
{
    Q_Q(FileNameWorker);

    // 搜索
    quint32 searchStartOffset = 0;
    quint32 searchEndOffset = 0;
    // 过滤系统隐藏文件
    QRegExp hiddenFileFilter("^(?!.*/\\..*).+$");
    while ((m_resultFileCount < MAX_SEARCH_NUM || m_resultFolderCount < MAX_SEARCH_NUM) && !m_searchDirList.isEmpty()) {
        //中断
        if (m_status.loadAcquire() != ProxyWorker::Runing)
            return false;

        const auto result = m_anythingInterface->search(100, 100, searchStartOffset,
                                                           searchEndOffset, m_searchDirList.first(),
                                                           m_context, false);
        QStringList searchResults = result.argumentAt<0>();
        searchResults = searchResults.filter(hiddenFileFilter);
        searchStartOffset = result.argumentAt<1>();
        searchEndOffset = result.argumentAt<2>();

        // 当前目录已经搜索到了结尾
        if (searchStartOffset >= searchEndOffset) {
            searchStartOffset = searchEndOffset = 0;
            m_searchDirList.removeAt(0);
        }

        for (auto &path : searchResults) {
            //中断
            if (m_status.loadAcquire() != ProxyWorker::Runing)
                return false;

            // 去除掉添加的data前缀
            if (m_hasAddDataPrefix && path.startsWith("/data"))
                path = path.mid(5);

            appendSearchResult(path);

            //推送
            tryNotify();

            if (m_resultFileCount > MAX_SEARCH_NUM && m_resultFolderCount > MAX_SEARCH_NUM)
                break;
        }
    }

    int leave = itemCount();
    qInfo() << "anything search completed, found file items:" << m_resultFileCount
            << "folder items:" << m_resultFolderCount
            << "total spend:" << m_time.elapsed()
            << "current items" << leave;

    return true;
}

void FileNameWorkerPrivate::tryNotify()
{
    Q_Q(FileNameWorker);
    int cur = m_time.elapsed();
    if (q->hasItem() && (cur - m_lastEmit) > EMIT_INTERVAL) {
        m_lastEmit = cur;
        qDebug() << "unearthed, current spend:" << cur;
        emit q->unearthed(q);
    }
}

int FileNameWorkerPrivate::itemCount() const
{
    QMutexLocker lk(&m_mutex);
    return m_items.count();
}

FileNameWorker::FileNameWorker(const QString &name, QObject *parent)
    : ProxyWorker(name, parent),
      d_ptr(new FileNameWorkerPrivate(this))
{

}

void FileNameWorker::setContext(const QString &context)
{
    Q_D(FileNameWorker);

    if (context.isEmpty())
        qWarning() << "search key is empty.";
    d->m_context = context;
}

bool FileNameWorker::isAsync() const
{
    return false;
}

bool FileNameWorker::working(void *context)
{
    Q_D(FileNameWorker);
    Q_UNUSED(context)

    //准备状态切运行中，否则直接返回
    if (!d->m_status.testAndSetRelease(Ready, Runing))
        return false;

    if (!d->m_anythingInterface->isValid() || d->m_context.isEmpty() || d->m_searchPath.isEmpty()) {
        d->m_status.storeRelease(Completed);
        return false;
    }

    d->m_time.start();

    //检查home路径
    bool useAnything = true;
    if (!d->m_anythingInterface->hasLFT(d->m_searchPath)) {
        // 有可能 anything 不支持/home目录，但是支持/data/home
        if (QFile("/data/home").exists()) {
            d->m_searchPath.prepend("/data");
            if (!d->m_anythingInterface->hasLFT(d->m_searchPath)) {
                qWarning() << "Do not support quick search for " << d->m_searchPath;
                useAnything = false;
            } else {
                d->m_hasAddDataPrefix = true;
            }
        } else {
            qWarning() << "Data path is not exist!";
            useAnything = false;
        }
    }

    // 搜索最新使用文件
    if (!d->searchRecentFile())
        return false; //中断

    // 搜索user目录下文件
    if (!d->searchUserPath())
        return false; //中断

    // 使用anything搜索
    if (useAnything) {
        if (!d->searchByAnything())
            return false; //中断
    }

    //检查是否还有数据
    if (d->m_status.testAndSetRelease(Runing, Completed)) {
        //发送数据
        if (hasItem())
            emit unearthed(this);
    }
    return true;
}

void FileNameWorker::terminate()
{
    Q_D(FileNameWorker);

    d->m_status.storeRelease(Terminated);
}

ProxyWorker::Status FileNameWorker::status()
{
    Q_D(FileNameWorker);

    return static_cast<ProxyWorker::Status>(d->m_status.loadAcquire());
}

bool FileNameWorker::hasItem() const
{
    Q_D(const FileNameWorker);

    QMutexLocker lk(&d->m_mutex);
    return !d->m_items.isEmpty();
}

GrandSearch::MatchedItemMap FileNameWorker::takeAll()
{
    Q_D(FileNameWorker);

    QMutexLocker lk(&d->m_mutex);
    GrandSearch::MatchedItems items = std::move(d->m_items);

    Q_ASSERT(d->m_items.isEmpty());
    lk.unlock();

    //添加分组
    GrandSearch::MatchedItemMap ret;
    ret.insert(group(), items);

    return ret;
}

QString FileNameWorker::group() const
{
    return GRANDSEARCH_GROUP_FILE;
}

