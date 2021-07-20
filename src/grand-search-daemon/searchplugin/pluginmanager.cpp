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
#include "loader/pluginloader.h"

#include <QStandardPaths>
#include <QDebug>

PluginManagerPrivate::PluginManagerPrivate(PluginManager *parent)
    : QObject(parent)
    , q(parent)
{

}

PluginManager::PluginManager(QObject *parent)
    : QObject(parent)
    , d(new PluginManagerPrivate(this))
{

}

bool PluginManager::loadPlugin()
{
    if (!d->m_loader) {
        d->m_loader = new PluginLoader(this);

        auto defaultPath = PLUGIN_SEARCHER_DIR;
        static_assert(std::is_same<decltype(defaultPath), const char *>::value, "PLUGIN_SEARCHER_DIR is not a string.");

        d->m_loader->setPluginPath({QString(defaultPath)});
    }

    return d->m_loader->load();
}
