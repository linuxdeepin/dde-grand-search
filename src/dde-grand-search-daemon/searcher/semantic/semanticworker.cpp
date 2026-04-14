// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semanticworker_p.h"
#include "global/builtinsearch.h"
#include "searcher/file/filesearchutils.h"
#include "config/ruleconfigloader.h"

#include <dfm-search/searchfactory.h>
#include <dfm-search/filenamesearchapi.h>
#include <dfm-search/contentsearchapi.h>
#include <dfm-search/ocrtextsearchapi.h>

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;
DFM_SEARCH_USE_NS

#define checkRunning()                                 \
    if (m_status.loadAcquire() != ProxyWorker::Runing) \
    return false

SemanticWorkerPrivate::SemanticWorkerPrivate(SemanticWorker *parent)
    : q(parent)
{
    qCDebug(logDaemon) << "SemanticWorkerPrivate constructor";
    initParser();
}

SemanticWorkerPrivate::~SemanticWorkerPrivate()
{
    qCDebug(logDaemon) << "SemanticWorkerPrivate destructor";
}

void SemanticWorkerPrivate::initParser()
{
    // 使用 RuleConfigLoader 实现配置加载优先级
    // 优先级：本地配置 > qrc 内置配置
    m_configLoader = std::make_unique<RuleConfigLoader>();

    m_ruleEngine = std::make_unique<SemanticRuleEngine>();
    if (!m_ruleEngine->loadWithConfigLoader(m_configLoader.get())) {
        qCWarning(logDaemon) << "Failed to load semantic rules";
    }

    // 创建意图解析器
    m_parser = std::make_unique<IntentParser>(m_ruleEngine.get());

    // 创建查询构建器
    m_queryBuilder = std::make_unique<QueryBuilder>();
    m_queryBuilder->setMaxResults(MAX_RESULTS_TOTAL);

    qCDebug(logDaemon) << "Semantic parser initialized";
}

bool SemanticWorkerPrivate::searchByFileName(const SemanticSearchQuery &query)
{
    if (!query.hasFileNameSearch) {
        qCDebug(logDaemon) << "No file name search query";
        return true;
    }

    qCInfo(logDaemon) << "Starting file name search - keyword:" << query.fileNameQuery.keyword();

    QObject holder;
    SearchEngine *engine = SearchFactory::createEngine(SearchType::FileName, &holder);
    if (!engine) {
        qCWarning(logDaemon) << "Failed to create file name search engine";
        return false;
    }

    engine->setSearchOptions(query.fileNameOptions);

    const SearchResultExpected &result = engine->searchSync(query.fileNameQuery);
    return processSearchResults(result, false);
}

bool SemanticWorkerPrivate::searchByContent(const SemanticSearchQuery &query)
{
    if (!query.hasContentSearch) {
        qCDebug(logDaemon) << "No content search query";
        return true;
    }

    qCInfo(logDaemon) << "Starting content search - keyword:" << query.contentQuery.keyword();

    QObject holder;
    SearchEngine *engine = SearchFactory::createEngine(SearchType::Content, &holder);
    if (!engine) {
        qCWarning(logDaemon) << "Failed to create content search engine";
        return false;
    }

    engine->setSearchOptions(query.contentOptions);

    const SearchResultExpected &result = engine->searchSync(query.contentQuery);
    return processSearchResults(result, true);
}

bool SemanticWorkerPrivate::searchByOcr(const SemanticSearchQuery &query)
{
    if (!query.hasOcrSearch) {
        qCDebug(logDaemon) << "No OCR text search query";
        return true;
    }

    qCInfo(logDaemon) << "Starting OCR text search - keyword:" << query.ocrQuery.keyword();

    QObject holder;
    SearchEngine *engine = SearchFactory::createEngine(SearchType::Ocr, &holder);
    if (!engine) {
        qCWarning(logDaemon) << "Failed to create OCR text search engine";
        return false;
    }

    engine->setSearchOptions(query.ocrOptions);

    const SearchResultExpected &result = engine->searchSync(query.ocrQuery);
    return processSearchResults(result, false);
}

bool SemanticWorkerPrivate::processSearchResults(const SearchResultExpected &result, bool isContentSearch)
{
    if (!result.hasValue()) {
        qCWarning(logDaemon) << "Search failed - error:" << result.error().message();
        return false;
    }

    const auto &files = result.value();
    qCDebug(logDaemon) << "Processing" << files.size() << "search results (isContent:" << isContentSearch << ")";

    int processedCount = 0;
    for (const auto &file : files) {
        checkRunning();

        appendSearchResult(file.path(), isContentSearch);

        // 定期通知
        if (++processedCount % 100 == 0) {
            tryNotify();
        }

        // 检查结果限制
        if (isResultLimit()) {
            qCDebug(logDaemon) << "Result limit reached at" << processedCount;
            break;
        }
    }

    return true;
}

bool SemanticWorkerPrivate::appendSearchResult(const QString &filePath, bool isContentSearch)
{
    // 去重检查
    if (m_tmpSearchResults.contains(filePath)) {
        return false;
    }

    // 黑名单过滤
    if (FileSearchUtils::filterByBlacklist(filePath)) {
        return false;
    }

    // 语义搜索结果统一放在 GRANDSEARCH_GROUP_FILE_INFERENCE 分组，不进行分类
    QString groupKey = GRANDSEARCH_GROUP_FILE_INFERENCE;

    // 打包结果项
    MatchedItem item = FileSearchUtils::packItem(filePath, q->name());

    // 如果是内容搜索，添加额外信息
    if (isContentSearch) {
        QVariantHash extra = item.extra.toHash();
        extra.insert(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 10);   // 内容搜索结果权重更高
        item.extra = QVariant::fromValue(extra);
    }

    // 添加到结果集
    {
        QMutexLocker locker(&m_mutex);
        if (m_items[groupKey].size() < MAX_RESULTS_TOTAL) {
            m_items[groupKey].append(item);
            m_tmpSearchResults.insert(filePath);
        }
    }

    return true;
}

void SemanticWorkerPrivate::tryNotify()
{
    int cur = m_time.elapsed();
    if (q->hasItem() && (cur - m_lastEmit) > EMIT_INTERVAL) {
        m_lastEmit = cur;
        qCDebug(logDaemon) << "Emitting unearthed signal - elapsed:" << cur << "ms";
        emit q->unearthed(q);
    }
}

int SemanticWorkerPrivate::itemCount() const
{
    QMutexLocker locker(&m_mutex);
    int count = 0;
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        count += it.value().size();
    }
    return count;
}

bool SemanticWorkerPrivate::isResultLimit() const
{
    return itemCount() >= MAX_RESULTS_TOTAL;
}

//===================================================================
// SemanticWorker implementation
//===================================================================

SemanticWorker::SemanticWorker(const QString &name, QObject *parent)
    : ProxyWorker(name, parent), d(new SemanticWorkerPrivate(this))
{
    qCDebug(logDaemon) << "SemanticWorker constructor - name:" << name;
}

SemanticWorker::~SemanticWorker()
{
    qCDebug(logDaemon) << "SemanticWorker destructor";
    delete d;
    d = nullptr;
}

void SemanticWorker::setContext(const QString &context)
{
    qCDebug(logDaemon) << "Setting context - length:" << context.length();
    d->m_context = context;
}

bool SemanticWorker::isAsync() const
{
    return false;
}

bool SemanticWorker::working(void *context)
{
    Q_UNUSED(context)

    // 状态检查
    if (!d->m_status.testAndSetRelease(Ready, Runing)) {
        qCWarning(logDaemon) << "Worker not in Ready state";
        return false;
    }

    qCInfo(logDaemon) << "Starting semantic search - context:" << d->m_context;

    // 发送空数据用于提前显示分组
    {
        QMutexLocker locker(&d->m_mutex);
        d->m_items.insert(GRANDSEARCH_GROUP_FILE_INFERENCE, {});
    }
    emit unearthed(this);

    // 启动计时
    d->m_time.start();

    // 检查语义搜索是否启用
    if (!d->m_semanticEnabled) {
        qCWarning(logDaemon) << "Semantic search not enabled";
        d->m_status.storeRelease(Completed);
        return false;
    }

    // 解析用户意图
    if (!d->m_parser || !d->m_queryBuilder) {
        qCWarning(logDaemon) << "Parser or QueryBuilder not initialized";
        d->m_status.storeRelease(Completed);
        return false;
    }
    ParsedIntent intent = d->m_parser->parse(d->m_context);
    qCInfo(logDaemon) << "Parsed intent:" << intent.toString();

    // 检查是否有有效的搜索条件
    if (!intent.hasKeywords() && !intent.hasFileTypes() && !intent.hasTimeConstraint()) {
        qCWarning(logDaemon) << "No valid search criteria found";
        d->m_status.storeRelease(Completed);
        return false;
    }

    // 构建搜索查询（时间约束已通过 TimeRangeFilter 集成到搜索选项中）
    d->m_queryBuilder->setFulltextEnabled(d->m_fulltextEnabled);
    d->m_queryBuilder->setOcrSearchEnabled(d->m_ocrtextEnabled);
    SemanticSearchQuery query = d->m_queryBuilder->build(intent);
    qCInfo(logDaemon) << "Built query:" << query.toString();

    // 执行文件名搜索
    if (query.hasFileNameSearch) {
        if (d->m_status.loadAcquire() != ProxyWorker::Runing)
            return false;
        if (!d->searchByFileName(query)) {
            qCWarning(logDaemon) << "File name search failed";
        }
        d->tryNotify();
    }

    // 执行内容搜索（仅在全文搜索启用时）
    if (query.hasContentSearch && d->m_fulltextEnabled && !d->isResultLimit()) {
        if (d->m_status.loadAcquire() != ProxyWorker::Runing)
            return false;
        if (!d->searchByContent(query)) {
            qCWarning(logDaemon) << "Content search failed";
        }
        d->tryNotify();
    }

    // 执行 OCR 文本搜索
    if (query.hasOcrSearch && d->m_ocrtextEnabled && !d->isResultLimit()) {
        if (d->m_status.loadAcquire() != ProxyWorker::Runing)
            return false;
        if (!d->searchByOcr(query)) {
            qCWarning(logDaemon) << "OCR text search failed";
        }
        d->tryNotify();
    }

    // 完成
    d->m_status.storeRelease(Completed);

    qCInfo(logDaemon) << "Semantic search completed - elapsed:" << d->m_time.elapsed()
                      << "ms, results:" << d->itemCount();

    // 发送最终结果
    if (d->itemCount() > 0) {
        emit unearthed(this);
    }

    return true;
}

void SemanticWorker::terminate()
{
    qCDebug(logDaemon) << "Terminating semantic worker";
    d->m_status.storeRelease(Terminated);
}

ProxyWorker::Status SemanticWorker::status()
{
    return static_cast<ProxyWorker::Status>(d->m_status.loadAcquire());
}

bool SemanticWorker::hasItem() const
{
    QMutexLocker locker(&d->m_mutex);
    return !d->m_items.isEmpty();
}

MatchedItemMap SemanticWorker::takeAll()
{
    QMutexLocker locker(&d->m_mutex);
    MatchedItemMap items = std::move(d->m_items);
    return items;
}

void SemanticWorker::setEngineState(bool semantic, bool ocrtext, bool fulltext)
{
    d->m_semanticEnabled = semantic;
    d->m_fulltextEnabled = fulltext;
    d->m_ocrtextEnabled = ocrtext;
    qCDebug(logDaemon) << "Engine state set - semantic:" << semantic << "fulltext:" << fulltext << "ocrtext:" << ocrtext;
}
