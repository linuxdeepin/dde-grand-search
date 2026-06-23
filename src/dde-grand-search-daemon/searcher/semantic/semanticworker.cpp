// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semanticworker_p.h"
#include "global/builtinsearch.h"

#include <QDebug>
#include <QFileInfo>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

DFM_SEARCH_USE_NS

#define MAX_SEARCH_NUM_GROUP 100
#define EMIT_INTERVAL 50

using namespace GrandSearch;

SemanticWorkerPrivate::SemanticWorkerPrivate(SemanticWorker *qq)
    : q_ptr(qq)
{
}

SemanticWorkerPrivate::~SemanticWorkerPrivate()
{
}

bool SemanticWorkerPrivate::doSearch()
{
    Q_Q(SemanticWorker);

    qCDebug(logDaemon) << "Starting semantic search - Context:" << m_context;

    SemanticSearcher dfmSearcher;

    // 设置搜索参数
    dfmSearcher.setMaxResults(MAX_SEARCH_NUM_GROUP);
    dfmSearcher.setSearchTimeout(60);

    // 先判断是否为语义查询，如果不是则跳过
    if (!dfmSearcher.isSemanticQuery(m_context)) {
        qCDebug(logDaemon) << "Input is not a semantic query, skipping semantic search";
        return false;
    }

    // 通过 intentParsed 信号提取关键词
    // searchSync 内部会发出 intentParsed 信号，但使用 searchSync 时
    // 信号是在同一个线程中同步发出的，需要提前连接
    QObject::connect(&dfmSearcher, &SemanticSearcher::intentParsed,
                     [this](const ParsedIntent &intent) {
                         m_keywords = intent.keywords();
                         qCDebug(logDaemon) << "Semantic intent parsed - Keywords:" << m_keywords
                                            << "Extensions:" << intent.fileExtensions();
                     });

    // 执行同步搜索
    const SearchResultExpected result = dfmSearcher.searchSync(m_context);

    if (!result.hasValue()) {
        qCWarning(logDaemon) << "Semantic search failed - Error:" << result.error().message();
        return false;
    }

    processResults(result.value());

    qCInfo(logDaemon) << "Semantic search completed - Total results:" << m_tmpSearchResults.size()
                      << "Time elapsed:" << m_time.elapsed() << "ms";

    return true;
}

void SemanticWorkerPrivate::processResults(const SearchResultList &results)
{
    Q_Q(SemanticWorker);

    qCDebug(logDaemon) << "Processing semantic search results - Count:" << results.size();

    int count = 0;
    for (const auto &r : results) {
        if (m_status.loadAcquire() != ProxyWorker::Runing)
            return;

        const QString &filePath = r.path();

        // 去重
        if (m_tmpSearchResults.contains(filePath)) {
            continue;
        }
        m_tmpSearchResults << filePath;

        // 使用 FileSearchUtils 打包结果
        MatchedItem item = FileSearchUtils::packItem(filePath, q->name(), m_keywords);

        // 设置权重
        QVariantHash extra = item.extra.toHash();
        extra.insert(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 5);
        item.extra = extra;

        {
            QMutexLocker lk(&m_mutex);
            m_items.append(item);
        }

        count++;
        tryNotify();
    }
}

void SemanticWorkerPrivate::tryNotify()
{
    Q_Q(SemanticWorker);
    int cur = m_time.elapsed();
    if (q->hasItem() && (cur - m_lastEmit) > EMIT_INTERVAL) {
        m_lastEmit = cur;
        emit q->unearthed(q);
    }
}

int SemanticWorkerPrivate::itemCount() const
{
    QMutexLocker lk(&m_mutex);
    return m_items.size();
}

SemanticWorker::SemanticWorker(const QString &name, QObject *parent)
    : ProxyWorker(name, parent),
      d_ptr(new SemanticWorkerPrivate(this))
{
    qCDebug(logDaemon) << "SemanticWorker constructor - Name:" << name;
}

SemanticWorker::~SemanticWorker()
{
    qCDebug(logDaemon) << "SemanticWorker destructor called";
    delete d_ptr;
    d_ptr = nullptr;
}

void SemanticWorker::setContext(const QString &context)
{
    Q_D(SemanticWorker);

    if (context.isEmpty())
        qCWarning(logDaemon) << "Semantic search context is empty";
    d->m_context = context;
    qCDebug(logDaemon) << "Semantic search context set - Length:" << context.length();
}

bool SemanticWorker::isAsync() const
{
    return false;
}

bool SemanticWorker::working(void *context)
{
    Q_D(SemanticWorker);
    Q_UNUSED(context)

    qCDebug(logDaemon) << "SemanticWorker starting work";

    if (!d->m_status.testAndSetRelease(Ready, Runing)) {
        qCWarning(logDaemon) << "Failed to start worker - Invalid state transition";
        return false;
    }

    if (d->m_context.isEmpty()) {
        qCWarning(logDaemon) << "Search context is empty";
        d->m_status.storeRelease(Completed);
        return false;
    }

    qCDebug(logDaemon) << "Starting semantic search with context:" << d->m_context;
    d->m_time.start();

    if (!d->doSearch()) {
        qCDebug(logDaemon) << "Semantic search returned no results or was skipped";
    }

    if (d->m_status.testAndSetRelease(Runing, Completed)) {
        if (hasItem()) {
            qCDebug(logDaemon) << "Semantic search completed - Emitting final results";
            emit unearthed(this);
        }
    }
    return true;
}

void SemanticWorker::terminate()
{
    Q_D(SemanticWorker);
    qCDebug(logDaemon) << "Terminating semantic worker";
    d->m_status.storeRelease(Terminated);
}

ProxyWorker::Status SemanticWorker::status()
{
    Q_D(SemanticWorker);
    return static_cast<ProxyWorker::Status>(d->m_status.loadAcquire());
}

bool SemanticWorker::hasItem() const
{
    Q_D(const SemanticWorker);

    QMutexLocker lk(&d->m_mutex);
    return !d->m_items.isEmpty();
}

MatchedItemMap SemanticWorker::takeAll()
{
    Q_D(SemanticWorker);

    qCDebug(logDaemon) << "Taking all semantic search results";

    MatchedItemMap ret;

    QMutexLocker lk(&d->m_mutex);
    if (!d->m_items.isEmpty()) {
        MatchedItems items = std::move(d->m_items);
        Q_ASSERT(d->m_items.isEmpty());
        ret.insert(GRANDSEARCH_GROUP_FILE_SEMANTIC, items);
    }
    lk.unlock();

    return ret;
}
