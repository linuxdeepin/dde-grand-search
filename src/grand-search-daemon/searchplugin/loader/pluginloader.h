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
#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include "utils/searchplugininfo.h"

#include <QObject>
#include <QMap>

class PluginLoader : public QObject
{
    Q_OBJECT
public:
    explicit PluginLoader(QObject *parent = nullptr);
    void setPluginPath(const QStringList &dirPaths);
    bool load();
    QList<GrandSearch::SearchPluginInfo> plugins() const;
    bool getPlugin(const QString &name, GrandSearch::SearchPluginInfo &plugin) const;

protected:
    bool readInfo(const QString &path, GrandSearch::SearchPluginInfo &info);
private:
    QStringList m_paths;
    QMap<QString, GrandSearch::SearchPluginInfo> m_plugins;
};

#endif // PLUGINLOADER_H
