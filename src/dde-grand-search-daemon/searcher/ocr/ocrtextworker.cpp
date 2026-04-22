// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrtextworker_p.h"
#include "global/builtinsearch.h"
#include "utils/specialtools.h"

#include <DConfig>

#include <QStandardPaths>
#include <QLoggingCategory>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

DCORE_USE_NAMESPACE

using namespace GrandSearch;
DFM_SEARCH_USE_NS

#define MAX_SEARCH_NUM 100
#define EMIT_INTERVAL 50

OcrTextWorkerPrivate::OcrTextWorkerPrivate(OcrTextWorker *qq)
    : q_ptr(qq)
{
    qCDebug(logDaemon) << "OcrTextWorkerPrivate constructor called";
}

DFMSEARCH::SearchOptions OcrTextWorkerPrivate::createSearchOptions() const
{
    qCDebug(logDaemon) << "Creating OCR search options - Path:" << QDir::homePath();

    SearchOptions options;
    options.setSearchPath(QDir::homePath());
    options.setSearchMethod(SearchMethod::Indexed);
    options.setMaxResults(MAX_SEARCH_NUM);
    qCDebug(logDaemon) << "OCR search options configured - Max results:" << MAX_SEARCH_NUM;
    return options;
}

DFMSEARCH::SearchQuery OcrTextWorkerPrivate::createSearchQuery() const
{
    qCDebug(logDaemon) << "Creating OCR search query - Keyword:" << m_keyword;

    // Create a simple query for the keyword
    SearchQuery query = SearchFactory::createQuery(m_keyword, SearchQuery::Type::Simple);
    return query;
}

bool OcrTextWorkerPrivate::processSearchResults(const DFMSEARCH::SearchResultExpected &result)
{
    qCDebug(logDaemon) << "Processing OCR search results";

    if (!result.hasValue()) {
        qCWarning(logDaemon) << "OCR search failed for keyword:" << m_keyword
                             << "Error:" << result.error().message();
        return false;
    }

    for (const auto &file : result.value()) {
        if (m_status.loadAcquire() != ProxyWorker::Runing)
            return false;

        const QString &filePath = file.path();

        // Deduplication
        if (m_tmpSearchResults.contains(filePath)) {
            qCDebug(logDaemon) << "Duplicate OCR result ignored:" << filePath;
            continue;
        }

        if (m_resultCount >= MAX_SEARCH_NUM) {
            qCDebug(logDaemon) << "OCR result limit reached";
            break;
        }

        m_tmpSearchResults << filePath;

        // Create matched item
        QFileInfo fileInfo(filePath);
        QMimeType mimeType = SpecialTools::getMimeType(fileInfo);

        GrandSearch::MatchedItem item;
        item.item = filePath;
        item.name = fileInfo.fileName();
        item.type = mimeType.name();
        item.icon = mimeType.iconName();
        item.searcher = q_ptr->name();

        // Get OCR content and store in extra field
        QVariantHash extra;
        DFMSEARCH::SearchResult mutableResult = const_cast<DFMSEARCH::SearchResult &>(file);
        DFMSEARCH::OcrTextResultAPI ocrResult(mutableResult);
        QString ocrContent = ocrResult.ocrContent();

        if (!ocrContent.isEmpty()) {
            extra.insert(GRANDSEARCH_PROPERTY_ITEM_MATCHEDCONTEXT, ocrContent);
            qCDebug(logDaemon) << "OCR content found for file:" << filePath
                               << "Content length:" << ocrContent.length();
        }

        // Set weight method for file-like weight calculation
        extra.insert(GRANDSEARCH_PROPERTY_WEIGHT_METHOD,
                     GRANDSEARCH_PROPERTY_WEIGHT_METHOD_LOCALFILE);
        item.extra = extra;

        {
            QMutexLocker lk(&m_mutex);
            m_items.append(item);
            m_resultCount++;
        }

        qCDebug(logDaemon) << "OCR search result added - File:" << filePath
                           << "Count:" << m_resultCount;

        // Notify
        tryNotify();
    }

    qCInfo(logDaemon) << "OCR search completed - Results:" << m_resultCount
                      << "Time elapsed:" << m_time.elapsed() << "ms";

    return true;
}

bool OcrTextWorkerPrivate::searchByDFMSearch()
{
    qCDebug(logDaemon) << "Starting DFM OCR search with keyword:" << m_keyword;
    QObject holder;
    SearchEngine *engine = SearchFactory::createEngine(SearchType::Ocr, &holder);

    // Create search options
    SearchOptions options = createSearchOptions();
    // Create search query
    SearchQuery query = createSearchQuery();

    // Configure OCR options
    OcrTextOptionsAPI ocrOptions(options);
    // Enable filename-OCR content mixed search
    ocrOptions.setFilenameOcrContentMixedAndSearchEnabled(true);

    // Set search options
    engine->setSearchOptions(options);
    qCDebug(logDaemon) << "OCR search options configured on engine";

    // Execute search
    qCDebug(logDaemon) << "Executing synchronous OCR search";
    const SearchResultExpected &result = engine->searchSync(query);
    return processSearchResults(result);
}

void OcrTextWorkerPrivate::tryNotify()
{
    Q_Q(OcrTextWorker);
    int cur = m_time.elapsed();
    if (q->hasItem() && (cur - m_lastEmit) > EMIT_INTERVAL) {
        m_lastEmit = cur;
        qCDebug(logDaemon) << "Emitting new OCR results - Time elapsed:" << cur << "ms";
        emit q->unearthed(q);
    }
}

int OcrTextWorkerPrivate::itemCount() const
{
    QMutexLocker lk(&m_mutex);
    return m_items.size();
}

bool OcrTextWorkerPrivate::isResultLimit()
{
    return m_resultCount >= MAX_SEARCH_NUM;
}

OcrTextWorker::OcrTextWorker(const QString &name, QObject *parent)
    : ProxyWorker(name, parent),
      d_ptr(new OcrTextWorkerPrivate(this))
{
    qCDebug(logDaemon) << "OcrTextWorker constructor - Name:" << name;
}

OcrTextWorker::~OcrTextWorker()
{
    qCDebug(logDaemon) << "OcrTextWorker destructor called";
    delete d_ptr;
    d_ptr = nullptr;
}

void OcrTextWorker::setContext(const QString &context)
{
    Q_D(OcrTextWorker);

    if (context.isEmpty())
        qCWarning(logDaemon) << "OCR search key is empty";

    // Parse the keyword from context
    // Context may be JSON or plain text
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(context.toLocal8Bit(), &error);
    if (error.error != QJsonParseError::NoError || doc.isEmpty()) {
        d->m_keyword = context;
    } else {
        QJsonObject obj = doc.object();
        QJsonArray keywordArr = obj["keywords"].toArray();
        if (!keywordArr.isEmpty()) {
            d->m_keyword = keywordArr.first().toString();
        } else {
            d->m_keyword = context;
        }
    }

    qCDebug(logDaemon) << "Parsed OCR search keyword:" << d->m_keyword;
}

bool OcrTextWorker::isAsync() const
{
    return false;
}

bool OcrTextWorker::working(void *context)
{
    Q_D(OcrTextWorker);
    Q_UNUSED(context)

    qCDebug(logDaemon) << "OcrTextWorker starting work";

    if (!d->m_status.testAndSetRelease(Ready, Runing)) {
        qCWarning(logDaemon) << "Failed to start OCR worker - Invalid state transition";
        return false;
    }

    if (d->m_keyword.toLocal8Bit().size() < 2) {
        qCWarning(logDaemon) << "OCR search prerequisites not met - keyword: " << d->m_keyword;
        d->m_status.storeRelease(Completed);
        return false;
    }

    qCDebug(logDaemon) << "Starting OCR text search with keyword:" << d->m_keyword;
    d->m_time.start();

    if (!d->searchByDFMSearch()) {
        qCWarning(logDaemon) << "OCR search operation failed";
        return false;
    }

    // Check if there's still data
    if (d->m_status.testAndSetRelease(Runing, Completed)) {
        if (hasItem()) {
            qCDebug(logDaemon) << "OCR search completed - Emitting final results";
            emit unearthed(this);
        }
    }
    return true;
}

void OcrTextWorker::terminate()
{
    Q_D(OcrTextWorker);
    qCDebug(logDaemon) << "Terminating OCR text worker";
    d->m_status.storeRelease(Terminated);
}

ProxyWorker::Status OcrTextWorker::status()
{
    Q_D(OcrTextWorker);
    return static_cast<ProxyWorker::Status>(d->m_status.loadAcquire());
}

bool OcrTextWorker::hasItem() const
{
    Q_D(const OcrTextWorker);
    QMutexLocker lk(&d->m_mutex);
    return !d->m_items.isEmpty();
}

MatchedItemMap OcrTextWorker::takeAll()
{
    Q_D(OcrTextWorker);

    qCDebug(logDaemon) << "Taking all OCR search results";

    MatchedItemMap ret;

    QMutexLocker lk(&d->m_mutex);
    if (!d->m_items.isEmpty()) {
        MatchedItems items = std::move(d->m_items);
        Q_ASSERT(d->m_items.isEmpty());
        ret.insert(GRANDSEARCH_GROUP_FILE_OCR, items);
    }
    lk.unlock();

    return ret;
}
