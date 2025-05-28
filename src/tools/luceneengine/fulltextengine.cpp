// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fulltextengine_p.h"

#include <dfm-search/searchfactory.h>
#include <dfm-search/contentsearchapi.h>
#include <dfm-search/searchoptions.h>
#include <dfm-search/searchquery.h>
#include <dfm-search/dsearch_global.h>

#include <QFileInfo>
#include <QDebug>
#include <QLoggingCategory>
#include <QRegularExpression>
#include <QDir>

Q_LOGGING_CATEGORY(logToolFullText, "org.deepin.dde.grandsearch.tool.fulltext")
using namespace GrandSearch;
using namespace DFMSEARCH;

FullTextEnginePrivate::FullTextEnginePrivate(FullTextEngine *qq)
    : q(qq)
{
}

FullTextEnginePrivate::~FullTextEnginePrivate()
{
    if (m_engineHolder) {
        delete m_engineHolder;
        m_engineHolder = nullptr;
        m_engine = nullptr;
    }
}

bool FullTextEnginePrivate::isContentIndexAvailable() const
{
    return DFMSEARCH::Global::isContentIndexAvailable();
}

QSet<QString> FullTextEnginePrivate::extractMatchedKeys(const QString &content, const QStringList &keywords) const
{
    QSet<QString> matchedKeys;

    for (const QString &keyword : keywords) {
        if (keyword.isEmpty())
            continue;

        // Use case-insensitive matching to find keywords in content
        QRegularExpression regex(QRegularExpression::escape(keyword),
                                 QRegularExpression::CaseInsensitiveOption);
        if (regex.match(content).hasMatch()) {
            matchedKeys.insert(keyword);
        }
    }

    return matchedKeys;
}

FullTextEngine::FullTextEngine(QObject *parent)
    : QObject(parent), d(new FullTextEnginePrivate(this))
{
}

FullTextEngine::~FullTextEngine()
{
    delete d;
    d = nullptr;
}

bool FullTextEngine::init(const QString &cache)
{
    Q_UNUSED(cache)   // dfm-search manages its own index, cache parameter not needed

    if (d->m_engine) {
        qCWarning(logToolFullText) << "Full text engine already initialized";
        return false;
    }

    if (!d->isContentIndexAvailable()) {
        qCWarning(logToolFullText) << "Content index is not available";
        return false;
    }

    qCDebug(logToolFullText) << "Initializing dfm-search content engine";

    // Create search engine for content search
    d->m_engineHolder = new QObject();
    d->m_engine = SearchFactory::createEngine(SearchType::Content, d->m_engineHolder);

    if (!d->m_engine) {
        qCWarning(logToolFullText) << "Failed to create dfm-search content engine";
        delete d->m_engineHolder;
        d->m_engineHolder = nullptr;
        return false;
    }

    qCDebug(logToolFullText) << "Full text content engine initialized successfully";
    return true;
}

void FullTextEngine::query(const QString &searchPath, const QStringList &keys, CheckAndPushItem func, void *pdata)
{
    if (!d->m_engine || !func || searchPath.isEmpty()) {
        qCWarning(logToolFullText) << "Invalid parameters for content search - Engine:"
                                   << (d->m_engine != nullptr) << "Func:" << (func != nullptr)
                                   << "Path:" << searchPath;
        return;
    }

    if (keys.isEmpty()) {
        qCDebug(logToolFullText) << "No keywords provided for content search";
        return;
    }

    QString keyword = keys.join(' ').trimmed();
    qCDebug(logToolFullText) << "Performing content search - Keywords:" << keyword
                             << "Directory:" << searchPath;

    // Create search options for content search
    SearchOptions options;
    options.setSearchPath(searchPath);
    options.setSearchMethod(SearchMethod::Indexed);
    options.setMaxResults(200);

    // Set search options
    d->m_engine->setSearchOptions(options);

    // Create search query
    SearchQuery query;
    if (keys.size() > 1) {
        query = SearchFactory::createQuery(keys, SearchQuery::Type::Boolean);
        query.setBooleanOperator(SearchQuery::BooleanOperator::AND);
    } else {
        query = SearchFactory::createQuery(keyword, SearchQuery::Type::Simple);
    }

    try {
        // Execute search with callback
        d->m_engine->searchWithCallback(query, [this, func, pdata, keys](const SearchResult &result) -> bool {
            QString filePath = result.path();

            if (!QFile::exists(filePath)) {
                qCDebug(logToolFullText) << "File not found:" << filePath;
                return true;   // Continue search
            }

            // Create context for matched keys
            FullTextEnginePrivate::ContentContext context;
            context.keywords = keys;

            // Get highlighted content if available
            ContentResultAPI contentResult(const_cast<SearchResult &>(result));
            context.highlightedContent = contentResult.highlightedContent();

            // Extract matched keys from content or highlighted content
            QString contentForMatching = context.highlightedContent.isEmpty()
                    ? keys.first()
                    : context.highlightedContent;
            context.matchedKeys = d->extractMatchedKeys(contentForMatching, keys);

            qCDebug(logToolFullText) << "Found content match - File:" << filePath
                                     << "Matched keys:" << context.matchedKeys;

            // Call the callback function
            bool shouldContinue = func(filePath, pdata, &context);
            return shouldContinue;
        });
    } catch (const std::exception &e) {
        qCWarning(logToolFullText) << "Content search error - Details:" << QString(e.what());
    } catch (...) {
        qCWarning(logToolFullText) << "Content search failed - Unknown error occurred";
    }
}

QSet<QString> FullTextEngine::matchedKeys(void *ctx) const
{
    if (!ctx) {
        qCDebug(logToolFullText) << "No context provided for matched keys";
        return {};
    }

    FullTextEnginePrivate::ContentContext *context =
            static_cast<FullTextEnginePrivate::ContentContext *>(ctx);

    return context->matchedKeys;
}
