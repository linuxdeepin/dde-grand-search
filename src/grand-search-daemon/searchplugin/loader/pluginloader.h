// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
