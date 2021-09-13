/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     houchengqiu<houchengqiu@uniontech.com>
 *
 * Maintainer: houchengqiu<houchengqiu@uniontech.com>
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
#ifndef PREVIEWPLUGINMANAGER_H
#define PREVIEWPLUGINMANAGER_H

#include "gui/datadefine.h"
#include "global/matcheditem.h"

namespace GrandSearch {
class PreviewPlugin;
}

class PreviewPluginManager : QObject
{
public:
    explicit PreviewPluginManager();
    ~PreviewPluginManager();

    GrandSearch::PreviewPlugin *getPreviewPlugin(const GrandSearch::MatchedItem& item);

private:
    void clearPluginInfo();
    bool readPluginConfig();
    bool loadConfig();
    bool readInfo(const QString &path, GrandSearch::PreviewPluginInfo &info);

    GrandSearch::PreviewPluginInfo *getPreviewPlugin(const QString &name);
    void setPluginPath(const QStringList &dirPaths);
    // 插件版本是否向下兼容
    bool downwardCompatibility(const QString &version);

private:
    QStringList m_paths;
    GrandSearch::PreviewPluginInfoList m_plugins;
    QString m_mainVersion;
};

#endif // PREVIEWPLUGINMANAGER_H
