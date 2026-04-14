// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QUERYBUILDER_H
#define QUERYBUILDER_H

#include <QObject>
#include <QString>
#include <QStringList>

#include <dfm-search/dsearch_global.h>
#include <dfm-search/searchquery.h>
#include <dfm-search/searchoptions.h>
#include <dfm-search/filenamesearchapi.h>
#include <dfm-search/contentsearchapi.h>
#include <dfm-search/ocrtextsearchapi.h>
#include <dfm-search/timerangefilter.h>

#include "intentparser.h"

DFM_SEARCH_USE_NS

/**
 * @brief 语义搜索查询结果
 *
 * 封装 dfmsearch 的 SearchQuery 和 SearchOptions，
 * 由 QueryBuilder 根据 ParsedIntent 构建。
 */
struct SemanticSearchQuery {
    DFMSEARCH::SearchQuery fileNameQuery;       // 文件名搜索查询
    DFMSEARCH::SearchOptions fileNameOptions;   // 文件名搜索选项
    DFMSEARCH::SearchQuery contentQuery;        // 内容搜索查询
    DFMSEARCH::SearchOptions contentOptions;    // 内容搜索选项
    DFMSEARCH::SearchQuery ocrQuery;            // OCR 文本搜索查询
    DFMSEARCH::SearchOptions ocrOptions;        // OCR 文本搜索选项
    bool hasFileNameSearch = false;             // 是否需要文件名搜索
    bool hasContentSearch = false;              // 是否需要内容搜索
    bool hasOcrSearch = false;                  // 是否需要 OCR 文本搜索

    // 原始意图信息（用于调试和日志）
    QStringList keywords;                       // 关键词列表
    QStringList fileTypes;                      // 文件类型列表

    /**
     * @brief 转换为字符串（用于调试）
     */
    QString toString() const;
};

/**
 * @brief 查询构建器
 *
 * 将解析后的意图转换为 dfmsearch 搜索查询
 */
class QueryBuilder : public QObject {
    Q_OBJECT

public:
    explicit QueryBuilder(QObject *parent = nullptr);
    ~QueryBuilder() override;

    /**
     * @brief 构建搜索查询
     * @param intent 解析后的意图
     * @return 语义搜索查询对象
     */
    SemanticSearchQuery build(const ParsedIntent &intent);

    /**
     * @brief 设置搜索路径
     * @param path 搜索路径
     */
    void setSearchPath(const QString &path) { m_searchPath = path; }

    /**
     * @brief 设置最大结果数
     * @param maxResults 最大结果数
     */
    void setMaxResults(int maxResults) { m_maxResults = maxResults; }

    /**
     * @brief 设置是否启用全文搜索
     * @param enabled 是否启用
     */
    void setFulltextEnabled(bool enabled) { m_fulltextEnabled = enabled; }

    /**
     * @brief 设置是否启用 OCR 文本搜索
     * @param enabled 是否启用
     */
    void setOcrSearchEnabled(bool enabled) { m_ocrSearchEnabled = enabled; }

private:
    QString m_searchPath;
    int m_maxResults;
    bool m_fulltextEnabled;
    bool m_ocrSearchEnabled;

    /**
     * @brief 构建文件名搜索查询和选项
     * @param intent 意图对象
     * @param timeFilter 预构建的时间范围过滤器
     * @param result 搜索查询结果（输出参数）
     */
    void buildFileNameSearch(const ParsedIntent &intent, const DFMSEARCH::TimeRangeFilter &timeFilter, SemanticSearchQuery &result);

    /**
     * @brief 构建内容搜索查询和选项
     * @param intent 意图对象
     * @param timeFilter 预构建的时间范围过滤器
     * @param result 搜索查询结果（输出参数）
     */
    void buildContentSearch(const ParsedIntent &intent, const DFMSEARCH::TimeRangeFilter &timeFilter, SemanticSearchQuery &result);

    /**
     * @brief 构建 OCR 文本搜索查询和选项
     * @param intent 意图对象
     * @param timeFilter 预构建的时间范围过滤器
     * @param result 搜索查询结果（输出参数）
     */
    void buildOcrSearch(const ParsedIntent &intent, const DFMSEARCH::TimeRangeFilter &timeFilter, SemanticSearchQuery &result);

    /**
     * @brief 根据意图的时间约束构建 TimeRangeFilter
     * @param intent 意图对象
     * @return 时间范围过滤器
     */
    DFMSEARCH::TimeRangeFilter buildTimeRangeFilter(const ParsedIntent &intent) const;

    /**
     * @brief 创建通用搜索选项
     * @return 搜索选项
     */
    DFMSEARCH::SearchOptions createBaseOptions() const;

    /**
     * @brief 构建搜索关键词查询
     * @param keywords 关键词列表
     * @return dfmsearch 搜索查询
     */
    DFMSEARCH::SearchQuery buildKeywordQuery(const QStringList &keywords) const;
};

#endif // QUERYBUILDER_H
