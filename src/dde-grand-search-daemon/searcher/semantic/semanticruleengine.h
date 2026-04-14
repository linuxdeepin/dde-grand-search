// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEMANTICRULEENGINE_H
#define SEMANTICRULEENGINE_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QString>
#include <QRegularExpression>
#include <QVariantMap>
#include <QJsonObject>

#include <memory>

class QFileSystemWatcher;
class RuleConfigLoader;

/**
 * @brief 单条正则规则
 */
struct RegexRule {
    QString id;                    // 规则ID（唯一标识）
    QRegularExpression pattern;    // 正则表达式对象
    QString description;           // 规则描述
    bool enabled;                  // 是否启用
    QVariantMap metadata;          // 扩展元数据（如优先级、标签等）
    int priority;                  // 优先级（数值越大优先级越高）

    RegexRule() : enabled(true), priority(0) {}
};

/**
 * @brief 规则组（按功能分组）
 */
struct RegexRuleGroup {
    QString name;                  // 组名（如 "time", "filetype", "action"）
    QString version;               // 版本号
    QList<RegexRule> rules;        // 规则列表
};

/**
 * @brief 正则表达式规则管理器
 *
 * 支持从JSON配置文件加载规则、热更新、规则匹配等功能
 */
class SemanticRuleEngine : public QObject {
    Q_OBJECT

public:
    explicit SemanticRuleEngine(QObject* parent = nullptr);
    ~SemanticRuleEngine() override;

    /**
     * @brief 从多个文件加载规则（合并到同一引擎）
     *
     * 使用 RuleConfigLoader 按优先级解析每个规则文件路径，
     * 然后依次加载并合并所有规则组。
     * @param filePaths 规则文件路径列表
     * @return 是否至少加载了一个规则组
     */
    bool loadFromMultipleFiles(const QStringList &filePaths);

    /**
     * @brief 使用 RuleConfigLoader 自动加载所有规则
     *
     * 自动处理本地配置 > qrc 内置配置的优先级。
     * @param loader 配置加载器
     * @return 是否加载成功
     */
    bool loadWithConfigLoader(RuleConfigLoader *loader);

    /**
     * @brief 规则匹配
     * @param group 规则组名
     * @param input 输入文本
     * @param matchedRuleId 匹配到的规则ID（输出参数）
     * @param matchResult 匹配结果（输出参数）
     * @return 是否匹配成功
     */
    bool match(const QString& group, const QString& input,
               QString* matchedRuleId = nullptr,
               QRegularExpressionMatch* matchResult = nullptr);

    /**
     * @brief 匹配所有符合条件的规则
     * @param group 规则组名
     * @param input 输入文本
     * @return 匹配到的规则列表
     */
    QList<RegexRule> matchAll(const QString& group, const QString& input);

    /**
     * @brief 获取指定ID的规则
     * @param id 规则ID
     * @return 规则对象
     */
    RegexRule getRule(const QString& id) const;

    /**
     * @brief 获取指定组的所有规则
     * @param group 组名
     * @return 规则列表
     */
    QList<RegexRule> getGroupRules(const QString& group) const;

    /**
     * @brief 获取所有规则组
     * @return 规则组列表
     */
    QList<RegexRuleGroup> getAllGroups() const;

    /**
     * @brief 启用/禁用热更新
     * @param enabled 是否启用
     */
    void enableHotReload(bool enabled);

    /**
     * @brief 检查热更新是否启用
     * @return 是否启用
     */
    bool isHotReloadEnabled() const { return m_hotReloadEnabled; }

    /**
     * @brief 获取规则版本
     * @return 版本号
     */
    QString getVersion() const { return m_version; }

signals:
    /**
     * @brief 规则重载完成信号
     */
    void rulesReloaded();

    /**
     * @brief 规则加载错误信号
     * @param error 错误信息
     */
    void ruleLoadError(const QString& error);

private:
    QMap<QString, RegexRuleGroup> m_groups;                  // 规则组映射
    QMap<QString, RegexRule> m_allRules;                     // 所有规则映射（按ID索引）
    std::unique_ptr<QFileSystemWatcher> m_watcher;           // 文件系统监控器
    QString m_version;                                       // 配置版本
    bool m_hotReloadEnabled;                                 // 是否启用热更新
    RuleConfigLoader *m_configLoader = nullptr;              // 配置加载器（外部持有生命周期）

    /**
     * @brief 设置文件监控（监控本地配置目录）
     */
    void setupWatcher();

    /**
     * @brief 文件变更时重载规则
     */
    void reloadOnChange();

    /**
     * @brief 校验规则是否有效
     * @param rule 规则对象
     * @return 是否有效
     */
    bool validateRule(const RegexRule& rule);

    /**
     * @brief 解析单个规则组
     * @param groupObj JSON对象
     * @return 规则组对象
     */
    RegexRuleGroup parseGroup(const QJsonObject& groupObj);

    /**
     * @brief 解析单个规则
     * @param ruleObj JSON对象
     * @return 规则对象
     */
    RegexRule parseRule(const QJsonObject& ruleObj);

    /**
     * @brief 按优先级排序规则
     * @param group 规则组名
     */
    void sortRulesByPriority(const QString& group);
};

#endif // SEMANTICRULEENGINE_H
