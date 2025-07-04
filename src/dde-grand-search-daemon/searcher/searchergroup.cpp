// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchergroup.h"
#include "searchergroup_p.h"

#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

SearcherGroupPrivate::SearcherGroupPrivate(SearcherGroup *parent)
    : QObject(parent),
      q(parent)
{
}

void SearcherGroupPrivate::initBuiltin()
{
    Q_ASSERT(m_builtin.isEmpty());

    qCInfo(logDaemon) << "Initializing FileNameSearcher";
    auto fileSearcher = new FileNameSearcher(this);
    m_builtin << fileSearcher;

    qCInfo(logDaemon) << "Initializing DesktopAppSearcher";
    auto appSearcher = new DesktopAppSearcher(this);
    appSearcher->asyncInit();
    m_builtin << appSearcher;

    qCInfo(logDaemon) << "Initializing StaticTextEchoer";
    auto stWebSearcher = new StaticTextEchoer(this);
    m_builtin << stWebSearcher;

#ifdef ENABLE_SEMANTIC
    qInfo() << "create SemanticSearcher.";
    auto semanticSearcher = new SemanticSearcher(this);
    m_builtin << semanticSearcher;
#endif
}

bool SearcherGroupPrivate::addExtendSearcher(const SearchPluginInfo &pluginInfo)
{
    // 在插件模块需对以下信息做严格的校验，此处不再做有效性检查
    if (Q_UNLIKELY(pluginInfo.name.isEmpty() || pluginInfo.address.isEmpty()
                   || pluginInfo.service.isEmpty() || pluginInfo.interface.isEmpty()
                   || pluginInfo.ifsVersion.isEmpty())) {
        qCWarning(logDaemon) << "Invalid plugin info - Missing required fields";
        return false;
    }

    if (q->searcher(pluginInfo.name)) {
        qCWarning(logDaemon) << "Searcher already exists:" << pluginInfo.name;
        return false;
    }

    qCDebug(logDaemon) << "Creating ExtendSearcher:" << pluginInfo.name;
    ExtendSearcher *extend = new ExtendSearcher(pluginInfo.name, this);
    extend->setService(pluginInfo.service, pluginInfo.address,
                       pluginInfo.interface, pluginInfo.ifsVersion);

    // 自动启动的插件设置为可激活
    if (pluginInfo.mode == SearchPluginInfo::Auto) {
        qCDebug(logDaemon) << "Setting up auto-activation for plugin:" << pluginInfo.name;
        extend->setActivatable(ExtendSearcher::InnerActivation);

        // 激活操作,必须直连
        connect(extend, &ExtendSearcher::activateRequest, this, &SearcherGroupPrivate::onActivatePlugin, Qt::DirectConnection);
    } else if (pluginInfo.mode == SearchPluginInfo::Trigger) {   // dbus触发启动
        qCDebug(logDaemon) << "Setting up trigger activation for plugin:" << pluginInfo.name;
        extend->setActivatable(ExtendSearcher::Trigger);
    }

    m_extend << extend;
    return true;
}

void SearcherGroupPrivate::onActivatePlugin(const QString &name, bool &ret)
{
    Q_ASSERT(m_pluginManager);
    qCDebug(logDaemon) << "Activating plugin:" << name;
    ret = m_pluginManager->activatePlugin(name);
}

bool SearcherGroupPrivate::initPluinManager()
{
    Q_ASSERT(m_pluginManager == nullptr);

    qCDebug(logDaemon) << "Initializing plugin manager";
    m_pluginManager = new PluginManager(this);
    bool success = m_pluginManager->loadPlugin();
    if (!success) {
        qCCritical(logDaemon) << "Failed to initialize plugin manager";
    }
    return success;
}

void SearcherGroupPrivate::initExtendSearcher()
{
    Q_ASSERT(m_pluginManager);

    qCDebug(logDaemon) << "Initializing extended searchers";
    QList<SearchPluginInfo> plugins = m_pluginManager->plugins();
    for (const SearchPluginInfo &info : plugins) {
        if (!addExtendSearcher(info)) {
            qCWarning(logDaemon) << "Failed to create ExtendSearcher:" << info.name;
        }
    }
}

SearcherGroup::SearcherGroup(QObject *parent)
    : QObject(parent),
      d(new SearcherGroupPrivate(this))
{
    qCDebug(logDaemon) << "SearcherGroup constructor - Initializing searcher group";
}

bool SearcherGroup::init()
{
    qCInfo(logDaemon) << "SearcherGroup initialization started";

    // 初始化内置搜索项
    d->initBuiltin();

    // 初始化插件
    if (!d->initPluinManager()) {
        qCCritical(logDaemon) << "Failed to initialize plugin manager";
        return false;
    }

    // 创建扩展搜索项
    d->initExtendSearcher();

    // 启动高优先级插件
    d->m_pluginManager->autoActivate();

    qCInfo(logDaemon) << "SearcherGroup initialization completed successfully";
    return true;
}

QList<Searcher *> SearcherGroup::searchers() const
{
    return d->m_builtin + d->m_extend;
}

Searcher *SearcherGroup::searcher(const QString &name) const
{
    auto allSearcher = searchers();

    auto iter = std::find_if(allSearcher.begin(), allSearcher.end(), [name](const Searcher *s) {
        return s->name() == name;
    });

    if (iter != allSearcher.end()) {
        return *iter;
    } else {
        qCDebug(logDaemon) << "Searcher not found:" << name;
        return nullptr;
    }
}

void SearcherGroup::dormancy()
{
    qCDebug(logDaemon) << "Entering dormancy mode";

    if (d->m_pluginManager) {
        QList<SearchPluginInfo> plugins = d->m_pluginManager->plugins();
        for (const SearchPluginInfo &plugin : plugins) {
            // 停用低优先级的Auto类型插件
            if (plugin.mode == SearchPluginInfo::Auto
                && plugin.priority == SearchPluginInfo::Low) {
                qCDebug(logDaemon) << "Deactivating low priority plugin:" << plugin.name;
                d->m_pluginManager->inactivate(plugin.name);
            }
        }
    } else {
        qCWarning(logDaemon) << "Plugin manager not available for dormancy";
    }
}
