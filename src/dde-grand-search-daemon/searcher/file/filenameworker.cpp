// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filenameworker_p.h"
#include "global/builtinsearch.h"
#include "utils/specialtools.h"
#include "configuration/configer.h"

#include <QStandardPaths>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;
DFM_SEARCH_USE_NS

#define MAX_SEARCH_NUM_GROUP 1000
#define MAX_SEARCH_NUM_TOTAL 20000
#define EMIT_INTERVAL 50

FileNameWorkerPrivate::FileNameWorkerPrivate(FileNameWorker *qq)
    : q_ptr(qq)
{
    qCDebug(logDaemon) << "FileNameWorkerPrivate constructor called";
    m_searchPath = QDir::homePath();
    initConfig();
}

void FileNameWorkerPrivate::initConfig()
{
    qCDebug(logDaemon) << "Initializing file search configuration";

    // 获取支持的搜索类目
    auto config = ConfigerIns->group(GRANDSEARCH_CLASS_FILE_DEEPIN);
    if (config->value(GRANDSEARCH_GROUP_FOLDER, false)) {
        qCDebug(logDaemon) << "Folder search enabled";
        m_resultCountHash.insert(FileSearchUtils::Folder, 0);
    }

    if (config->value(GRANDSEARCH_GROUP_FILE, false)) {
        m_resultCountHash.insert(FileSearchUtils::File, 0);
        qCDebug(logDaemon) << "File search enabled";
    }

    if (config->value(GRANDSEARCH_GROUP_FILE_VIDEO, false)) {
        m_resultCountHash.insert(FileSearchUtils::Video, 0);
        qCDebug(logDaemon) << "Video search enabled";
    }

    if (config->value(GRANDSEARCH_GROUP_FILE_AUDIO, false)) {
        m_resultCountHash.insert(FileSearchUtils::Audio, 0);
        qCDebug(logDaemon) << "Audio search enabled";
    }

    if (config->value(GRANDSEARCH_GROUP_FILE_PICTURE, false)) {
        m_resultCountHash.insert(FileSearchUtils::Picture, 0);
        qCDebug(logDaemon) << "Picture search enabled";
    }

    if (config->value(GRANDSEARCH_GROUP_FILE_DOCUMNET, false)) {
        m_resultCountHash.insert(FileSearchUtils::Document, 0);
        qCDebug(logDaemon) << "Document search enabled";
    }

    qCDebug(logDaemon) << "Search groups enabled:" << m_resultCountHash.keys();
}

bool FileNameWorkerPrivate::appendSearchResult(const QString &fileName)
{
    Q_Q(FileNameWorker);

    // /目录片段 范围限定与排除词后置过滤
    if (!FileSearchUtils::matchScope(m_searchInfo, fileName)) {
        qCDebug(logDaemon) << "File result filtered by scope rule:" << fileName;
        return false;
    }

    if (m_tmpSearchResults.contains(fileName)) {
        qCDebug(logDaemon) << "Duplicate file result ignored:" << fileName;
        return false;
    }

    auto group = FileSearchUtils::getGroupByName(fileName);
    Q_ASSERT(group >= FileSearchUtils::GroupBegin && group < FileSearchUtils::GroupCount);

    // 类型过滤（doc:/pic:/ext:xxx）生效时，文件夹不参与类型浏览
    if (!m_searchInfo.typeFilter.isEmpty() && group == FileSearchUtils::Folder) {
        qCDebug(logDaemon) << "Folder result ignored by type filter:" << fileName;
        return false;
    }

    // 根据搜索类目配置判断是否需要进行添加
    if (!m_resultCountHash.contains(group)) {
        if (group == FileSearchUtils::Folder) {
            qCDebug(logDaemon) << "Folder result ignored - Folder search not enabled:" << fileName;
            return false;
        }

        if (m_resultCountHash.contains(FileSearchUtils::File)) {
            group = FileSearchUtils::File;
            qCDebug(logDaemon) << "File moved to generic File group:" << fileName;
        } else {
            qCDebug(logDaemon) << "File result ignored - No matching group enabled:" << fileName;
            return false;
        }
    }

    if (m_resultCountHash[group] >= MAX_SEARCH_NUM_GROUP)
        return false;

    m_tmpSearchResults << fileName;
    // Get keywords for highlighting
    QStringList keywords { m_searchInfo.keyword };
    if (!m_searchInfo.boolKeywords.isEmpty()) {
        keywords = m_searchInfo.boolKeywords;
    } else if (!m_searchInfo.typeKeywords.isEmpty()) {
        keywords = m_searchInfo.typeKeywords;
    }
    const auto &item = FileSearchUtils::packItem(fileName, q->name(), keywords);

    QMutexLocker lk(&m_mutex);
    m_items[group].append(item);
    m_resultCountHash[group]++;

    qCDebug(logDaemon) << "Search result added - File:" << fileName << "Group:" << group
                       << "Group count:" << m_resultCountHash[group];

    // 非文件类目搜索，不需要向文件类目中添加搜索结果
    if (m_searchInfo.isCombinationSearch && !m_searchInfo.groupList.contains(FileSearchUtils::File)) {
        qCDebug(logDaemon) << "Combination search - File group not included, skipping file group addition";
        return true;
    }

    // 文档/图片/视频/音频与「其他文件」（File 组）互斥：
    // 不再将非文件类型结果重复塞入文件组，保证各类别互斥且合计为全部文件名命中结果
    return true;
}

SearchOptions FileNameWorkerPrivate::createSearchOptions() const
{
    qCDebug(logDaemon) << "Creating search options - Path:" << m_searchPath;

    SearchOptions options;
    options.setSearchPath(m_searchPath);

    auto config = Configer::instance()->group(GRANDSEARCH_BLACKLIST_GROUP);
    auto blacklist = config->value(GRANDSEARCH_BLACKLIST_PATH, QStringList());
    options.setSearchExcludedPaths(blacklist);

    // 检查文件名索引目录是否可用，如果不可用则回退到实时搜索
    if (DFMSEARCH::Global::isFileNameIndexDirectoryAvailable()) {
        qCDebug(logDaemon) << "Using indexed search method";
        options.setSearchMethod(SearchMethod::Indexed);
    } else {
        qCWarning(logDaemon) << "File name index directory is not available, falling back to realtime search";
        options.setSearchMethod(SearchMethod::Realtime);
    }

    options.setMaxResults(MAX_SEARCH_NUM_TOTAL);
    qCDebug(logDaemon) << "Search options configured - Max results:" << MAX_SEARCH_NUM_TOTAL;
    return options;
}

SearchQuery FileNameWorkerPrivate::createSearchQuery() const
{
    const QString &keyword = m_searchInfo.keyword;
    const QStringList &boolKeywords = m_searchInfo.boolKeywords;
    const QStringList &typeKeywords = m_searchInfo.typeKeywords;

    qCDebug(logDaemon) << "Creating search query - Keyword:" << keyword
                       << "Bool keywords:" << boolKeywords.size()
                       << "Type keywords:" << typeKeywords.size();

    // 类型过滤（doc:/pic:/ext:xxx）存在时，不走“纯 Boolean”分支：
    // 扩展名过滤统一改为 matchScope() 后置处理，避免 dfm-search 库层
    // “Boolean 查询 + fileExtensions”组合在部分版本上返回 0 结果
    // （如 “doc: 8月 会议” 搜不到任何文件）。
    bool useBoolQuery = (boolKeywords.size() >= 2) && m_searchInfo.typeFilter.isEmpty();
    bool useTypeSearch = (typeKeywords.size() >= 2) || m_searchInfo.isCombinationSearch;
    SearchQuery query;

    if (useTypeSearch) {
        qCDebug(logDaemon) << "Using type search mode";
        if (typeKeywords.size() >= 2) {
            query = SearchFactory::createQuery(typeKeywords, SearchQuery::Type::Boolean);
            query.setBooleanOperator(SearchQuery::BooleanOperator::OR);
            qCDebug(logDaemon) << "Created boolean OR query with type keywords";
        } else if (typeKeywords.size() == 1) {
            query = SearchFactory::createQuery(typeKeywords.first(), SearchQuery::Type::Simple);
            qCDebug(logDaemon) << "Created simple query with single type keyword";
        } else {
            query = SearchFactory::createQuery(m_searchInfo.keyword, SearchQuery::Type::Simple);
            qCDebug(logDaemon) << "Created simple query with main keyword";
        }
    } else if (useBoolQuery) {
        qCDebug(logDaemon) << "Using boolean search mode";
        query = SearchFactory::createQuery(boolKeywords, SearchQuery::Type::Boolean);
        // 多关键字检索（空格分隔）：文件名必须同时包含所有关键字
        query.setBooleanOperator(SearchQuery::BooleanOperator::AND);
    } else if (m_searchInfo.hasScope || !m_searchInfo.excludeKeywords.isEmpty() || !m_searchInfo.typeFilter.isEmpty()) {
        // 目录片段限定/排除词/类型过滤搜索：只基于包含词建查询；无包含词时用全量通配候选，由后置过滤
        if (boolKeywords.size() == 1) {
            query = SearchFactory::createQuery(boolKeywords.first(), SearchQuery::Type::Simple);
        } else if (boolKeywords.size() >= 2) {
            // 多包含词 + 类型过滤：Boolean AND 取候选（不带扩展名下推），扩展名由 matchScope() 后置过滤
            query = SearchFactory::createQuery(boolKeywords, SearchQuery::Type::Boolean);
            query.setBooleanOperator(SearchQuery::BooleanOperator::AND);
            qCDebug(logDaemon) << "Created boolean AND query with type filter (post-filtered)";
        } else {
            query = SearchFactory::createQuery(QString("*"), SearchQuery::Type::Wildcard);
        }
    } else {
        SearchQuery::Type queryType = FileSearchUtils::hasWildcard(keyword)
                ? SearchQuery::Type::Wildcard
                : SearchQuery::Type::Simple;
        query = SearchFactory::createQuery(keyword, queryType);
        qCDebug(logDaemon) << "Created query - Type:" << (queryType == SearchQuery::Type::Wildcard ? "Wildcard" : "Simple");
    }

    qCDebug(logDaemon) << "Search query created successfully";
    return query;
}

void FileNameWorkerPrivate::configureFileNameOptions(FileNameOptionsAPI &fileNameOptions, const SearchQuery &query) const
{
    const QString &keyword = m_searchInfo.keyword;
    const QStringList &typeKeywords = m_searchInfo.typeKeywords;

    qCDebug(logDaemon) << "Configuring file name options";

    bool useTypeSearch = (typeKeywords.size() >= 2) || m_searchInfo.isCombinationSearch;

    if (useTypeSearch) {
        qCDebug(logDaemon) << "Type search configuration - Enabling pinyin and setting file types";
        fileNameOptions.setPinyinEnabled(true);
        fileNameOptions.setPinyinAcronymEnabled(true);
        fileNameOptions.setFileTypes(FileSearchUtils::buildDFMSearchFileTypes(m_searchInfo.groupList));
        fileNameOptions.setFileExtensions(m_searchInfo.suffixList);
        qCDebug(logDaemon) << "File extensions configured:" << m_searchInfo.suffixList;
    } else if (query.type() == SearchQuery::Type::Boolean) {
        qCDebug(logDaemon) << "Boolean query configuration - Enabling pinyin";
        fileNameOptions.setPinyinEnabled(true);
        fileNameOptions.setPinyinAcronymEnabled(true);
    } else if (query.type() == SearchQuery::Type::Wildcard) {
        qCDebug(logDaemon) << "Wildcard query configuration - Disabling pinyin";
        // 通配符搜索通常不需要拼音支持，因为用户输入的是精确的模式
        fileNameOptions.setPinyinEnabled(false);
        fileNameOptions.setPinyinAcronymEnabled(false);
    } else if (FileSearchUtils::isPinyin(keyword)) {
        qCDebug(logDaemon) << "Pinyin keyword detected - Enabling pinyin support";
        fileNameOptions.setPinyinEnabled(true);
        fileNameOptions.setPinyinAcronymEnabled(true);
    }

    // 注意：doc:/pic:/ext:xxx 类型过滤不再下推到库层 setFileExtensions（部分 dfm-search
    // 版本与 Boolean 查询组合后返回 0 结果），统一由 matchScope() 后置过滤扩展名。
}

bool FileNameWorkerPrivate::processSearchResults(const SearchResultExpected &result)
{
    qCDebug(logDaemon) << "Processing search results";

    if (!result.hasValue()) {
        qCWarning(logDaemon) << "Search failed for keyword:" << m_searchInfo.keyword
                             << "Error:" << result.error().message();
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
        if (index % 100 == 0 && isResultLimit()) {
            qCDebug(logDaemon) << "Result limit reached - Breaking search loop at index:" << index;
            break;
        }

        ++index;
    }

    int leave = itemCount();
    qCInfo(logDaemon) << "Search completed - Results by group:" << m_resultCountHash
                      << "Time elapsed:" << m_time.elapsed() << "ms"
                      << "Total items:" << leave;

    return true;
}

bool FileNameWorkerPrivate::searchByDFMSearch()
{
    qCDebug(logDaemon) << "Starting DFM search with keyword:" << m_searchInfo.keyword;
    QObject holder;
    SearchEngine *engine = SearchFactory::createEngine(SearchType::FileName, &holder);

    // 创建搜索选项
    SearchOptions options = createSearchOptions();
    // 创建搜索查询
    SearchQuery query = createSearchQuery();

    // 配置文件名选项
    FileNameOptionsAPI fileNameOptions(options);
    configureFileNameOptions(fileNameOptions, query);

    // 设置搜索选项
    engine->setSearchOptions(options);
    qCDebug(logDaemon) << "Search options configured on engine";

    // 执行搜索并处理结果
    qCDebug(logDaemon) << "Executing synchronous search";
    const SearchResultExpected &result = engine->searchSync(query);
    return processSearchResults(result);
}

void FileNameWorkerPrivate::tryNotify()
{
    Q_Q(FileNameWorker);
    int cur = m_time.elapsed();
    if (q->hasItem() && (cur - m_lastEmit) > EMIT_INTERVAL) {
        m_lastEmit = cur;
        qCDebug(logDaemon) << "Emitting new results - Time elapsed:" << cur << "ms";
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
    bool limited = false;
    auto it = m_resultCountHash.cbegin();
    for (; it != m_resultCountHash.cend(); ++it) {
        limited = it.value() >= MAX_SEARCH_NUM_GROUP;
        if (limited)
            break;
    }

    return limited;
}

FileNameWorker::FileNameWorker(const QString &name, QObject *parent)
    : ProxyWorker(name, parent),
      d_ptr(new FileNameWorkerPrivate(this))
{
    qCDebug(logDaemon) << "FileNameWorker constructor - Name:" << name;
}

FileNameWorker::~FileNameWorker()
{
    qCDebug(logDaemon) << "FileNameWorker destructor called";
    delete d_ptr;
    d_ptr = nullptr;
}

void FileNameWorker::setContext(const QString &context)
{
    Q_D(FileNameWorker);

    if (context.isEmpty())
        qCWarning(logDaemon) << "Search key is empty";
    d->m_searchInfo = FileSearchUtils::parseContent(context);
    qCDebug(logDaemon) << "Parsed search keyword:" << d->m_searchInfo.keyword;
}

bool FileNameWorker::isAsync() const
{
    return false;
}

bool FileNameWorker::working(void *context)
{
    Q_D(FileNameWorker);
    Q_UNUSED(context)

    qCDebug(logDaemon) << "FileNameWorker starting work";

    if (!d->m_status.testAndSetRelease(Ready, Runing)) {
        qCWarning(logDaemon) << "Failed to start worker - Invalid state transition";
        return false;
    }

    if (d->m_searchInfo.keyword.isEmpty() || d->m_searchPath.isEmpty()) {
        qCWarning(logDaemon) << "Search prerequisites not met - Empty keyword or path";
        d->m_status.storeRelease(Completed);
        return false;
    }

    qCDebug(logDaemon) << "Starting file name search with keyword:" << d->m_searchInfo.keyword;
    d->m_time.start();

    if (!d->searchByDFMSearch()) {
        qCWarning(logDaemon) << "Search operation failed";
        return false;
    }

    // 检查是否还有数据
    if (d->m_status.testAndSetRelease(Runing, Completed)) {
        if (hasItem()) {
            qCDebug(logDaemon) << "Search completed - Emitting final results";
            emit unearthed(this);
        }
    }
    return true;
}

void FileNameWorker::terminate()
{
    Q_D(FileNameWorker);
    qCDebug(logDaemon) << "Terminating file name worker";
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

    qCDebug(logDaemon) << "Taking all search results";

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
