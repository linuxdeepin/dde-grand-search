// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fulltextengine_p.h"

#include "lucene/chineseanalyzer.h"

// Lucune++ headers
#include <FileUtils.h>
#include <FilterIndexReader.h>
#include <FuzzyQuery.h>
#include <QueryWrapperFilter.h>

#include <QFileInfo>
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logToolFullText, "org.deepin.dde.grandsearch.tool.fulltext")
using namespace GrandSearch;
using namespace Lucene;

FullTextEnginePrivate::FullTextEnginePrivate(FullTextEngine *qq)
    : q(qq)
{
}

Lucene::IndexReaderPtr FullTextEnginePrivate::createReader(const QString &cache)
{
    return IndexReader::open(FSDirectory::open(cache.toStdWString()), true);
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
    QFileInfo info(cache);
    if (!info.isReadable()) {
        qCWarning(logToolFullText) << "Cache file is not readable - Path:" << cache;
        return false;
    } else if (d->m_reader) {
        qCCritical(logToolFullText) << "Full text engine already initialized";
        return false;
    }
    qCDebug(logToolFullText) << "Initializing full text cache - Path:" << cache;
    d->m_reader = d->createReader(cache);
    return d->m_reader.get() != nullptr;
}

void FullTextEngine::query(const QString &searchPath, const QStringList &keys, CheckAndPushItem func, void *pdata)
{
    if (d->m_reader.get() == nullptr || func == nullptr || searchPath.isEmpty())
        return;

    QString key = keys.join(' ').trimmed();
    qCDebug(logToolFullText) << "Performing full text search - Keywords:" << key << "Directory:" << searchPath;
    if (key.isEmpty())
        return;

    try {
        SearcherPtr searcher = newLucene<IndexSearcher>(d->m_reader);
        AnalyzerPtr analyzer = newLucene<ChineseAnalyzer>();
        QueryParserPtr parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"contents", analyzer);

        QueryPtr query = parser->parse(key.toStdWString());
        String filterPath = searchPath.endsWith("/") ? (searchPath + "*").toStdWString() : (searchPath + "/*").toStdWString();
        FilterPtr filter = newLucene<QueryWrapperFilter>(newLucene<WildcardQuery>(newLucene<Term>(L"path", filterPath)));
        TopDocsPtr topDocs = searcher->search(query, filter, 200);
        Collection<ScoreDocPtr> scoreDocs = topDocs->scoreDocs;

        // for get matched keys
        FormatterPtr simple(new KeyFormatter);
        QueryScorerPtr score(new QueryScorer(query));
        HighlighterPtr lighter(new Highlighter(simple, score));
        FragmenterPtr frag(new SimpleFragmenter(0));
        lighter->setTextFragmenter(frag);

        for (auto scoreDoc : scoreDocs) {
            DocumentPtr doc = searcher->doc(scoreDoc->doc);
            String resultPath = doc->get(L"path");
            QString filePath = StringUtils::toUTF8(resultPath).c_str();

            if (!QFile::exists(filePath))
                continue;

            FullTextEnginePrivate::KeyContext ctx {
                analyzer, simple, lighter, doc
            };
            if (!func(filePath, pdata, &ctx))
                return;   // 中断
        }
    } catch (const LuceneException &e) {
        qCWarning(logToolFullText) << "Lucene search error - Details:" << QString::fromStdWString(e.getError());
    } catch (const std::exception &e) {
        qCWarning(logToolFullText) << "Search error - Details:" << QString(e.what());
    } catch (...) {
        qCWarning(logToolFullText) << "Search failed - Unknown error occurred";
    }
}

QSet<QString> FullTextEngine::matchedKeys(void *ctx) const
{
    if (!ctx)
        return {};
    FullTextEnginePrivate::KeyContext *kctx =
            static_cast<FullTextEnginePrivate::KeyContext *>(ctx);
    auto format = kctx->keyFormatter();
    format->clear();
    kctx->lighter->getBestFragments(kctx->analyzer, L"contents", kctx->doc->get(L"contents"), 50);
    return format->keys();
}
