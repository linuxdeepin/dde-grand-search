// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DGS_LOGRULESMANAGER_H
#define DGS_LOGRULESMANAGER_H

#include <QObject>

namespace Dtk {
namespace Core {
class DConfig;
}
}

namespace GrandSearch {

/**
 * @brief 动态日志规则管理器（调试模式适配）
 *
 * 从 DConfig(org.deepin.dde.grandsearch) 读取 log_rules 键值并应用为 Qt 日志
 * 过滤规则，同时监听配置变更，实现应用运行期间日志级别的动态调整。
 * 配合 deepin-debug-config 调试模式使用：调试模式开启后通过
 * `dde-dconfig --set -a org.deepin.dde.grandsearch -r org.deepin.dde.grandsearch
 * -k log_rules -v <rules>` 下发规则，本类即时应用，无需重启应用。
 *
 * 使用约束：applyLogRules() 必须在任何日志打印之前调用（如 main() 中
 * Application 构造之前、插件构造函数最开始处），因为其内部会执行
 * qunsetenv("QT_LOGGING_RULES")，环境变量须先于 QApplication 内部的日志输出清除。
 */
class LogRulesManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(LogRulesManager)

public:
    static LogRulesManager *instance();
    // 在任何日志打印之前调用；读取 DConfig 并注册变更监听
    void applyLogRules();

private:
    explicit LogRulesManager(QObject *parent = nullptr);
    ~LogRulesManager() override;
    // 设置规则（覆盖已有，用于初始化）
    void setRules(const QString &rules);
    // 追加规则（用于配置变更，保证不会清掉进程原有规则）
    void appendRules(const QString &rules);

private:
    QString m_rules;
    Dtk::Core::DConfig *m_config = nullptr;
};

}   // namespace GrandSearch

#define dgsLogRulesManager ::GrandSearch::LogRulesManager::instance()

#endif   // DGS_LOGRULESMANAGER_H
