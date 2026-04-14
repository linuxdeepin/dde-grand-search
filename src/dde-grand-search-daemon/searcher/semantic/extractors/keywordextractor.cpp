// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "keywordextractor.h"
#include "searcher/semantic/intentparser.h"

#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)
#include <QRegularExpression>

#include <algorithm>

void KeywordExtractor::extract(const QString &input, ParsedIntent &intent)
{
    // 策略1: 优先使用 keyword 规则组匹配结构化的关键词模式
    // 如 "包含测试" → 提取 "测试"，"包含测试和报告" → 提取 ["测试", "报告"]
    QList<RegexRule> keywordRules = m_engine->matchAll("keyword", input);
    if (!keywordRules.isEmpty()) {
        for (const RegexRule &rule : keywordRules) {
            int captureGroup = rule.metadata.value("capture_group", 1).toInt();
            bool isMultiKeyword = rule.metadata.value("multi_keyword", false).toBool();
            QRegularExpressionMatch match = rule.pattern.match(input);
            if (match.hasMatch() && captureGroup <= match.lastCapturedIndex()) {
                QString captured = match.captured(captureGroup).trimmed();
                if (!captured.isEmpty()) {
                    // 如果规则标记了 multi_keyword，对捕获内容进行连接词/逗号分割
                    QStringList parts = isMultiKeyword
                            ? splitMultiKeywords(captured)
                            : QStringList { captured };
                    for (const QString &kw : parts) {
                        if (!kw.isEmpty() && !intent.keywords.contains(kw)) {
                            intent.keywords.append(kw);
                        }
                    }
                }
                // 记录整个关键词模式的消费区间
                intent.consumedSpans.append(MatchSpan(match.capturedStart(), match.capturedEnd(), rule.id));
            }
        }

        qCDebug(logDaemon) << "Detected keywords via keyword rules:" << intent.keywords;
        return;
    }

    // 策略2: 回退 - 利用已消费区间精确提取未识别的文本作为关键词
    QList<MatchSpan> allSpans = intent.consumedSpans;

    // 记录噪声词的消费区间
    QList<RegexRule> noiseRules = m_engine->matchAll("noise", input);
    for (const RegexRule &rule : noiseRules) {
        QRegularExpressionMatch noiseMatch = rule.pattern.match(input);
        if (noiseMatch.hasMatch()) {
            allSpans.append(MatchSpan(noiseMatch.capturedStart(), noiseMatch.capturedEnd(), rule.id));
        }
    }

    // 提取未被任何规则消费的文本
    QString remaining = extractUnconsumedText(input, allSpans);

    // 清理剩余文本
    remaining = cleanText(remaining);

    // 如果还有剩余文本，作为关键词
    if (!remaining.isEmpty()) {
        // 先按连接词和逗号分割，再按空格分割
        QStringList parts = splitMultiKeywords(remaining);
        for (const QString &part : parts) {
            QStringList subParts = part.split(' ', Qt::SkipEmptyParts);
            for (const QString &kw : subParts) {
                if (!kw.isEmpty() && !intent.keywords.contains(kw)) {
                    intent.keywords.append(kw);
                }
            }
        }

        qCDebug(logDaemon) << "Detected keywords from remaining text:" << intent.keywords;
    }
}

QString KeywordExtractor::extractUnconsumedText(const QString &input, const QList<MatchSpan> &spans)
{
    if (spans.isEmpty()) {
        return input;
    }

    // 按起始位置排序
    QList<MatchSpan> sorted = spans;
    std::sort(sorted.begin(), sorted.end(),
              [](const MatchSpan &a, const MatchSpan &b) {
                  return a.start < b.start;
              });

    // 合并重叠区间
    QList<MatchSpan> merged;
    merged.append(sorted.first());
    for (int i = 1; i < sorted.size(); ++i) {
        MatchSpan &last = merged.last();
        if (sorted[i].start <= last.end) {
            last.end = qMax(last.end, sorted[i].end);
        } else {
            merged.append(sorted[i]);
        }
    }

    // 提取未被消费的文本片段
    QString result;
    int pos = 0;
    for (const MatchSpan &span : merged) {
        if (pos < span.start) {
            result += input.mid(pos, span.start - pos);
        }
        pos = span.end;
    }
    if (pos < input.length()) {
        result += input.mid(pos);
    }

    return result;
}

QString KeywordExtractor::cleanText(const QString &text)
{
    QString result = text;

    // 移除助词
    QStringList particles = { "的", "了", "吗", "呢", "吧", "啊" };
    for (const QString &particle : particles) {
        result.remove(particle);
    }

    // 移除标点符号（保留连接词分隔符、和逗号，它们在 splitMultiKeywords 中处理）
    result.remove(QRegularExpression("[。！？；：“"
                                     "”‘’（）【】《》]"));

    // 移除多余空格
    result = result.simplified();

    return result;
}

QStringList KeywordExtractor::splitMultiKeywords(const QString &text)
{
    // 按中文连接词（和、与、以及、或、或者）和逗号（、，,）分割多关键词串
    static const QRegularExpression separator(
            "(?:以及|或者|和|与|或|[、,，])");

    QStringList result;
    const QStringList parts = text.split(separator, Qt::SkipEmptyParts);
    for (const QString &part : parts) {
        QString trimmed = part.trimmed();
        if (!trimmed.isEmpty()) {
            result.append(trimmed);
        }
    }

    // 如果分割后只有一个元素，说明没有连接词，直接返回原文本
    if (result.isEmpty()) {
        result.append(text.trimmed());
    }

    return result;
}
