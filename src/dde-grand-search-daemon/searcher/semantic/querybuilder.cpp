// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "querybuilder.h"

#include <dfm-search/searchfactory.h>
#include <dfm-search/timerangefilter.h>

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

DFM_SEARCH_USE_NS

QueryBuilder::QueryBuilder(QObject *parent)
    : QObject(parent),
      m_searchPath(QDir::homePath()),
      m_maxResults(100),
      m_fulltextEnabled(false),
      m_ocrSearchEnabled(false)
{
}

QueryBuilder::~QueryBuilder()
{
}

SemanticSearchQuery QueryBuilder::build(const ParsedIntent &intent)
{
    SemanticSearchQuery result;

    // 保存原始意图信息
    result.keywords = intent.keywords;
    result.fileTypes = intent.fileTypes;

    // 预先构建时间范围过滤器（避免在每个子方法中重复构建）
    TimeRangeFilter timeFilter;
    if (intent.hasTimeConstraint()) {
        timeFilter = buildTimeRangeFilter(intent);
    }

    // 构建文件名搜索（始终启用）
    buildFileNameSearch(intent, timeFilter, result);

    // 如果有关键词且关键词长度足够，也构建内容搜索（需要全文搜索启用）
    if (m_fulltextEnabled && intent.hasKeywords()) {
        bool hasLongKeyword = false;
        for (const QString &kw : intent.keywords) {
            if (kw.length() >= DFMSEARCH::Global::kMinContentSearchKeywordLength) {
                hasLongKeyword = true;
                break;
            }
        }
        if (hasLongKeyword) {
            buildContentSearch(intent, timeFilter, result);
        }
    }

    // 构建 OCR 文本搜索（需要 OCR 索引可用且启用）
    if (m_ocrSearchEnabled && intent.hasKeywords()) {
        buildOcrSearch(intent, timeFilter, result);
    }

    return result;
}

void QueryBuilder::buildFileNameSearch(const ParsedIntent &intent, const TimeRangeFilter &timeFilter, SemanticSearchQuery &result)
{
    if (!intent.hasKeywords() && !intent.hasFileTypes() && !intent.hasTimeConstraint()) {
        return;
    }

    result.hasFileNameSearch = true;
    result.fileNameOptions = createBaseOptions();
    result.fileNameQuery = intent.hasKeywords()
            ? buildKeywordQuery(intent.keywords)
            : SearchFactory::createQuery("");

    // 配置文件名搜索选项
    FileNameOptionsAPI fileNameApi(result.fileNameOptions);
    fileNameApi.setPinyinEnabled(true);
    fileNameApi.setPinyinAcronymEnabled(true);

    // 设置文件类型过滤
    if (intent.hasFileTypes()) {
        fileNameApi.setFileExtensions(intent.fileTypes);
    } else if (intent.hasTimeConstraint()) {
        static QStringList types { "app", "archive", "audio",
                                   "doc", "pic", "video", "dir" };
        fileNameApi.setFileTypes(types);
    }

    // 使用 TimeRangeFilter 替代后过滤（在索引层面过滤，性能更优）
    if (timeFilter.isValid()) {
        result.fileNameOptions.setTimeRangeFilter(timeFilter);
    }
}

void QueryBuilder::buildContentSearch(const ParsedIntent &intent, const TimeRangeFilter &timeFilter, SemanticSearchQuery &result)
{
    if (!intent.hasKeywords()) {
        return;
    }

    if (!DFMSEARCH::Global::isContentIndexAvailable()) {
        qCDebug(logDaemon) << "QueryBuilder: Content index not available, skipping content search";
        return;
    }

    result.hasContentSearch = true;
    result.contentOptions = createBaseOptions();

    // 内容搜索使用索引方式
    result.contentOptions.setSearchMethod(SearchMethod::Indexed);

    // 构建内容搜索查询
    result.contentQuery = buildKeywordQuery(intent.keywords);

    // 配置内容搜索选项
    ContentOptionsAPI contentApi(result.contentOptions);

    // 使用 TimeRangeFilter 在索引层面过滤
    if (timeFilter.isValid()) {
        result.contentOptions.setTimeRangeFilter(timeFilter);
    }

    qCDebug(logDaemon) << "QueryBuilder: content search configured"
                       << "keywords:" << intent.keywords;
}

void QueryBuilder::buildOcrSearch(const ParsedIntent &intent, const TimeRangeFilter &timeFilter, SemanticSearchQuery &result)
{
    if (!intent.hasKeywords()) {
        return;
    }

    // 检查 OCR 索引是否可用
    if (!DFMSEARCH::Global::isOcrTextIndexAvailable()) {
        qCDebug(logDaemon) << "QueryBuilder: OCR text index not available, skipping OCR search";
        return;
    }

    result.hasOcrSearch = true;
    result.ocrOptions = createBaseOptions();

    // OCR 搜索使用索引方式
    result.ocrOptions.setSearchMethod(SearchMethod::Indexed);

    // 构建 OCR 搜索查询
    result.ocrQuery = buildKeywordQuery(intent.keywords);

    // 配置 OCR 搜索选项
    OcrTextOptionsAPI ocrApi(result.ocrOptions);

    // 使用 TimeRangeFilter 在索引层面过滤
    if (timeFilter.isValid()) {
        result.ocrOptions.setTimeRangeFilter(timeFilter);
    }
    qCDebug(logDaemon) << "QueryBuilder: OCR search configured"
                       << "keywords:" << intent.keywords;
}

TimeRangeFilter QueryBuilder::buildTimeRangeFilter(const ParsedIntent &intent) const
{
    TimeRangeFilter filter;

    if (!intent.hasTimeConstraint()) {
        return filter;
    }

    const TimeConstraint &tc = intent.timeConstraint;

    // 默认按修改时间过滤
    filter.setTimeField(TimeField::BirthTime);

    switch (tc.kind) {
    case TimeConstraint::Kind::Preset: {
        // 预设时间段：直接调用 TimeRangeFilter 对应的预设方法
        switch (tc.preset) {
        case TimeConstraint::Preset::Today:
            filter.setToday();
            break;
        case TimeConstraint::Preset::Yesterday:
            filter.setYesterday();
            break;
        case TimeConstraint::Preset::ThisWeek:
            filter.setThisWeek();
            break;
        case TimeConstraint::Preset::LastWeek:
            filter.setLastWeek();
            break;
        case TimeConstraint::Preset::ThisMonth:
            filter.setThisMonth();
            break;
        case TimeConstraint::Preset::LastMonth:
            filter.setLastMonth();
            break;
        case TimeConstraint::Preset::ThisYear:
            filter.setThisYear();
            break;
        case TimeConstraint::Preset::LastYear:
            filter.setLastYear();
            break;
        default:
            return TimeRangeFilter();
        }
        break;
    }
    case TimeConstraint::Kind::Relative: {
        // 相对时间：relativeUnit 已经是 DFMSEARCH::TimeUnit，直接使用
        filter.setLast(tc.relativeValue, tc.relativeUnit);
        break;
    }
    case TimeConstraint::Kind::Custom: {
        // 自定义时间范围
        filter.setRange(tc.customStart, tc.customEnd);
        filter.setIncludeLower(true);
        filter.setIncludeUpper(true);
        break;
    }
    default:
        return TimeRangeFilter();
    }

    auto range = filter.resolveTimeRange();
    qCDebug(logDaemon) << "QueryBuilder: built TimeRangeFilter"
                       << "start:" << range.first
                       << "end:" << range.second;

    return filter;
}

SearchOptions QueryBuilder::createBaseOptions() const
{
    SearchOptions options;
    options.setSearchPath(m_searchPath);

    // 检查文件名索引目录是否可用
    if (DFMSEARCH::Global::isFileNameIndexDirectoryAvailable()) {
        options.setSearchMethod(SearchMethod::Indexed);
    } else {
        options.setSearchMethod(SearchMethod::Realtime);
    }

    options.setMaxResults(m_maxResults);
    options.setIncludeHidden(false);

    return options;
}

SearchQuery QueryBuilder::buildKeywordQuery(const QStringList &keywords) const
{
    if (keywords.size() == 1) {
        const QString &keyword = keywords.first();
        SearchQuery::Type queryType = SearchQuery::Type::Simple;
        if (keyword.contains('*') || keyword.contains('?')) {
            queryType = SearchQuery::Type::Wildcard;
        }
        return SearchFactory::createQuery(keyword, queryType);
    }

    // 多关键词使用 Boolean AND 查询
    SearchQuery query = SearchFactory::createQuery(keywords, SearchQuery::Type::Boolean);
    query.setBooleanOperator(SearchQuery::BooleanOperator::AND);
    return query;
}

QString SemanticSearchQuery::toString() const
{
    QStringList parts;

    if (hasFileNameSearch) {
        QString fileNamePart = QString("FileName[keyword=%1]").arg(fileNameQuery.keyword());
        if (fileNameOptions.hasTimeRangeFilter()) {
            auto range = fileNameOptions.timeRangeFilter().resolveTimeRange();
            fileNamePart += QString("+Time[%1~%2]")
                                    .arg(range.first.toString("yyyy-MM-dd"))
                                    .arg(range.second.toString("yyyy-MM-dd"));
        }
        parts << fileNamePart;
    }

    if (hasContentSearch) {
        parts << QString("Content[keyword=%1]").arg(contentQuery.keyword());
    }

    if (hasOcrSearch) {
        parts << QString("OCR[keyword=%1]").arg(ocrQuery.keyword());
    }

    if (!fileTypes.isEmpty()) {
        parts << QString("Types[%1]").arg(fileTypes.join(","));
    }

    if (parts.isEmpty()) {
        return QStringLiteral("EmptyQuery");
    }

    return parts.join(" + ");
}
