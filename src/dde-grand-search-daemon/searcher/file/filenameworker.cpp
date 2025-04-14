// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filenameworker_p.h"
#include "global/builtinsearch.h"
#include "utils/specialtools.h"
#include "configuration/configer.h"

#include <dfm-search/searchfactory.h>
#include <dfm-search/filenamesearchapi.h>

#include <QStandardPaths>

using namespace GrandSearch;
DFM_SEARCH_USE_NS

#define MAX_SEARCH_NUM_GROUP 100
#define MAX_SEARCH_NUM_TOTAL 10000
#define EMIT_INTERVAL 50

FileNameWorkerPrivate::FileNameWorkerPrivate(FileNameWorker *qq)
    : q_ptr(qq)
{
    // TODO (search): 搜索目录为user, by anything
    m_searchPath = QDir::rootPath();
    initConfig();
}

void FileNameWorkerPrivate::initConfig()
{
    // 获取支持的搜索类目
    auto config = ConfigerIns->group(GRANDSEARCH_CLASS_FILE_DEEPIN);
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

bool FileNameWorkerPrivate::appendSearchResult(const QString &fileName)
{
    Q_Q(FileNameWorker);

    if (m_tmpSearchResults.contains(fileName))
        return false;

    auto group = FileSearchUtils::getGroupByName(fileName);
    Q_ASSERT(group >= FileSearchUtils::GroupBegin && group < FileSearchUtils::GroupCount);

    // 根据搜索类目配置判断是否需要进行添加
    if (!m_resultCountHash.contains(group)) {
        if (group == FileSearchUtils::Folder)
            return false;

        if (m_resultCountHash.contains(FileSearchUtils::File)) {
            group = FileSearchUtils::File;
        } else {
            return false;
        }
    }

    if (!FileSearchUtils::fileShouldVisible(fileName, group, m_searchInfo))
        return false;

    if (m_resultCountHash[group] >= MAX_SEARCH_NUM_GROUP || FileSearchUtils::filterByBlacklist(fileName))
        return false;

    m_tmpSearchResults << fileName;
    const auto &item = FileSearchUtils::packItem(fileName, q->name());
    QMutexLocker lk(&m_mutex);
    m_items[group].append(item);
    m_resultCountHash[group]++;

    // 非文件类目搜索，不需要向文件类目中添加搜索结果
    if (m_searchInfo.isCombinationSearch && !m_searchInfo.groupList.contains(FileSearchUtils::File))
        return true;

    // 文档、音频、视频、图片需添加到文件组中
    if (group != FileSearchUtils::File && m_resultCountHash.contains(FileSearchUtils::File)) {
        if (group != FileSearchUtils::Folder && m_resultCountHash[FileSearchUtils::File] < MAX_SEARCH_NUM_GROUP) {
            m_items[FileSearchUtils::File].append(item);
            m_resultCountHash[FileSearchUtils::File]++;
        }
    }

    return true;
}

bool FileNameWorkerPrivate::searchByDFMSearch()
{
    // TODO (search): 类目、bool
    const QString &keyword = m_searchInfo.keyword;
    const QStringList &boolKeywords = m_searchInfo.boolKeywords;
    QObject holder;
    SearchEngine *engine = SearchFactory::createEngine(SearchType::FileName, &holder);
    SearchOptions options;
    options.setSearchPath(m_searchPath);
    options.setSearchMethod(SearchMethod::Indexed);
    options.setMaxResults(MAX_SEARCH_NUM_TOTAL);

    // TODO (search): refactor
    bool useBoolQuery = (boolKeywords.size() >= 2);
    SearchQuery query;
    FileNameOptionsAPI fileNameOptions(options);

    if (useBoolQuery) {
        query = SearchFactory::createQuery(boolKeywords, SearchQuery::Type::Boolean);
        fileNameOptions.setPinyinEnabled(true);
    } else {
        query = SearchFactory::createQuery(keyword, SearchQuery::Type::Simple);
        if (isPurePinyin(keyword)) {   // TODO (search): remove
            fileNameOptions.setPinyinEnabled(true);
        }
    }

    engine->setSearchOptions(options);
    const SearchResultExpected &result = engine->searchSync(query);
    if (!result.hasValue()) {
        qWarning() << "DFMSearch failed for key: " << keyword << result.error().message();
        return false;
    }

    int index = 0;
    for (const auto &file : result.value()) {
        if (m_status.loadAcquire() != ProxyWorker::Runing)
            return false;

        appendSearchResult(file.path());

        // 推送
        tryNotify();

        // perf: 降频
        if (index % 100 == 0 && isResultLimit())
            break;

        ++index;
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
    for (int i = FileSearchUtils::GroupBegin; i < FileSearchUtils::GroupCount; ++i)
        count += m_items[i].size();

    return count;
}

bool FileNameWorkerPrivate::isResultLimit()
{
    const auto &iter = std::find_if(m_resultCountHash.begin(), m_resultCountHash.end(), [](const int &num) {
        return num <= MAX_SEARCH_NUM_GROUP;
    });

    return iter == m_resultCountHash.end();
}

bool FileNameWorkerPrivate::isPurePinyin(const QString &str)
{
    static QRegularExpression regex(R"(^[a-zA-Z]+$)");
    return regex.match(str).hasMatch();
}

FileNameWorker::FileNameWorker(const QString &name, QObject *parent)
    : ProxyWorker(name, parent),
      d_ptr(new FileNameWorkerPrivate(this))
{
}

FileNameWorker::~FileNameWorker()
{
    delete d_ptr;
    d_ptr = nullptr;
}

void FileNameWorker::setContext(const QString &context)
{
    Q_D(FileNameWorker);

    if (context.isEmpty())
        qWarning() << "search key is empty.";
    d->m_searchInfo = FileSearchUtils::parseContent(context);
}

bool FileNameWorker::isAsync() const
{
    return false;
}

bool FileNameWorker::working(void *context)
{
    Q_D(FileNameWorker);
    Q_UNUSED(context)

    // 准备状态切运行中，否则直接返回
    if (!d->m_status.testAndSetRelease(Ready, Runing))
        return false;

    if (d->m_searchInfo.keyword.isEmpty() || d->m_searchPath.isEmpty()
        || !DFMSEARCH::Global::isFileNameIndexDirectoryAvailable()) {
        d->m_status.storeRelease(Completed);
        return false;
    }

    d->m_time.start();

    if (!d->searchByDFMSearch())
        return false;   // 中断

    // 检查是否还有数据
    if (d->m_status.testAndSetRelease(Runing, Completed)) {
        // 发送数据
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
    for (int i = FileSearchUtils::GroupBegin; i < FileSearchUtils::GroupCount; ++i)
        if (!d->m_items[i].isEmpty())
            return true;

    return false;
}

MatchedItemMap FileNameWorker::takeAll()
{
    Q_D(FileNameWorker);

    // 添加分组
    MatchedItemMap ret;

    QMutexLocker lk(&d->m_mutex);
    for (int i = FileSearchUtils::GroupBegin; i < FileSearchUtils::GroupCount; ++i) {
        if (!d->m_items[i].isEmpty()) {
            MatchedItems items = std::move(d->m_items[i]);
            Q_ASSERT(d->m_items[i].isEmpty());
            ret.insert(FileSearchUtils::groupKey(static_cast<FileSearchUtils::Group>(i)), items);
        }
    }
    lk.unlock();

    return ret;
}
