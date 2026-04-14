// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYWORDEXTRACTOR_H
#define KEYWORDEXTRACTOR_H

#include "dimensionextractor.h"
#include "searcher/semantic/semanticruleengine.h"

/**
 * @brief 关键词维度提取器
 *
 * 从输入文本中提取搜索关键词。
 * 策略1: 使用 keyword 规则组匹配结构化模式（如"包含XX"）
 * 策略2: 回退到利用已消费区间提取未识别文本
 */
class KeywordExtractor : public DimensionExtractor
{
public:
    explicit KeywordExtractor(SemanticRuleEngine *engine)
        : DimensionExtractor(engine) { }

    void extract(const QString &input, ParsedIntent &intent) override;
    QString name() const override { return "KeywordExtractor"; }

private:
    /**
     * @brief 提取未被消费的文本
     */
    QString extractUnconsumedText(const QString &input, const QList<MatchSpan> &spans);

    /**
     * @brief 清理文本（移除助词、标点等）
     */
    QString cleanText(const QString &text);

    /**
     * @brief 按连接词和逗号分割多关键词串
     *
     * 支持的分隔符：和、与、以及、或、或者、顿号、逗号
     * 如 "测试和报告" → ["测试", "报告"]
     *    "测试、报告、文档" → ["测试", "报告", "文档"]
     */
    QStringList splitMultiKeywords(const QString &text);
};

#endif   // KEYWORDEXTRACTOR_H
