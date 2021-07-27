/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "searchergroup.h"
#include "searchergroupprivate.h"

#include <QDebug>

SearcherGroupPrivate::SearcherGroupPrivate(SearcherGroup *parent)
    : QObject(parent)
    , q(parent)
{
}

void SearcherGroupPrivate::initBuiltin()
{
    Q_ASSERT(m_builtin.isEmpty());

#ifdef ENABLE_DEEPINANYTHING
    qInfo() << "create FileNameSearcher";
    auto fileSearcher = new FileNameSearcher(this);
    m_builtin << fileSearcher;
#endif

#ifdef ENABLE_FSEARCH
    qInfo() << "create FsSearcher.";
    auto fSearcher = new FsSearcher(this);
    fSearcher->asyncInitDataBase();
    m_builtin << fSearcher;
#endif

    qInfo() << "create DesktopAppSearcher.";
    auto appSearcher = new DesktopAppSearcher(this);
    appSearcher->asyncInit();
    m_builtin << appSearcher;

    //todo 设置搜索，全文搜索
}

bool SearcherGroupPrivate::addExtendSearcher(const GrandSearch::SearchPluginInfo &pluginInfo)
{
    //在插件模块需对以下信息做严格的校验，此处不再做有效性检查
    if (Q_UNLIKELY(pluginInfo.name.isEmpty() || pluginInfo.address.isEmpty()
            || pluginInfo.service.isEmpty() || pluginInfo.interface.isEmpty()
            || pluginInfo.ifsVersion.isEmpty()))
        return false;

    if (q->searcher(pluginInfo.name)) {
        qWarning() << "searcher has existed." << pluginInfo.name;
        return false;
    }

    qDebug() << "cretate ExtendSearcher" << pluginInfo.name;
    ExtendSearcher *extend = new ExtendSearcher(pluginInfo.name, this);
    extend->setService(pluginInfo.service, pluginInfo.address,
                         pluginInfo.interface, pluginInfo.ifsVersion);

    //自动启动的插件设置为可激活
    if (pluginInfo.mode == GrandSearch::SearchPluginInfo::Auto) {
        extend->setActivatable(true);

        //激活操作,必须直连
        connect(extend, &ExtendSearcher::activateRequest, this, &SearcherGroupPrivate::onActivatePlugin, Qt::DirectConnection);
    }

    m_extend << extend;
    return true;
}

void SearcherGroupPrivate::onActivatePlugin(const QString &name, bool &ret)
{
    Q_ASSERT(m_pluginManager);
    ret = m_pluginManager->activatePlugin(name);
}

bool SearcherGroupPrivate::initPluinManager()
{
    Q_ASSERT(m_pluginManager == nullptr);

    m_pluginManager = new PluginManager(this);
    return m_pluginManager->loadPlugin();
}

void SearcherGroupPrivate::initExtendSearcher()
{
    Q_ASSERT(m_pluginManager);

    QList<GrandSearch::SearchPluginInfo> plugins = m_pluginManager->plugins();
    for (const GrandSearch::SearchPluginInfo &info : plugins)
        if (!addExtendSearcher(info))
            qWarning() << "create ExtendSearcher error:" << info.name;
}

SearcherGroup::SearcherGroup(QObject *parent)
    : QObject(parent)
    , d(new SearcherGroupPrivate(this))
{
}

bool SearcherGroup::init()
{
    //初始化内置搜索项
    d->initBuiltin();

    //初始化插件
    if (!d->initPluinManager()) {
        qCritical() << "error: fail to init PluinManager.";
        return false;
    }

    //创建扩展搜索项
    d->initExtendSearcher();

    //启动高优先级插件
    d->m_pluginManager->autoActivate();

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

    if (iter != allSearcher.end())
        return *iter;
    else
        return nullptr;
}

