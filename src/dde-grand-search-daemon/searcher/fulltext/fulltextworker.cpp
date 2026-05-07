// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fulltextworker_p.h"
#include "global/builtinsearch.h"
#include "configuration/configer.h"

#include <QStandardPaths>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;
DFM_SEARCH_USE_NS

#define MAX_SEARCH_NUM_GROUP 100
#define EMIT_INTERVAL 50

FullTextWorkerPrivate::FullTextWorkerPrivate(FullTextWorker *qq)
    : q_ptr(qq)
{
    qCDebug(logDaemon) << "FullTextWorkerPrivate constructor called";
    m_searchPath = QDir::homePath();

    auto config = ConfigerIns->group(GRANDSEARCH_CLASS_FILE_DEEPIN);
    m_fileGroupEnabled = config->value(GRANDSEARCH_GROUP_FILE, false);
    qCDebug(logDaemon) << "FullText file group enabled:" << m_fileGroupEnabled;
}

bool FullTextWorkerPrivate::doSearch()
{
    qCDebug(logDaemon) << "Starting content search with keyword:" << m_searchInfo.keyword;

    QObject holder;
    SearchEngine *engine = SearchFactory::createEngine(SearchType::Content, &holder);
    if (!engine) {
        qCWarning(logDaemon) << "Failed to create content search engine";
        return false;
    }

    SearchOptions options;
    options.setSearchPath(m_searchPath);
    options.setSearchMethod(SearchMethod::Indexed);
    options.setMaxResults(MAX_SEARCH_NUM_GROUP);

    ContentOptionsAPI contentOptions(options);
    contentOptions.setMaxPreviewLength(50);
    contentOptions.setFilenameContentMixedAndSearchEnabled(true);

    engine->setSearchOptions(options);
    qCDebug(logDaemon) << "Content search options configured";

    SearchQuery query;
    if (!m_searchInfo.boolKeywords.isEmpty()) {
        query = SearchFactory::createQuery(m_searchInfo.boolKeywords, SearchQuery::Type::Boolean);
        query.setBooleanOperator(SearchQuery::BooleanOperator::AND);
    } else if (!m_searchInfo.typeKeywords.isEmpty()) {
        query = SearchFactory::createQuery(m_searchInfo.typeKeywords, SearchQuery::Type::Boolean);
        query.setBooleanOperator(SearchQuery::BooleanOperator::OR);
    } else {
        query = SearchFactory::createQuery(m_searchInfo.keyword, SearchQuery::Type::Simple);
    }

    qCDebug(logDaemon) << "Executing content search";
    const SearchResultExpected &result = engine->searchSync(query);
    return processResults(result);
}

bool FullTextWorkerPrivate::processResults(const SearchResultExpected &result)
{
    Q_Q(FullTextWorker);

    qCDebug(logDaemon) << "Processing content search results";

    if (!result.hasValue()) {
        qCWarning(logDaemon) << "Content search failed - Error:" << result.error().message();
        return false;
    }

    int contentCount = 0;
    // Get keywords for highlighting
    QStringList keywords { m_searchInfo.keyword };
    if (!m_searchInfo.boolKeywords.isEmpty()) {
        keywords = m_searchInfo.boolKeywords;
    } else if (!m_searchInfo.typeKeywords.isEmpty()) {
        keywords = m_searchInfo.typeKeywords;
    }

    for (const auto &file : result.value()) {
        if (m_status.loadAcquire() != ProxyWorker::Runing)
            return false;

        const QString &filePath = file.path();

        if (m_tmpSearchResults.contains(filePath)) {
            qCDebug(logDaemon) << "Duplicate content result ignored:" << filePath;
            continue;
        }

        m_tmpSearchResults << filePath;
        MatchedItem item = FileSearchUtils::packItem(filePath, q->name(), keywords);

        // Set higher weight so frontend dedup prefers content results over filename results
        QVariantHash extra = item.extra.toHash();
        extra.insert(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 10);

        // Get highlighted content preview
        SearchResult mutableResult = const_cast<SearchResult &>(file);
        ContentResultAPI contentResult(mutableResult);
        QString highlightedContent = contentResult.highlightedContent();
        if (!highlightedContent.isEmpty()) {
            extra.insert(GRANDSEARCH_PROPERTY_ITEM_MATCHEDCONTEXT, highlightedContent);
        }

        item.extra = extra;

        // Add to Document category
        {
            QMutexLocker lk(&m_mutex);
            m_items[FileSearchUtils::Document].append(item);

            // Also add to File group if enabled
            if (m_fileGroupEnabled) {
                m_items[FileSearchUtils::File].append(item);
            }
        }

        contentCount++;
        qCDebug(logDaemon) << "Content search result added - File:" << filePath
                           << "Total doc count:" << contentCount;

        tryNotify();
    }

    qCInfo(logDaemon) << "Content search completed - Added:" << contentCount
                      << "Time elapsed:" << m_time.elapsed() << "ms";

    return true;
}

void FullTextWorkerPrivate::tryNotify()
{
    Q_Q(FullTextWorker);
    int cur = m_time.elapsed();
    if (q->hasItem() && (cur - m_lastEmit) > EMIT_INTERVAL) {
        m_lastEmit = cur;
        qCDebug(logDaemon) << "Emitting new results - Time elapsed:" << cur << "ms";
        emit q->unearthed(q);
    }
}

int FullTextWorkerPrivate::itemCount() const
{
    QMutexLocker lk(&m_mutex);

    int count = 0;
    for (int i = FileSearchUtils::GroupBegin; i < FileSearchUtils::GroupCount; ++i)
        count += m_items[i].size();

    return count;
}

FullTextWorker::FullTextWorker(const QString &name, QObject *parent)
    : ProxyWorker(name, parent),
      d_ptr(new FullTextWorkerPrivate(this))
{
    qCDebug(logDaemon) << "FullTextWorker constructor - Name:" << name;
}

FullTextWorker::~FullTextWorker()
{
    qCDebug(logDaemon) << "FullTextWorker destructor called";
    delete d_ptr;
    d_ptr = nullptr;
}

void FullTextWorker::setContext(const QString &context)
{
    Q_D(FullTextWorker);

    if (context.isEmpty())
        qCWarning(logDaemon) << "Search key is empty";
    d->m_searchInfo = FileSearchUtils::parseContent(context);
    qCDebug(logDaemon) << "Parsed search keyword:" << d->m_searchInfo.keyword;
}

bool FullTextWorker::isAsync() const
{
    return false;
}

bool FullTextWorker::working(void *context)
{
    Q_D(FullTextWorker);
    Q_UNUSED(context)

    qCDebug(logDaemon) << "FullTextWorker starting work";

    if (!d->m_status.testAndSetRelease(Ready, Runing)) {
        qCWarning(logDaemon) << "Failed to start worker - Invalid state transition";
        return false;
    }

    if (d->m_searchInfo.keyword.isEmpty() || d->m_searchPath.isEmpty()) {
        qCWarning(logDaemon) << "Search prerequisites not met - Empty keyword or path";
        d->m_status.storeRelease(Completed);
        return false;
    }

    // Skip combination search (handled by FullTextSearcher::isActive, double-check here)
    if (d->m_searchInfo.isCombinationSearch) {
        qCDebug(logDaemon) << "Skipping content search in combination search mode";
        d->m_status.storeRelease(Completed);
        return false;
    }

    qCDebug(logDaemon) << "Starting full-text content search with keyword:" << d->m_searchInfo.keyword;
    d->m_time.start();

    if (!d->doSearch()) {
        qCWarning(logDaemon) << "Content search operation failed";
        return false;
    }

    if (d->m_status.testAndSetRelease(Runing, Completed)) {
        if (hasItem()) {
            qCDebug(logDaemon) << "Content search completed - Emitting final results";
            emit unearthed(this);
        }
    }
    return true;
}

void FullTextWorker::terminate()
{
    Q_D(FullTextWorker);
    qCDebug(logDaemon) << "Terminating full-text worker";
    d->m_status.storeRelease(Terminated);
}

ProxyWorker::Status FullTextWorker::status()
{
    Q_D(FullTextWorker);

    return static_cast<ProxyWorker::Status>(d->m_status.loadAcquire());
}

bool FullTextWorker::hasItem() const
{
    Q_D(const FullTextWorker);

    QMutexLocker lk(&d->m_mutex);
    for (int i = FileSearchUtils::GroupBegin; i < FileSearchUtils::GroupCount; ++i)
        if (!d->m_items[i].isEmpty())
            return true;

    return false;
}

MatchedItemMap FullTextWorker::takeAll()
{
    Q_D(FullTextWorker);

    qCDebug(logDaemon) << "Taking all content search results";

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
