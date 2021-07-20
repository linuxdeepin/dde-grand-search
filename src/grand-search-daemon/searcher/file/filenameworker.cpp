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
#include "anything_interface.h"

#include <QStandardPaths>
#include <QRegExp>

#define MAX_SEARCH_NUM 100

FileNameWorker::FileNameWorker(const QString &name, QObject *parent) : ProxyWorker(name, parent)
{
    // 搜索目录为user
    QStringList homePaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (!homePaths.isEmpty())
        m_searchPath = homePaths.first();

    anythingInterface = new ComDeepinAnythingInterface("com.deepin.anything",
                                                       "/com/deepin/anything",
                                                       QDBusConnection::systemBus(),
                                                       this);
    anythingInterface->setTimeout(1000);
}

void FileNameWorker::setContext(const QString &context)
{
    if (context.isEmpty())
        qWarning() << "search key is empty.";
    m_context = context;
}

bool FileNameWorker::isAsync() const
{
    return false;
}

bool FileNameWorker::working(void *context)
{
    //准备状态切运行中，否则直接返回
    if (!m_status.testAndSetRelease(Ready, Runing))
        return false;

    if (!anythingInterface->isValid() || m_context.isEmpty() || m_searchPath.isEmpty()) {
        m_status.storeRelease(Completed);
        return false;
    }

    if (!anythingInterface->hasLFT(m_searchPath)) {
        // 有可能 anything 不支持/home目录，但是支持/data/home
        if (QFile("/data/home").exists()) {
            m_searchPath.prepend("/data");
            if (!anythingInterface->hasLFT(m_searchPath)) {
                qWarning() << "Do not support quick search for " << m_searchPath;
                m_status.storeRelease(Completed);
                return false;
            }
            m_isAddDataPrefix = true;
            m_regExpFlag.setPattern("^/data");
        } else {
            qWarning() << "Data path is not exist!";
            m_status.storeRelease(Completed);
            return false;
        }
    }

    Q_UNUSED(context)
    // 如果挂载在此路径下的其它目录也支持索引数据, 则一并搜索
    QStringList searchDirList;
    searchDirList << anythingInterface->hasLFTSubdirectories(m_searchPath);
    if (searchDirList.isEmpty() || searchDirList.first() != m_searchPath)
        searchDirList.prepend(m_searchPath);

    //计时
    QTime time;
    time.start();

    // 搜索
    quint32 searchStartOffset = 0;
    quint32 searchEndOffset = 0;
    quint16 resultCount = 0;
    while (resultCount < MAX_SEARCH_NUM && !searchDirList.isEmpty()) {
        //中断
        if (m_status.loadAcquire() != Runing)
            return false;

        const auto result = anythingInterface->search(MAX_SEARCH_NUM, 100, searchStartOffset,
                                                      searchEndOffset, searchDirList.first(), m_context, false);
        QStringList searchResults = result.argumentAt<0>();
        searchStartOffset = result.argumentAt<1>();
        searchEndOffset = result.argumentAt<2>();

        // 当前目录已经搜索到了结尾
        if (searchStartOffset >= searchEndOffset) {
            searchStartOffset = searchEndOffset = 0;
            searchDirList.removeAt(0);
        }

        for (auto &path : searchResults) {
            //中断
            if (m_status.loadAcquire() != Runing)
                return false;

            if (resultCount >= MAX_SEARCH_NUM)
                break;

            // 过滤隐藏文件
            QFileInfo fileInfo(path);
            if (fileInfo.isHidden())
                continue;

            // 删除添加的/data前缀
            if (m_isAddDataPrefix)
                path.remove(m_regExpFlag);

            GrandSearch::MatchedItem item;
            QFileInfo file(path);
            item.item = path;
            item.name = file.fileName();

            QMutexLocker lk(&m_mutex);
            m_items.append(item);
            ++resultCount;
        }
    }

    qInfo() << "search completed, found items:" << resultCount << "total spend:" << time.elapsed()
            << "current items" << m_items.count();

    emit unearthed(this);
    m_status.storeRelease(Completed);

    return true;
}

void FileNameWorker::terminate()
{
    m_status.storeRelease(Terminated);
}

ProxyWorker::Status FileNameWorker::status()
{
    return static_cast<ProxyWorker::Status>(m_status.loadAcquire());
}

bool FileNameWorker::hasItem() const
{
    QMutexLocker lk(&m_mutex);
    return !m_items.isEmpty();
}

GrandSearch::MatchedItemMap FileNameWorker::takeAll()
{
    QMutexLocker lk(&m_mutex);
    GrandSearch::MatchedItems items = std::move(m_items);

    Q_ASSERT(m_items.isEmpty());
    lk.unlock();

    //添加分组
    GrandSearch::MatchedItemMap ret;
    ret.insert(group(),items);

    return ret;
}

QString FileNameWorker::group() const
{
    return tr("filename");
}
