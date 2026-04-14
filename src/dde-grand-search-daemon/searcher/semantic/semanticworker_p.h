// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEMANTICWORKER_P_H
#define SEMANTICWORKER_P_H

#include "semanticworker.h"
#include "intentparser.h"
#include "querybuilder.h"
#include "semanticruleengine.h"
#include "config/ruleconfigloader.h"

#include <dfm-search/searchresult.h>

#include <memory>

namespace GrandSearch {

class SemanticWorkerPrivate
{
public:
    explicit SemanticWorkerPrivate(SemanticWorker *parent);
    ~SemanticWorkerPrivate();

    /**
     * @brief 初始化规则引擎和解析器
     */
    void initParser();

    /**
     * @brief 执行文件名搜索
     * @param query 搜索查询
     * @return 是否成功
     */
    bool searchByFileName(const SemanticSearchQuery &query);

    /**
     * @brief 执行内容搜索
     * @param query 搜索查询
     * @return 是否成功
     */
    bool searchByContent(const SemanticSearchQuery &query);

    /**
     * @brief 执行 OCR 文本搜索
     * @param query 搜索查询
     * @return 是否成功
     */
    bool searchByOcr(const SemanticSearchQuery &query);

    /**
     * @brief 处理搜索结果
     * @param result 搜索结果
     * @param isContentSearch 是否为内容搜索
     * @return 是否成功
     */
    bool processSearchResults(const DFMSEARCH::SearchResultExpected &result, bool isContentSearch = false);

    /**
     * @brief 添加单个搜索结果
     * @param filePath 文件路径
     * @param isContentSearch 是否为内容搜索结果
     * @return 是否添加成功
     */
    bool appendSearchResult(const QString &filePath, bool isContentSearch = false);

    /**
     * @brief 尝试通知结果
     */
    void tryNotify();

    /**
     * @brief 获取结果数量
     * @return 结果数量
     */
    int itemCount() const;

    /**
     * @brief 检查是否达到结果限制
     * @return 是否达到限制
     */
    bool isResultLimit() const;

public:
    SemanticWorker *q = nullptr;
    QAtomicInt m_status = ProxyWorker::Ready;
    QString m_context;

    // 意图解析相关
    std::unique_ptr<RuleConfigLoader> m_configLoader;
    std::unique_ptr<SemanticRuleEngine> m_ruleEngine;
    std::unique_ptr<IntentParser> m_parser;
    std::unique_ptr<QueryBuilder> m_queryBuilder;

    // 搜索结果
    mutable QMutex m_mutex;
    MatchedItemMap m_items;
    QSet<QString> m_tmpSearchResults;   // 用于去重

    // 引擎状态
    bool m_semanticEnabled = false;
    bool m_fulltextEnabled = false;
    bool m_ocrtextEnabled = false;

    // 计时
    QElapsedTimer m_time;
    int m_lastEmit = 0;

    // 配置
    static constexpr int MAX_RESULTS_TOTAL = 100;
    static constexpr int EMIT_INTERVAL = 50;
};

}

#endif // SEMANTICWORKER_P_H
