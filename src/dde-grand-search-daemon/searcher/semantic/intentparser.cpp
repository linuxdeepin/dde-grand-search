// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "intentparser.h"
#include "semanticruleengine.h"
#include "extractors/dimensionextractor.h"
#include "extractors/timeextractor.h"
#include "extractors/filetypeextractor.h"
#include "extractors/keywordextractor.h"
#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

IntentParser::IntentParser(SemanticRuleEngine *ruleManager, QObject *parent)
    : QObject(parent), m_ruleManager(ruleManager)
{
    initDefaultExtractors();
}

IntentParser::~IntentParser()
{
}

void IntentParser::initDefaultExtractors()
{
    // 注意顺序：关键词提取器必须最后，因为它依赖前面提取器记录的已消费区间
    addExtractor(std::make_unique<TimeExtractor>(m_ruleManager));
    addExtractor(std::make_unique<FileTypeExtractor>(m_ruleManager));
    addExtractor(std::make_unique<KeywordExtractor>(m_ruleManager));
}

void IntentParser::addExtractor(std::unique_ptr<DimensionExtractor> extractor)
{
    m_extractors.append(std::shared_ptr<DimensionExtractor>(extractor.release()));
}

ParsedIntent IntentParser::parse(const QString &userInput)
{
    ParsedIntent intent;
    intent.rawInput = userInput;

    qCDebug(logDaemon) << "Starting parsing:" << userInput;

    // 依次调用各维度提取器
    for (auto &extractor : m_extractors) {
        extractor->extract(userInput, intent);
    }

    qCDebug(logDaemon) << "Parsing result:" << intent.toString();

    return intent;
}

QString ParsedIntent::toString() const
{
    QStringList parts;

    // 时间约束
    if (hasTimeConstraint()) {
        switch (timeConstraint.kind) {
        case TimeConstraint::Kind::Preset: {
            static const QHash<TimeConstraint::Preset, QString> presetNames = {
                { TimeConstraint::Preset::Today, "今天" },
                { TimeConstraint::Preset::Yesterday, "昨天" },
                { TimeConstraint::Preset::ThisWeek, "本周" },
                { TimeConstraint::Preset::LastWeek, "上周" },
                { TimeConstraint::Preset::ThisMonth, "本月" },
                { TimeConstraint::Preset::LastMonth, "上月" },
                { TimeConstraint::Preset::ThisYear, "今年" },
                { TimeConstraint::Preset::LastYear, "去年" },
            };
            parts << QString("时间: %1").arg(presetNames.value(timeConstraint.preset, "未知"));
            break;
        }
        case TimeConstraint::Kind::Relative: {
            static const QHash<DFMSEARCH::TimeUnit, QString> unitNames = {
                { DFMSEARCH::TimeUnit::Minutes, "分钟" },
                { DFMSEARCH::TimeUnit::Hours, "小时" },
                { DFMSEARCH::TimeUnit::Days, "天" },
                { DFMSEARCH::TimeUnit::Weeks, "周" },
                { DFMSEARCH::TimeUnit::Months, "个月" },
                { DFMSEARCH::TimeUnit::Years, "年" },
            };
            parts << QString("时间: 最近%1%2")
                             .arg(timeConstraint.relativeValue)
                             .arg(unitNames.value(timeConstraint.relativeUnit, "天"));
            break;
        }
        case TimeConstraint::Kind::Custom:
            parts << QString("时间: %1 至 %2")
                             .arg(timeConstraint.customStart.toString("yyyy-MM-dd"))
                             .arg(timeConstraint.customEnd.toString("yyyy-MM-dd"));
            break;
        default:
            break;
        }
    }

    // 文件类型
    if (hasFileTypes()) {
        parts << QString("类型: %1").arg(fileTypes.join(", "));
    }

    // 关键词
    if (hasKeywords()) {
        parts << QString("关键词: %1").arg(keywords.join(", "));
    }

    return parts.join(" | ");
}
