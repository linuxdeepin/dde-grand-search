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
#include "pluginmanager.h"
#include "pluginmanagerprivate.h"

#include <QStandardPaths>
#include <QDebug>

PluginManagerPrivate::PluginManagerPrivate(PluginManager *parent)
    : QObject(parent)
    , q(parent)
{

}

bool PluginManagerPrivate::readConf()
{
    if (!m_loader) {
        m_loader = new PluginLoader(this);

        //默认路径
#ifdef QT_DEBUG
        const char *defaultPath = realpath("./", nullptr);
#else
        auto defaultPath = PLUGIN_SEARCHER_DIR;
#endif
        static_assert(std::is_same<decltype(defaultPath), const char *>::value, "PLUGIN_SEARCHER_DIR is not a string.");

        m_loader->setPluginPath({QString(defaultPath)});
    }

    return m_loader->load();
}

void PluginManagerPrivate::prepareProcess()
{
    if (!m_process)
        m_process = new PluginProcess(this);

    Q_ASSERT(m_loader);

    //清除所有进程
    m_process->clear();

    QList<GrandSearch::SearchPluginInfo> plugins = m_loader->plugins();
    for (const GrandSearch::SearchPluginInfo &plugin : plugins) {
        //Auto类型的插件需进行进程管理
        if (plugin.mode == GrandSearch::SearchPluginInfo::Auto) {
            qDebug() << "create process" << plugin.name;

            if (m_process->addProgram(plugin.name, plugin.exec)) {

                //高，中优先级添加守护
                if (plugin.priority  <= GrandSearch::SearchPluginInfo::Middle)
                    m_process->setWatched(plugin.name, true);
            } else {
                qWarning() << "program error: " <<  plugin.name << plugin.exec << plugin.from;
            }
        }
    }
}

PluginManager::PluginManager(QObject *parent)
    : QObject(parent)
    , d(new PluginManagerPrivate(this))
{

}

bool PluginManager::loadPlugin()
{
    //读取插件目录下的conf
    bool ret = d->readConf();

    //加入进程管理
    d->prepareProcess();

    return ret;
}

QList<GrandSearch::SearchPluginInfo> PluginManager::plugins() const
{
    if (d->m_loader)
        return d->m_loader->plugins();
}

void PluginManager::autoActivate()
{
    Q_ASSERT(d->m_loader);
    Q_ASSERT(d->m_process);

    QList<GrandSearch::SearchPluginInfo> plugins = d->m_loader->plugins();
    for (const GrandSearch::SearchPluginInfo &plugin : plugins) {
        //启动高优先级的Auto类型插件
        if (plugin.mode == GrandSearch::SearchPluginInfo::Auto
                && plugin.priority  == GrandSearch::SearchPluginInfo::High) {
            d->m_process->startProgram(plugin.name);
        }
    }
}
