// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginmanager.h"
#include "pluginmanager_p.h"
#include "convertors/dataconvertor.h"

#include <QStandardPaths>
#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

PluginManagerPrivate::PluginManagerPrivate(PluginManager *parent)
    : QObject(parent),
      q(parent)
{
}

bool PluginManagerPrivate::readConf()
{
    if (!m_loader) {
        m_loader = new PluginLoader(this);

        // 默认路径
#ifdef QT_DEBUG
        char path[PATH_MAX] = { 0 };
        const char *defaultPath = realpath("./", path);
#else
        auto defaultPath = PLUGIN_SEARCHER_DIR;
#endif
        static_assert(std::is_same<decltype(defaultPath), const char *>::value, "PLUGIN_SEARCHER_DIR is not a string.");

        qCDebug(logDaemon) << "Setting plugin search path:" << defaultPath;
        m_loader->setPluginPath({ QString(defaultPath) });
    }

    return m_loader->load();
}

void PluginManagerPrivate::prepareProcess()
{
    if (!m_process)
        m_process = new PluginProcess(this);

    Q_ASSERT(m_loader);

    // 清除所有进程
    m_process->clear();

    QList<SearchPluginInfo> plugins = m_loader->plugins();
    qCDebug(logDaemon) << "Found" << plugins.size() << "plugins to process";

    for (const SearchPluginInfo &plugin : plugins) {
        // Auto类型的插件需进行进程管理
        if (plugin.mode == SearchPluginInfo::Auto) {
            qCDebug(logDaemon) << "Creating process for plugin:" << plugin.name
                               << "Priority:" << plugin.priority
                               << "Source:" << plugin.from;

            if (m_process->addProgram(plugin.name, plugin.exec)) {

                // 高，中优先级添加守护
                if (plugin.priority <= SearchPluginInfo::Middle)
                    m_process->setWatched(plugin.name, true);
            } else {
                qCWarning(logDaemon) << "Failed to create process for plugin:" << plugin.name
                                     << "Executable:" << plugin.exec
                                     << "Source:" << plugin.from;
            }
        }
    }
}

PluginManager::PluginManager(QObject *parent)
    : QObject(parent),
      d(new PluginManagerPrivate(this))
{
    qCDebug(logDaemon) << "PluginManager constructor - Created plugin manager";
}

bool PluginManager::loadPlugin()
{
    qCDebug(logDaemon) << "Starting plugin loading process";

    // 初始化数据协议
    DataConvertor::instance()->initConvetor();

    // 读取插件目录下的conf
    bool ret = d->readConf();

    // 加入进程管理
    d->prepareProcess();

    qCDebug(logDaemon) << "Plugin loading process completed - Success:" << ret;
    return ret;
}

QList<SearchPluginInfo> PluginManager::plugins() const
{
    if (d->m_loader)
        return d->m_loader->plugins();

    return {};
}

void PluginManager::autoActivate()
{
    Q_ASSERT(d->m_loader);
    Q_ASSERT(d->m_process);

    qCDebug(logDaemon) << "Starting auto-activation of high priority plugins";
    QList<SearchPluginInfo> plugins = d->m_loader->plugins();
    for (const SearchPluginInfo &plugin : plugins) {
        // 启动高优先级的Auto类型插件
        if (plugin.mode == SearchPluginInfo::Auto
            && plugin.priority == SearchPluginInfo::High) {
            qCDebug(logDaemon) << "Auto-activating high priority plugin:" << plugin.name;
            d->m_process->startProgram(plugin.name);
        }
    }
}

bool PluginManager::activatePlugin(const QString &name)
{
    Q_ASSERT(d->m_process);
    return d->m_process->startProgram(name);
}

void PluginManager::inactivate(const QString &name)
{
    Q_ASSERT(d->m_process);
    qCDebug(logDaemon) << "Inactivating plugin:" << name;
    d->m_process->terminate(name);
    return;
}
