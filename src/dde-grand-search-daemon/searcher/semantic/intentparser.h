// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INTENTPARSER_H
#define INTENTPARSER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariantMap>
#include <QPair>
#include <QList>
#include <memory>

#include <dfm-search/dsearch_global.h>
DFM_SEARCH_USE_NS

class SemanticRuleEngine;
class DimensionExtractor;

/**
 * @brief 已消费的文本区间
 *
 * 记录解析过程中各维度匹配消费的文本位置，
 * 用于关键词提取时精确剔除已识别部分。
 */
struct MatchSpan
{
    int start;   // 起始位置（含）
    int end;   // 结束位置（不含）
    QString ruleId;   // 匹配的规则ID

    MatchSpan()
        : start(0), end(0) { }
    MatchSpan(int s, int e, const QString &id = QString())
        : start(s), end(e), ruleId(id) { }
};

/**
 * @brief 时间约束类型
 *
 * 与 dfm-search TimeRangeFilter 的预设方法直接对应：
 * - Preset 类型直接映射到 setToday()/setYesterday() 等
 * - Relative 类型映射到 setLast(value, unit)
 * - Custom 类型映射到 setRange(start, end)
 */
struct TimeConstraint
{
    /**
     * @brief 时间约束的种类
     */
    enum class Kind {
        Invalid,   // 无效/未设置
        Preset,   // 预设时间段（today, yesterday, this_week 等）
        Relative,   // 相对时间（最近 N 天/周/月）
        Custom   // 自定义时间范围
    };

    /**
     * @brief 预设时间段标识
     *
     * 与 TimeRangeFilter 的 setToday()/setThisWeek() 等方法一一对应
     */
    enum class Preset {
        None,
        Today,
        Yesterday,
        ThisWeek,
        LastWeek,
        ThisMonth,
        LastMonth,
        ThisYear,
        LastYear
    };

    Kind kind = Kind::Invalid;

    // Preset 类型使用
    Preset preset = Preset::None;

    // Relative 类型使用
    int relativeValue = 0;   // 数量（如 "最近3天" 中的 3）
    DFMSEARCH::TimeUnit relativeUnit = DFMSEARCH::TimeUnit::Days;   // 直接使用 dfm-search 枚举

    // Custom 类型使用
    QDateTime customStart;
    QDateTime customEnd;

    TimeConstraint() = default;

    /**
     * @brief 创建预设时间约束
     */
    static TimeConstraint makePreset(Preset p)
    {
        TimeConstraint tc;
        tc.kind = Kind::Preset;
        tc.preset = p;
        return tc;
    }

    /**
     * @brief 创建相对时间约束
     */
    static TimeConstraint makeRelative(int value, DFMSEARCH::TimeUnit unit)
    {
        TimeConstraint tc;
        tc.kind = Kind::Relative;
        tc.relativeValue = value;
        tc.relativeUnit = unit;
        return tc;
    }

    /**
     * @brief 创建自定义时间范围约束
     */
    static TimeConstraint makeCustom(const QDateTime &start, const QDateTime &end)
    {
        TimeConstraint tc;
        tc.kind = Kind::Custom;
        tc.customStart = start;
        tc.customEnd = end;
        return tc;
    }

    /**
     * @brief 是否有效
     */
    bool isValid() const { return kind != Kind::Invalid; }
};

/**
 * @brief 解析后的搜索意图
 */
struct ParsedIntent
{
    TimeConstraint timeConstraint;   // 时间约束
    QStringList fileTypes;   // 文件类型（扩展名列表）
    QStringList keywords;   // 关键词
    QVariantMap metadata;   // 其他元数据
    QString rawInput;   // 原始输入
    QList<MatchSpan> consumedSpans;   // 已消费的文本区间

    ParsedIntent() { }

    /**
     * @brief 检查是否有时间约束
     */
    bool hasTimeConstraint() const
    {
        return timeConstraint.isValid();
    }

    /**
     * @brief 检查是否有文件类型约束
     */
    bool hasFileTypes() const
    {
        return !fileTypes.isEmpty();
    }

    /**
     * @brief 检查是否有关键词
     */
    bool hasKeywords() const
    {
        return !keywords.isEmpty();
    }

    /**
     * @brief 转换为字符串（用于调试）
     */
    QString toString() const;
};

/**
 * @brief 意图解析器
 *
 * 将自然语言查询解析为结构化的意图对象
 */
class IntentParser : public QObject
{
    Q_OBJECT

public:
    explicit IntentParser(SemanticRuleEngine *ruleManager, QObject *parent = nullptr);
    ~IntentParser() override;

    /**
     * @brief 解析用户输入
     * @param userInput 用户输入的自然语言查询
     * @return 解析后的意图对象
     */
    ParsedIntent parse(const QString &userInput);

    /**
     * @brief 添加自定义维度提取器
     *
     * 提取器按添加顺序执行。关键词提取器应最后添加，
     * 因为它依赖前面提取器记录的已消费区间。
     * @param extractor 提取器（所有权转移给 IntentParser）
     */
    void addExtractor(std::unique_ptr<DimensionExtractor> extractor);

private:
    SemanticRuleEngine *m_ruleManager;
    QList<std::shared_ptr<DimensionExtractor>> m_extractors;   // 维度提取器列表

    /**
     * @brief 初始化默认提取器（时间、文件类型、关键词）
     */
    void initDefaultExtractors();
};

#endif   // INTENTPARSER_H
