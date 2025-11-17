// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "featurelibengine_p.h"

#include "lucene/chineseanalyzer.h"

// Lucune++ headers
#include <FileUtils.h>
#include <FilterIndexReader.h>
#include <FuzzyQuery.h>
#include <QueryWrapperFilter.h>
#include <PerFieldAnalyzerWrapper.h>

#include <QFileInfo>
#include <QDebug>
#include <QDateTime>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logToolFullText)

using namespace GrandSearch;
using namespace Lucene;

Q_DECLARE_METATYPE(FeatureLibEngine::QueryConditons)

FeatureLibEnginePrivate::FeatureLibEnginePrivate(FeatureLibEngine *qq)
    : q(qq)
{
    qCDebug(logToolFullText) << "FeatureLibEnginePrivate created";
}

int FeatureLibEnginePrivate::conditonsToString(const FeatureLibEngine::QueryConditons &cond, QString &out) const
{
    qCDebug(logToolFullText) << "Converting query conditions to string - Condition count:" << cond.size();

    int mul = 0;
    QString chain;
    for (const FeatureLibEngine::QueryProperty &prop : cond) {
        switch (prop.first) {
        case FeatureLibEngine::And:
            chain.append(" AND ");
            mul++;
            qCDebug(logToolFullText) << "Added AND operator to query chain";
            break;
        case FeatureLibEngine::Or:
            chain.append(" OR ");
            mul++;
            qCDebug(logToolFullText) << "Added OR operator to query chain";
            break;
        case FeatureLibEngine::Composite: {
            QString tmp;
            int subConditions = conditonsToString(prop.second.value<FeatureLibEngine::QueryConditons>(), tmp);
            if (subConditions > 0)
                tmp = QString("(%0)").arg(tmp);
            chain.append(tmp);
            qCDebug(logToolFullText) << "Added composite condition with" << subConditions << "sub-conditions";
        } break;
        case FeatureLibEngine::Path:
            qCDebug(logToolFullText) << "Path condition encountered but not implemented";
            break;
        case FeatureLibEngine::FileType: {
            const QString &fileTypeCondition = packageString("fileType", prop.second.toStringList());
            chain.append(fileTypeCondition);
            qCDebug(logToolFullText) << "Added file type condition:" << fileTypeCondition;
        } break;
        case FeatureLibEngine::CreatedTime:
            qCDebug(logToolFullText) << "Created time condition encountered but not implemented";
            break;
        case FeatureLibEngine::ModifiedTime: {
            const QString &timeCondition = packageTime("lastModified", prop.second.value<QList<QPair<qint64, qint64>>>());
            chain.append(timeCondition);
            qCDebug(logToolFullText) << "Added modified time condition:" << timeCondition;
        } break;
        case FeatureLibEngine::ReadTime:
            qCDebug(logToolFullText) << "Read time condition encountered but not implemented";
            break;
        case FeatureLibEngine::Album: {
            const QString &albumCondition = packageString("Album", prop.second.toStringList());
            chain.append(albumCondition);
            qCDebug(logToolFullText) << "Added album condition:" << albumCondition;
        } break;
        case FeatureLibEngine::Author: {
            const QString &authorCondition = packageString("Author", prop.second.toStringList());
            chain.append(authorCondition);
            qCDebug(logToolFullText) << "Added author condition:" << authorCondition;
        } break;
        case FeatureLibEngine::Duration: {
            const QString &durationCondition = packageString("duration", prop.second.toStringList());
            chain.append(durationCondition);
            qCDebug(logToolFullText) << "Added duration condition:" << durationCondition;
        } break;
        case FeatureLibEngine::Resolution: {
            const QString &resolutionCondition = packageString("resolution", prop.second.toStringList());
            chain.append(resolutionCondition);
            qCDebug(logToolFullText) << "Added resolution condition:" << resolutionCondition;
        } break;
        case FeatureLibEngine::Text: {
            const QString &textCondition = packageString("contents", prop.second.toStringList());
            chain.append(textCondition);
            qCDebug(logToolFullText) << "Added text content condition:" << textCondition;
        } break;
        default:
            qCWarning(logToolFullText) << "Unknown query condition type encountered:" << prop.first;
            break;
        }
    }

    out.append(chain);
    qCDebug(logToolFullText) << "Query condition conversion completed - Result:" << out << "Operators:" << mul;
    return mul;
}

QString FeatureLibEnginePrivate::packageString(const QString &key, const QStringList &value) const
{
    Q_ASSERT(!key.isEmpty());
    qCDebug(logToolFullText) << "Packaging string condition - Key:" << key << "Values:" << value;

    QString ret;
    const int size = value.size();
    int count = 0;
    for (int i = 0; i < size; ++i) {
        QString str = value.at(i).trimmed();
        if (str.isEmpty()) {
            qCDebug(logToolFullText) << "Skipping empty value at index" << i;
            continue;
        }

        ++count;
        ret.append(QString("%0:%1").arg(key).arg(str));
        if (i != (size - 1))
            ret.append(" OR ");
    }

    // add () for multiple conditions
    if (count > 1)
        ret = QString("(%0)").arg(ret);

    qCDebug(logToolFullText) << "String condition packaged - Result:" << ret << "Valid values:" << count;
    return ret;
}

QString FeatureLibEnginePrivate::packageTime(const QString &key, const QList<QPair<qint64, qint64>> &value) const
{
    Q_ASSERT(!key.isEmpty());
    qCDebug(logToolFullText) << "Packaging time condition - Key:" << key << "Intervals:" << value.size();

    QString ret;
    const QString format("yyyyMMddhhmmss");
    const int size = value.size();
    int count = 0;
    for (int i = 0; i < size; ++i) {
        QPair<qint64, qint64> interval = value.at(i);
        if (interval.first < 0) {
            qCDebug(logToolFullText) << "Normalizing negative start time to 0 for interval" << i;
            interval.first = 0;
        }

        if (interval.second < 0) {
            qCDebug(logToolFullText) << "Using current time for negative end time in interval" << i;
            interval.second = QDateTime::currentSecsSinceEpoch();
        }

        ret.append(QString("%0:[%1 TO %2]").arg(key).arg(QDateTime::fromSecsSinceEpoch(interval.first).toString(format)).arg(QDateTime::fromSecsSinceEpoch(interval.second).toString(format)));
        if (i != (size - 1))
            ret.append(" OR ");
        ++count;
    }
    // add () for multiple conditions
    if (count > 1)
        ret = QString("(%0)").arg(ret);

    qCDebug(logToolFullText) << "Time condition packaged - Result:" << ret << "Valid intervals:" << count;
    return ret;
}

IndexReaderPtr FeatureLibEnginePrivate::createReader(const QString &cache)
{
    return IndexReader::open(FSDirectory::open(cache.toStdWString()), true);
}

FeatureLibEngine::FeatureLibEngine(QObject *parent)
    : QObject(parent), d(new FeatureLibEnginePrivate(this))
{
    qCDebug(logToolFullText) << "FeatureLibEngine constructed";
}

FeatureLibEngine::~FeatureLibEngine()
{
    qCDebug(logToolFullText) << "FeatureLibEngine destructor called";
    delete d;
    d = nullptr;
}

bool FeatureLibEngine::init(const QString &cache)
{
    qCDebug(logToolFullText) << "Initializing FeatureLibEngine with cache:" << cache;
    QFileInfo info(cache);
    if (!info.isReadable()) {
        qCWarning(logToolFullText) << "Cache file is not readable - Path:" << cache;
        return false;
    } else if (d->m_reader) {
        qCCritical(logToolFullText) << "Feature library engine already initialized";
        return false;
    }

    qCDebug(logToolFullText) << "Initializing feature library cache - Path:" << cache;
    d->m_reader = d->createReader(cache);
    return d->m_reader.get() != nullptr;
}

void FeatureLibEngine::query(const QString &searchPath, const QueryConditons &cond, CheckAndPushItem func, void *pdata)
{
    qCDebug(logToolFullText) << "Starting feature library query - Path:" << searchPath << "Conditions:" << cond.size();

    // Validate parameters
    if (d->m_reader.get() == nullptr) {
        qCCritical(logToolFullText) << "Index reader is null - Engine not initialized";
        return;
    }

    if (func == nullptr) {
        qCCritical(logToolFullText) << "Callback function is null";
        return;
    }

    if (searchPath.isEmpty()) {
        qCWarning(logToolFullText) << "Search path is empty";
        return;
    }

    QString key;
    d->conditonsToString(cond, key);
    qCDebug(logToolFullText) << "Performing feature search - Conditions:" << key << "Directory:" << searchPath;
    if (key.isEmpty()) {
        qCWarning(logToolFullText) << "Query conditions resulted in empty search key";
        return;
    }

    try {
        SearcherPtr searcher = newLucene<IndexSearcher>(d->m_reader);

        // default analyzer: do not analyze
        AnalyzerPtr defaultAnalyzer = newLucene<KeywordAnalyzer>();
        PerFieldAnalyzerWrapperPtr analyzer = newLucene<PerFieldAnalyzerWrapper>(defaultAnalyzer);

        // the keys need analyze
        analyzer->addAnalyzer(L"contents", newLucene<ChineseAnalyzer>());
        analyzer->addAnalyzer(L"Album", newLucene<ChineseAnalyzer>());
        analyzer->addAnalyzer(L"Author", newLucene<ChineseAnalyzer>());

        QueryParserPtr parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"contents", analyzer);

        QueryPtr query = parser->parse(key.toStdWString());
        String filterPath = searchPath.endsWith("/") ? (searchPath + "*").toStdWString() : (searchPath + "/*").toStdWString();
        FilterPtr filter = newLucene<QueryWrapperFilter>(newLucene<WildcardQuery>(newLucene<Term>(L"path", filterPath)));
        TopDocsPtr topDocs = searcher->search(query, filter, 100);
        Collection<ScoreDocPtr> scoreDocs = topDocs->scoreDocs;
        qCDebug(logToolFullText) << "Search completed - Found" << scoreDocs.size() << "results";

        // for get matched keys
#if 0
        FormatterPtr simple(new KeyFormatter);
        auto format = dynamic_cast<KeyFormatter *>(simple.get());
        QueryScorerPtr score(new QueryScorer(query));
        HighlighterPtr lighter(new Highlighter(simple, score));
        FragmenterPtr frag(new SimpleFragmenter(0));
        lighter->setTextFragmenter(frag);
#endif
        for (auto scoreDoc : scoreDocs) {
            DocumentPtr doc = searcher->doc(scoreDoc->doc);
            String resultPath = doc->get(L"path");
            QString filePath = StringUtils::toUTF8(resultPath).c_str();

            QSet<QString> match;
#if 0
            format->clear();
            lighter->getBestFragments(analyzer, L"contents", doc->get(L"contents"), 50);
            match = format->keys();
#endif
            if (!QFile::exists(filePath))
                continue;
            if (!func(filePath, match, pdata))
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
