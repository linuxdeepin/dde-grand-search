// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logrulesmanager.h"

#include <QLoggingCategory>
#include <DConfig>

Q_LOGGING_CATEGORY(logRulesManager, "org.deepin.dde.grandsearch.rules")

using namespace GrandSearch;
DCORE_USE_NAMESPACE

namespace {
const char *kConfigAppId = "org.deepin.dde.grandsearch";
const char *kConfigName = "org.deepin.dde.grandsearch";
const char *kLogRulesKey = "log_rules";
}

LogRulesManager::LogRulesManager(QObject *parent)
    : QObject(parent)
{
}

LogRulesManager::~LogRulesManager()
{
    if (m_config) {
        m_config->deleteLater();
        m_config = nullptr;
    }
}

LogRulesManager *LogRulesManager::instance()
{
    static LogRulesManager ins;
    return &ins;
}

void LogRulesManager::applyLogRules()
{
    // QT_LOGGING_RULES 环境变量优先级高于 QLoggingCategory::setFilterRules，
    // 必须先清除，否则 DConfig 下发的规则不会生效。
    // 注意：调用点需保证在此之前没有任何日志打印。
    qunsetenv("QT_LOGGING_RULES");

    if (m_config)
        return;

    m_config = DConfig::create(kConfigAppId, kConfigName);
    if (!m_config) {
        qCWarning(logRulesManager) << "Create dconfig failed:" << kConfigAppId << kConfigName;
        return;
    }

    const QString rules = m_config->value(kLogRulesKey).toString();
    setRules(rules);
    qCDebug(logRulesManager) << "Log rules inited:" << rules;

    // 监听配置变更，运行中动态调整日志级别
    QObject::connect(m_config, &DConfig::valueChanged, this, [this](const QString &key) {
        if (key != kLogRulesKey)
            return;
        const QString newRules = m_config->value(kLogRulesKey).toString();
        appendRules(newRules);
        qCInfo(logRulesManager) << "Log rules changed:" << newRules;
    });
}

void LogRulesManager::setRules(const QString &rules)
{
    // Qt 的 QLoggingCategory::setFilterRules 按「行」分割规则，
    // 而 DConfig/QT_LOGGING_RULES 约定用“;”分隔，这里统一换成换行后再应用
    QString tmpRules = rules;
    tmpRules.replace(";", "\n");
    m_rules = tmpRules;
    QLoggingCategory::setFilterRules(m_rules);
}

void LogRulesManager::appendRules(const QString &rules)
{
    // 与进程原有规则串联：新规则在后，后匹配者生效；去重避免规则无限膨胀
    QString tmpRules = rules;
    tmpRules.replace(";", "\n");
    auto tmplist = tmpRules.split('\n');
    for (int i = 0; i < tmplist.count(); i++) {
        if (tmplist.at(i).isEmpty()) {
            tmplist.removeAt(i);
            i--;
            continue;
        }
        if (m_rules.contains(tmplist.at(i))) {
            tmplist.removeAt(i);
            i--;
        }
    }
    if (tmplist.isEmpty())
        return;
    m_rules = m_rules.isEmpty() ? tmplist.join("\n") : m_rules + "\n" + tmplist.join("\n");
    QLoggingCategory::setFilterRules(m_rules);
}
