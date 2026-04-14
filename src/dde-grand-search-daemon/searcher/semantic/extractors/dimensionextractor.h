// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DIMENSIONEXTRACTOR_H
#define DIMENSIONEXTRACTOR_H

#include <QObject>
#include <QString>
#include <QList>

// 前向声明
struct ParsedIntent;
struct MatchSpan;
class SemanticRuleEngine;

/**
 * @brief 维度提取器基类
 *
 * 每个维度（时间、文件类型、关键词等）对应一个提取器，
 * 负责从输入文本中提取该维度的信息并记录消费区间。
 * 新增维度时只需继承此类，无需修改 IntentParser。
 */
class DimensionExtractor
{
public:
    explicit DimensionExtractor(SemanticRuleEngine *engine)
        : m_engine(engine) { }
    virtual ~DimensionExtractor() = default;

    /**
     * @brief 从输入文本中提取维度信息
     * @param input 原始输入文本
     * @param intent 解析意图（输出参数）
     */
    virtual void extract(const QString &input, ParsedIntent &intent) = 0;

    /**
     * @brief 获取提取器名称（用于调试）
     */
    virtual QString name() const = 0;

protected:
    SemanticRuleEngine *m_engine;
};

#endif   // DIMENSIONEXTRACTOR_H
