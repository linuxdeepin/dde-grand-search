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
#include "filenameworker.h"
#include "filenameworkerprivate.h"
#include "anything_interface.h"

#include <QStandardPaths>

#define MAX_SEARCH_NUM 100

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

    QMimeType mimeType = m_mimeDB.mimeTypeForFile(fileName);

    GrandSearch::MatchedItem item;
    item.item = fileName;
    item.name = QFileInfo(fileName).fileName();
    item.type = mimeType.name();
    item.icon = mimeType.iconName();
    item.searcher = q->name();

    QMutexLocker lk(&m_mutex);
    m_items << item;
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

    //准备状态切运行中，否则直接返回
    if (!d->m_status.testAndSetRelease(Ready, Runing))
        return false;

    if (!d->m_anythingInterface->isValid() || d->m_context.isEmpty() || d->m_searchPath.isEmpty()) {
        d->m_status.storeRelease(Completed);
        return false;
    }

    if (!d->m_anythingInterface->hasLFT(d->m_searchPath)) {
        // 有可能 anything 不支持/home目录，但是支持/data/home
        if (QFile("/data/home").exists()) {
            d->m_searchPath.prepend("/data");
            if (!d->m_anythingInterface->hasLFT(d->m_searchPath)) {
                qWarning() << "Do not support quick search for " << d->m_searchPath;
                d->m_status.storeRelease(Completed);
                return false;
            }
        } else {
            qWarning() << "Data path is not exist!";
            d->m_status.storeRelease(Completed);
            return false;
        }
    }

    Q_UNUSED(context)
    //计时
    QTime time;
    time.start();
    int lastEmit = 0;

    quint16 resultCount = 0;
    QFileInfoList fileInfoList = d->traverseDirAndFile(d->m_searchPath);
    QStringList searchDirList;
    // 先对user目录下进行搜索
    for (const auto &info : fileInfoList) {
        //中断
        if (d->m_status.loadAcquire() != Runing)
            return false;

        if (info.isDir())
            searchDirList << info.absoluteFilePath();

        if (info.fileName().contains(d->m_context, Qt::CaseInsensitive)) {
            d->appendSearchResult(info.absoluteFilePath());

            //50ms推送一次
            int cur = time.elapsed();
            if ((cur - lastEmit) > 50) {
                lastEmit = cur;
                qDebug() << "unearthed, current spend:" << cur;
                emit unearthed(this);
            }

            if (++resultCount > MAX_SEARCH_NUM)
                break;
        }
    }

    int leave = 0;
    {
        QMutexLocker lk(&d->m_mutex);
        leave = d->m_items.count();
    }
    // 将user目录下的搜索结果推送
    if (leave > 0)
        emit unearthed(this);
    qInfo() << "normal search found items:" << resultCount << "total spend:" << time.elapsed()
            << "current items" << leave;
    time.restart();

    // 搜索
    quint32 searchStartOffset = 0;
    quint32 searchEndOffset = 0;
    // 过滤系统隐藏文件
    QRegExp hiddenFileFilter("^(?!.*/\\..*).+$");
    while (resultCount < MAX_SEARCH_NUM && !searchDirList.isEmpty()) {
        //中断
        if (d->m_status.loadAcquire() != Runing)
            return false;

        const auto result = d->m_anythingInterface->search(MAX_SEARCH_NUM, 100, searchStartOffset,
                                                           searchEndOffset, searchDirList.first(), d->m_context, false);
        QStringList searchResults = result.argumentAt<0>();
        searchResults = searchResults.filter(hiddenFileFilter);
        searchStartOffset = result.argumentAt<1>();
        searchEndOffset = result.argumentAt<2>();

        // 当前目录已经搜索到了结尾
        if (searchStartOffset >= searchEndOffset) {
            searchStartOffset = searchEndOffset = 0;
            searchDirList.removeAt(0);
        }

        for (const auto &path : searchResults) {
            //中断
            if (d->m_status.loadAcquire() != Runing)
                return false;

            if (resultCount >= MAX_SEARCH_NUM)
                break;

            d->appendSearchResult(path);
            //50ms推送一次
            int cur = time.elapsed();
            if ((cur - lastEmit) > 50) {
                lastEmit = cur;
                qDebug() << "unearthed, current spend:" << cur;
                emit unearthed(this);
            }
            ++resultCount;
        }
    }

    d->m_status.storeRelease(Completed);

    {
        QMutexLocker lk(&d->m_mutex);
        leave = d->m_items.count();
    }

    qInfo() << "search completed, found items:" << resultCount << "total spend:" << time.elapsed()
            << "current items" << leave;

    //还有数据再发一次
    if (leave > 0)
        emit unearthed(this);

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
    return tr("Files");
}
