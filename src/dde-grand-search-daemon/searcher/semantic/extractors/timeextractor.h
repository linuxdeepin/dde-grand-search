// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIMEEXTRACTOR_H
#define TIMEEXTRACTOR_H

#include "dimensionextractor.h"
#include "searcher/semantic/semanticruleengine.h"

#include <QDateTime>

struct TimeConstraint;

/**
 * @brief 时间维度提取器
 *
 * 从输入文本中提取时间约束，生成语义化的 TimeConstraint，
 * 与 dfm-search TimeRangeFilter 的预设方法直接对应。
 *
 * 不再手动计算 QDateTime 范围，而是保留语义信息，
 * 由 QueryBuilder 通过 TimeRangeFilter 的流式 API 完成映射。
 */
class TimeExtractor : public DimensionExtractor
{
public:
    explicit TimeExtractor(SemanticRuleEngine *engine)
        : DimensionExtractor(engine) { }

    void extract(const QString &input, ParsedIntent &intent) override;
    QString name() const override { return "TimeExtractor"; }

private:
    TimeConstraint parseTimeFromMetadata(const RegexRule &rule,
                                         const QRegularExpressionMatch &match);
};

#endif   // TIMEEXTRACTOR_H
