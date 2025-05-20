// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginloader.h"
#include "utils/searchpluginconf.h"
#include "global/builtinsearch.h"
#include "searchplugin/convertors/dataconvertor.h"

#include <QDir>
#include <QSettings>
#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

PluginLoader::PluginLoader(QObject *parent)
    : QObject(parent)
{
}

void PluginLoader::setPluginPath(const QStringList &dirPaths)
{
    QStringList paths;
    for (const QString &path : dirPaths) {
        QDir dir(path);
        if (dir.isReadable()) {
            qCDebug(logDaemon) << "Adding plugin path:" << path;
            paths << path;
        } else {
            qCWarning(logDaemon) << "Invalid plugin path:" << path;
        }
    }

    qCInfo(logDaemon) << "Updated plugin paths - Total paths:" << paths.size();
    m_paths = paths;
}

bool PluginLoader::load()
{
    qCDebug(logDaemon) << "Starting plugin loading process";
    m_plugins.clear();
    for (const QString &path : m_paths) {
        QDir dir(path);
        if (!dir.isReadable()) {
            qCWarning(logDaemon) << "Plugin directory not readable:" << path;
            continue;
        }

        auto entrys = dir.entryInfoList({ "*.conf" }, QDir::Files, QDir::Name);
        qCDebug(logDaemon) << "Found" << entrys.size() << "configuration files in" << path;

        for (const QFileInfo &entry : entrys) {
            SearchPluginInfo info;
            if (readInfo(entry.absoluteFilePath(), info)) {
                // 检查是否与内置的搜索项目冲突
                DEF_BUILTISEARCH_NAMES;
                if (predefBuiltinSearches.contains(info.name)) {
                    qCWarning(logDaemon) << "Plugin conflicts with built-in search - Name:" << info.name
                                         << "Path:" << path;
                    continue;
                }

                if (m_plugins.contains(info.name)) {
                    qCWarning(logDaemon) << "Duplicate plugin name found - Name:" << info.name
                                         << "Path:" << path;
                    continue;
                }

                // 检查协议是否有效
                if (!DataConvIns->isSupported(info.ifsVersion)) {
                    qCWarning(logDaemon) << "Unsupported interface version - Plugin:" << info.name
                                         << "Version:" << info.ifsVersion;
                    continue;
                }

                qCInfo(logDaemon) << "Added plugin - File:" << entry.fileName()
                                  << "Name:" << info.name
                                  << "Mode:" << (info.mode == SearchPluginInfo::Mode::Auto ? "Auto" : info.mode == SearchPluginInfo::Mode::Trigger ? "Trigger"
                                                                                                                                                   : "Manual");
                m_plugins.insert(info.name, info);
            } else {
                qCWarning(logDaemon) << "Failed to read plugin configuration:" << entry.absoluteFilePath();
            }
        }
    }

    qCInfo(logDaemon) << "Plugin loading completed - Total plugins:" << m_plugins.size();
    return true;
}

QList<SearchPluginInfo> PluginLoader::plugins() const
{
    return m_plugins.values();
}

bool PluginLoader::getPlugin(const QString &name, SearchPluginInfo &plugin) const
{
    if (m_plugins.contains(name)) {
        plugin = m_plugins.value(name);
        return true;
    }

    return false;
}

bool PluginLoader::readInfo(const QString &path, SearchPluginInfo &info)
{
    qCDebug(logDaemon) << "Reading plugin configuration:" << path;
    QSettings conf(path, QSettings::IniFormat);

    if (!conf.childGroups().contains(PLUGININTERFACE_CONF_ROOT)) {
        qCWarning(logDaemon) << "Missing root configuration group in:" << path;
        return false;
    }

    conf.beginGroup(PLUGININTERFACE_CONF_ROOT);

    // 插件名称
    info.name = conf.value(PLUGININTERFACE_CONF_NAME, "").toString();
    if (info.name.isEmpty() || m_plugins.contains(info.name)) {
        qCWarning(logDaemon) << "Invalid or duplicate plugin name:" << info.name;
        return false;
    }

    // 通信接口版本
    info.ifsVersion = conf.value(PLUGININTERFACE_CONF_INTERFACEVERSION, "").toString();
    if (info.ifsVersion.isEmpty()) {
        qCWarning(logDaemon) << "Missing interface version for plugin:" << info.name;
        return false;
    }

    // 运行方式
    {
        info.mode = SearchPluginInfo::Mode::Manual;
        QString mode = conf.value(PLUGININTERFACE_CONF_MODE, "").toString().toLower();
        if (mode == PLUGININTERFACE_CONF_MODE_AUTO)
            info.mode = SearchPluginInfo::Mode::Auto;
        else if (mode == PLUGININTERFACE_CONF_MODE_TRIGGER)
            info.mode = SearchPluginInfo::Mode::Trigger;
    }

    if (info.mode == SearchPluginInfo::Mode::Auto) {
        // 优先级
        info.priority = static_cast<SearchPluginInfo::Priority>(conf.value(PLUGININTERFACE_CONF_PRIORITY, -1).toInt());
        if (info.priority < SearchPluginInfo::Priority::High || info.priority > SearchPluginInfo::Priority::Low) {
            qCDebug(logDaemon) << "Using default priority (Low) for plugin:" << info.name;
            info.priority = SearchPluginInfo::Priority::Low;
        }

        // 启动命令
        info.exec = conf.value(PLUGININTERFACE_CONF_EXEC, "").toString();
        if (info.exec.isEmpty()) {
            qCWarning(logDaemon) << "Missing execution command for auto-mode plugin:" << info.name;
            return false;
        }
    }

    // 服务名
    info.service = conf.value(PLUGININTERFACE_CONF_DBUSSERVICE, "").toString();
    if (info.service.isEmpty()) {
        qCWarning(logDaemon) << "Missing DBus service name for plugin:" << info.name;
        return false;
    }

    // 服务地址
    info.address = conf.value(PLUGININTERFACE_CONF_DBUSADDRESS, "").toString();
    if (info.address.isEmpty()) {
        qCWarning(logDaemon) << "Missing DBus address for plugin:" << info.name;
        return false;
    }

    // 接口名
    info.interface = conf.value(PLUGININTERFACE_CONF_DBUSINTERFACE, "").toString();
    if (info.interface.isEmpty()) {
        qCWarning(logDaemon) << "Missing DBus interface for plugin:" << info.name;
        return false;
    }

    info.from = path;
    qCDebug(logDaemon) << "Successfully read plugin configuration - Name:" << info.name
                       << "Version:" << info.ifsVersion;
    return true;
}
