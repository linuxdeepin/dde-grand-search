// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semanticruleengine.h"
#include "config/ruleconfigloader.h"
#include <QFile>
#include <QDir>
#include <QFileSystemWatcher>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QTimer>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

SemanticRuleEngine::SemanticRuleEngine(QObject *parent)
    : QObject(parent),
      m_hotReloadEnabled(false)
{
}

SemanticRuleEngine::~SemanticRuleEngine() = default;

bool SemanticRuleEngine::loadFromMultipleFiles(const QStringList &filePaths)
{
    if (filePaths.isEmpty()) {
        qCWarning(logDaemon) << "No rule files to load";
        return false;
    }

    // 备份当前规则（用于回滚）
    auto oldGroups = m_groups;
    auto oldAllRules = m_allRules;
    auto oldVersion = m_version;

    // 清空旧规则
    m_groups.clear();
    m_allRules.clear();

    bool anyLoaded = false;
    for (const QString &filePath : filePaths) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qCWarning(logDaemon) << "Cannot open rule file:" << filePath;
            continue;
        }

        QByteArray data = file.readAll();
        file.close();

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qCWarning(logDaemon) << "JSON parse error in" << filePath
                                 << ":" << parseError.errorString();
            continue;
        }

        if (!doc.isObject()) {
            qCWarning(logDaemon) << "Invalid format in" << filePath;
            continue;
        }

        QJsonObject root = doc.object();

        // 合并版本号（取最新的）
        QString fileVersion = root["version"].toString("1.0.0");
        if (m_version.isEmpty() || fileVersion > m_version) {
            m_version = fileVersion;
        }

        // 加载并合并规则组
        QJsonArray groups = root["groups"].toArray();
        for (const QJsonValue &groupVal : groups) {
            if (!groupVal.isObject()) {
                continue;
            }

            QJsonObject groupObj = groupVal.toObject();
            RegexRuleGroup group = parseGroup(groupObj);

            if (group.rules.isEmpty()) {
                qCWarning(logDaemon) << "Empty rule group in" << filePath << ":" << group.name;
                continue;
            }

            // 如果同名组已存在，合并规则（后加载的覆盖同 ID 规则）
            if (m_groups.contains(group.name)) {
                RegexRuleGroup &existing = m_groups[group.name];

                // 用新版本号替换旧版本号
                if (group.version > existing.version) {
                    existing.version = group.version;
                }

                // 合并规则：按 ID 去重，后加载的覆盖先加载的
                QMap<QString, int> existingIdIndex;
                for (int i = 0; i < existing.rules.size(); ++i) {
                    existingIdIndex[existing.rules[i].id] = i;
                }

                for (const RegexRule &rule : group.rules) {
                    if (existingIdIndex.contains(rule.id)) {
                        existing.rules[existingIdIndex[rule.id]] = rule;
                    } else {
                        existing.rules.append(rule);
                    }
                    m_allRules[rule.id] = rule;
                }

                // 重新排序
                std::sort(existing.rules.begin(), existing.rules.end(),
                          [](const RegexRule &a, const RegexRule &b) {
                              return a.priority > b.priority;
                          });
            } else {
                m_groups[group.name] = group;
                for (const RegexRule &rule : group.rules) {
                    m_allRules[rule.id] = rule;
                }
            }

            anyLoaded = true;
        }

        qCDebug(logDaemon) << "Loaded rule file:" << filePath;
    }

    // 如果没有加载到任何规则组，回滚
    if (!anyLoaded || m_groups.isEmpty()) {
        qCWarning(logDaemon) << "No valid rules loaded from multiple files, rolling back";
        m_groups = std::move(oldGroups);
        m_allRules = std::move(oldAllRules);
        m_version = std::move(oldVersion);
        emit ruleLoadError(tr("No valid rules loaded from multiple files"));
        return false;
    }

    qCDebug(logDaemon) << "Successfully loaded" << m_groups.size()
                       << "rule groups from" << filePaths.size()
                       << "files, total" << m_allRules.size() << "rules";
    emit rulesReloaded();
    return true;
}

bool SemanticRuleEngine::loadWithConfigLoader(RuleConfigLoader *loader)
{
    if (!loader) {
        qCWarning(logDaemon) << "RuleConfigLoader is null";
        return false;
    }

    // 保存 loader 引用，用于热重载
    m_configLoader = loader;

    // 确保本地配置已初始化
    loader->ensureLocalConfig();

    // 获取所有规则文件的有效路径（按优先级解析）
    QStringList rulePaths = loader->resolveAllRulePaths();

    qCDebug(logDaemon) << "Loading rules with config loader, paths:" << rulePaths;

    return loadFromMultipleFiles(rulePaths);
}

bool SemanticRuleEngine::match(const QString &group, const QString &input,
                               QString *matchedRuleId,
                               QRegularExpressionMatch *matchResult)
{
    if (!m_groups.contains(group)) {
        qCWarning(logDaemon) << "Rule group does not exist:" << group;
        return false;
    }

    const RegexRuleGroup &ruleGroup = m_groups[group];

    for (const RegexRule &rule : ruleGroup.rules) {
        if (!rule.enabled) {
            continue;
        }

        QRegularExpressionMatch match = rule.pattern.match(input);
        if (match.hasMatch()) {
            if (matchedRuleId) {
                *matchedRuleId = rule.id;
            }
            if (matchResult) {
                *matchResult = match;
            }
            return true;
        }
    }

    return false;
}

QList<RegexRule> SemanticRuleEngine::matchAll(const QString &group, const QString &input)
{
    QList<RegexRule> matchedRules;

    if (!m_groups.contains(group)) {
        qCWarning(logDaemon) << "Rule group does not exist:" << group;
        return matchedRules;
    }

    const RegexRuleGroup &ruleGroup = m_groups[group];

    for (const RegexRule &rule : ruleGroup.rules) {
        if (!rule.enabled) {
            continue;
        }

        QRegularExpressionMatch match = rule.pattern.match(input);
        if (match.hasMatch()) {
            matchedRules.append(rule);
        }
    }

    return matchedRules;
}

RegexRule SemanticRuleEngine::getRule(const QString &id) const
{
    if (m_allRules.contains(id)) {
        return m_allRules[id];
    }

    qCWarning(logDaemon) << "Rule does not exist:" << id;
    return RegexRule();
}

QList<RegexRule> SemanticRuleEngine::getGroupRules(const QString &group) const
{
    if (m_groups.contains(group)) {
        return m_groups[group].rules;
    }

    qCWarning(logDaemon) << "Rule group does not exist:" << group;
    return QList<RegexRule>();
}

QList<RegexRuleGroup> SemanticRuleEngine::getAllGroups() const
{
    return m_groups.values();
}

void SemanticRuleEngine::enableHotReload(bool enabled)
{
    if (m_hotReloadEnabled == enabled) {
        return;
    }

    m_hotReloadEnabled = enabled;

    if (enabled) {
        setupWatcher();
        qCDebug(logDaemon) << "Hot reload enabled";
    } else {
        m_watcher.reset();
        qCDebug(logDaemon) << "Hot reload disabled";
    }
}

void SemanticRuleEngine::setupWatcher()
{
    if (!m_configLoader) {
        qCWarning(logDaemon) << "Cannot setup watcher: no config loader";
        return;
    }

    m_watcher = std::make_unique<QFileSystemWatcher>();

    // 监控本地配置目录
    QString localDir = m_configLoader->localConfigDir();
    if (!localDir.isEmpty() && QDir(localDir).exists()) {
        m_watcher->addPath(localDir);
        qCDebug(logDaemon) << "Watching local config directory:" << localDir;
    }

    // 监控本地配置目录下的各个规则文件
    QStringList rulePaths = m_configLoader->resolveAllRulePaths();
    for (const QString &path : rulePaths) {
        if (!path.startsWith(":/")) {   // 只监控本地文件，不监控 qrc
            m_watcher->addPath(path);
        }
    }

    connect(m_watcher.get(), &QFileSystemWatcher::fileChanged,
            this, [this](const QString &path) {
                Q_UNUSED(path);
                // 延迟重载，防止频繁IO
                QTimer::singleShot(100, this, &SemanticRuleEngine::reloadOnChange);
            });

    connect(m_watcher.get(), &QFileSystemWatcher::directoryChanged,
            this, [this](const QString &path) {
                Q_UNUSED(path);
                QTimer::singleShot(100, this, &SemanticRuleEngine::reloadOnChange);
            });

    qCDebug(logDaemon) << "File watcher set up for" << m_watcher->files().size()
                       << "files and" << m_watcher->directories().size() << "directories";
}

void SemanticRuleEngine::reloadOnChange()
{
    qCDebug(logDaemon) << "Config file changed detected, reloading rules...";

    if (!m_configLoader) {
        qCWarning(logDaemon) << "Cannot reload: no config loader";
        return;
    }

    QStringList rulePaths = m_configLoader->resolveAllRulePaths();
    if (loadFromMultipleFiles(rulePaths)) {
        qCDebug(logDaemon) << "Rules reloaded successfully, version:" << m_version;
    } else {
        // loadFromMultipleFiles 内部已处理规则回滚
        qCWarning(logDaemon) << "Rules reload failed, rolled back to previous version";
    }
}

bool SemanticRuleEngine::validateRule(const RegexRule &rule)
{
    // 检查ID是否为空
    if (rule.id.isEmpty()) {
        qCWarning(logDaemon) << "Rule ID cannot be empty";
        return false;
    }

    // 检查正则表达式是否有效
    if (!rule.pattern.isValid()) {
        qCWarning(logDaemon) << "Invalid regex pattern:" << rule.id
                             << "-" << rule.pattern.errorString();
        return false;
    }

    // 检查正则表达式模式是否为空
    if (rule.pattern.pattern().isEmpty()) {
        qCWarning(logDaemon) << "Regex pattern cannot be empty:" << rule.id;
        return false;
    }

    return true;
}

RegexRuleGroup SemanticRuleEngine::parseGroup(const QJsonObject &groupObj)
{
    RegexRuleGroup group;
    group.name = groupObj["name"].toString();
    group.version = groupObj["version"].toString("1.0.0");

    if (group.name.isEmpty()) {
        qCWarning(logDaemon) << "Rule group name cannot be empty";
        return group;
    }

    QJsonArray rules = groupObj["rules"].toArray();
    for (const QJsonValue &ruleVal : rules) {
        if (!ruleVal.isObject()) {
            qCWarning(logDaemon) << "Rule format error, skipping";
            continue;
        }

        QJsonObject ruleObj = ruleVal.toObject();
        RegexRule rule = parseRule(ruleObj);

        if (validateRule(rule)) {
            group.rules.append(rule);
        }
    }

    // 按优先级排序（在本地group对象上排序）
    std::sort(group.rules.begin(), group.rules.end(),
              [](const RegexRule &a, const RegexRule &b) {
                  return a.priority > b.priority;
              });

    return group;
}

RegexRule SemanticRuleEngine::parseRule(const QJsonObject &ruleObj)
{
    RegexRule rule;
    rule.id = ruleObj["id"].toString();
    // 使用 CaseInsensitiveOption 使匹配不区分大小写
    rule.pattern = QRegularExpression(ruleObj["pattern"].toString(),
                                       QRegularExpression::CaseInsensitiveOption);
    rule.description = ruleObj["description"].toString();
    rule.enabled = ruleObj["enabled"].toBool(true);
    rule.priority = ruleObj["priority"].toInt(0);
    rule.metadata = ruleObj["metadata"].toObject().toVariantMap();

    return rule;
}

void SemanticRuleEngine::sortRulesByPriority(const QString &group)
{
    if (!m_groups.contains(group)) {
        return;
    }

    // 按优先级降序排序（优先级高的在前）
    std::sort(m_groups[group].rules.begin(), m_groups[group].rules.end(),
              [](const RegexRule &a, const RegexRule &b) {
                  return a.priority > b.priority;
              });
}
