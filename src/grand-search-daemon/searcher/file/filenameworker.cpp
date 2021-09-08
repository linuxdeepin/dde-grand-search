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
#include "filenameworker_p.h"
#include "anything_interface.h"
#include "global/builtinsearch.h"
#include "utils/specialtools.h"
#include "configuration/configer.h"

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

    initAnything();
    initConfig();
}

void FileNameWorkerPrivate::initConfig()
{
    // 获取支持的搜索类目
    auto config = Configer::instance()->group(GRANDSEARCH_CLASS_FILE_DEEPIN);
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

void FileNameWorkerPrivate::initAnything()
{
    Q_Q(FileNameWorker);

    m_anythingInterface = new ComDeepinAnythingInterface("com.deepin.anything",
                                                         "/com/deepin/anything",
                                                         QDBusConnection::systemBus(),
                                                         q);
    m_anythingInterface->setTimeout(1000);

    // 自动索引内置磁盘
    if (!m_anythingInterface->autoIndexInternal())
        m_anythingInterface->setAutoIndexInternal(true);
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

bool FileNameWorkerPrivate::appendSearchResult(const QString &fileName, bool isRecentFile)
{
    Q_Q(FileNameWorker);

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
    item.searcher = q->name();

    // 最近使用文件需要置顶显示
    if (isRecentFile) {
        QVariantHash showLevelHash({{GRANDSEARCH_PROPERTY_ITEM_LEVEL, GRANDSEARCH_PROPERTY_ITEM_LEVEL_FIRST}});
        item.extra = QVariant::fromValue(showLevelHash);
    }

    QMutexLocker lk(&m_mutex);
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

bool FileNameWorkerPrivate::searchRecentFile()
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
    qInfo() << "recently-used search found items:" << m_resultCountHash
            << "total spend:" << m_time.elapsed()
            << "current items" << leave;

    return true;
}

bool FileNameWorkerPrivate::searchUserPath()
{
    QFileInfoList fileInfoList = traverseDirAndFile(m_searchPath);
    // 先对user目录下进行搜索
    for (const auto &info : fileInfoList) {
        //中断
        if (m_status.loadAcquire() != ProxyWorker::Runing)
            return false;

        if (info.isDir())
            m_searchDirList << info.absoluteFilePath();

        if (info.fileName().contains(m_context, Qt::CaseInsensitive)) {
            const auto &absoluteFilePath = info.absoluteFilePath();

            // 过滤文管设置的隐藏文件
            if (GrandSearch::SpecialTools::isHiddenFile(absoluteFilePath, m_hiddenFilters, QDir::homePath()))
                continue;

            appendSearchResult(info.absoluteFilePath());

            //推送
            tryNotify();

            if (isResultLimit())
                break;
        }
    }

    int leave = itemCount();
    qInfo() << "user path search found items:" << m_resultCountHash
            << "total spend:" << m_time.elapsed()
            << "current items" << leave;

    return true;
}

bool FileNameWorkerPrivate::searchByAnything()
{
    // 搜索
    quint32 searchStartOffset = 0;
    quint32 searchEndOffset = 0;
    // 过滤系统隐藏文件
    QRegExp hiddenFileFilter("^(?!.*/\\..*).+$");
    while (!isResultLimit() && !m_searchDirList.isEmpty()) {
        //中断
        if (m_status.loadAcquire() != ProxyWorker::Runing)
            return false;

        const auto result = m_anythingInterface->search(100, 100, searchStartOffset,
                                                           searchEndOffset, m_searchDirList.first(),
                                                           m_context, false);
        // fix bug 93806
        // 直接判断errorType为NoError，需要先取值再判断
        QStringList searchResults = result.argumentAt<0>();
        if (result.error().type() != QDBusError::NoError) {
            qWarning() << "deepin-anything search failed:"
                       << QDBusError::errorString(result.error().type())
                       << result.error().message();
            searchStartOffset = searchEndOffset = 0;
            m_searchDirList.removeAt(0);
            continue;
        }

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

            // 过滤文管设置的隐藏文件
            if (GrandSearch::SpecialTools::isHiddenFile(path, m_hiddenFilters, QDir::homePath()))
                continue;

            appendSearchResult(path);

            //推送
            tryNotify();

            if (isResultLimit())
                break;
        }
    }

    int leave = itemCount();
    qInfo() << "anything search completed, found items:" << m_resultCountHash
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

    int count = 0;
    for (int i = GroupBegin; i < GroupCount; ++i)
        count += m_items[i].size();

    return count;
}

QString FileNameWorkerPrivate::groupKey(FileNameWorkerPrivate::Group group) const
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

bool FileNameWorkerPrivate::isResultLimit()
{
    const auto &iter = std::find_if(m_resultCountHash.begin(), m_resultCountHash.end(), [](const int &num){
        return num <= MAX_SEARCH_NUM;
    });

    return iter == m_resultCountHash.end();
}

FileNameWorkerPrivate::Group FileNameWorkerPrivate::getGroupByFileName(const QString &fileName)
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
                                "|(mkv)|(asx)|(asf)|(flv)|(3gp))$");
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
    for (int i = FileNameWorkerPrivate::GroupBegin; i < FileNameWorkerPrivate::GroupCount; ++i)
        if (!d->m_items[i].isEmpty())
            return true;

    return false;
}

GrandSearch::MatchedItemMap FileNameWorker::takeAll()
{
    Q_D(FileNameWorker);

    //添加分组
    GrandSearch::MatchedItemMap ret;

    QMutexLocker lk(&d->m_mutex);
    for (int i = FileNameWorkerPrivate::GroupBegin; i < FileNameWorkerPrivate::GroupCount; ++i) {
        if (!d->m_items[i].isEmpty()) {
            GrandSearch::MatchedItems items = std::move(d->m_items[i]);
            Q_ASSERT(d->m_items[i].isEmpty());
            ret.insert(d->groupKey(static_cast<FileNameWorkerPrivate::Group>(i)), items);
        }
    }
    lk.unlock();

    return ret;
}


