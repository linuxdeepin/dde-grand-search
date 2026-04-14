// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RULECONFIGLOADER_H
#define RULECONFIGLOADER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonObject>

/**
 * @brief 规则配置加载器
 *
 * 负责管理规则文件的加载优先级和本地配置初始化。
 * 加载优先级：本地配置 > qrc 内置配置
 * 本地路径：~/.config/deepin/dde-grand-search-daemon/semantic/
 */
class RuleConfigLoader : public QObject {
    Q_OBJECT

public:
    explicit RuleConfigLoader(QObject *parent = nullptr);
    ~RuleConfigLoader() override;

    /**
     * @brief 规则文件类型枚举
     */
    enum RuleType {
        Noise,      ///< 基础动作规则（noise 组）
        Keyword,    ///< 关键词规则（keyword 组）
        Time,       ///< 时间规则（time 组）
        FileType    ///< 文件类型规则（filetype 组）
    };

    /**
     * @brief 获取指定类型规则文件的有效路径
     *
     * 按优先级查找：本地配置 > qrc 内置配置
     * @param type 规则类型
     * @return 有效的文件路径（本地或 qrc 路径）
     */
    QString resolveRulePath(RuleType type) const;

    /**
     * @brief 获取所有规则文件的有效路径列表
     * @return 路径列表（按 noise, keyword, time, filetype 顺序）
     */
    QStringList resolveAllRulePaths() const;

    /**
     * @brief 确保本地配置目录存在，不存在时从 qrc 初始化
     *
     * 将 qrc 内置规则导出到本地配置目录。
     * 如果本地已存在则跳过（不覆盖用户修改）。
     * @return 是否初始化成功
     */
    bool ensureLocalConfig();

    /**
     * @brief 将指定类型的 qrc 规则导出到本地
     * @param type 规则类型
     * @param overwrite 是否覆盖已有文件
     * @return 是否导出成功
     */
    bool exportToLocal(RuleType type, bool overwrite = false);

    /**
     * @brief 将所有 qrc 规则导出到本地
     * @param overwrite 是否覆盖已有文件
     * @return 是否全部导出成功
     */
    bool exportAllToLocal(bool overwrite = false);

    /**
     * @brief 验证本地规则文件是否有效
     * @param filePath 文件路径
     * @return 是否有效（JSON 格式正确且包含必要字段）
     */
    bool validateRuleFile(const QString &filePath) const;

    /**
     * @brief 获取本地配置目录路径
     * @return 本地配置目录路径
     */
    QString localConfigDir() const;

    /**
     * @brief 获取 qrc 内置规则目录前缀
     * @return qrc 路径前缀
     */
    QString qrcConfigPrefix() const;

    /**
     * @brief 获取规则类型对应的文件名
     * @param type 规则类型
     * @return 文件名（如 "noise_rules.json"）
     */
    static QString ruleFileName(RuleType type);

    /**
     * @brief 获取所有规则类型
     * @return 规则类型列表
     */
    static QList<RuleType> allRuleTypes();

signals:
    /**
     * @brief 本地配置初始化完成信号
     * @param success 是否成功
     */
    void localConfigInitialized(bool success);

    /**
     * @brief 规则文件验证失败信号
     * @param filePath 文件路径
     * @param error 错误信息
     */
    void ruleFileInvalid(const QString &filePath, const QString &error);

private:
    /**
     * @brief 获取指定类型的 qrc 路径
     */
    QString qrcRulePath(RuleType type) const;

    /**
     * @brief 获取指定类型的本地路径
     */
    QString localRulePath(RuleType type) const;

    /**
     * @brief 确保本地配置目录存在
     * @return 是否成功创建或已存在
     */
    bool ensureLocalDir() const;

    QString m_localConfigDir;   ///< 本地配置目录缓存
    QString m_qrcPrefix;        ///< qrc 路径前缀缓存
};

#endif // RULECONFIGLOADER_H
