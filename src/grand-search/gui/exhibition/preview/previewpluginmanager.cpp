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
#include "previewplugininterface.h"
#include "previewpluginmanager.h"
#include "generalpreviewplugin.h"
#include "utils/previewpluginconf.h"
#include "utils/utils.h"

using namespace GrandSearch;

PreviewPluginManager::PreviewPluginManager()
    : QObject ()
{
    m_generalPreviewPlugin = new GeneralPreviewPlugin();

    readPluginConfig();
}

PreviewPluginManager::~PreviewPluginManager()
{
    delete m_generalPreviewPlugin;

    clearPluginInfo();
}

PreviewPlugin *PreviewPluginManager::getPreviewPlugin(const GrandSearch::MatchedItem &item)
{
    PreviewPlugin* previewPlugin = nullptr;

    QString mimeType = item.type;
    if (mimeType.isEmpty())
        mimeType = Utils::getFileMimetype(item.item);

    if (!mimeType.isEmpty()) {
        for (auto pluginInfo : m_plugins) {
            if (!pluginInfo.bValid)
                continue;

            if (pluginInfo.mimeTypes.contains(mimeType)) {
                if (nullptr == pluginInfo.pPlugin) {
                    // 加载预览插件
                    QPluginLoader* loader = new QPluginLoader(pluginInfo.path, this);
                    if (!loader->load()) {
                        loader->deleteLater();
                        continue;
                    }

                    pluginInfo.pPlugin = loader;
                }

                // 从预览插件创建或获取预览界面
                QObject* pluginObject = pluginInfo.pPlugin->instance();
                if (PreviewPluginInterface * pluginIFace = qobject_cast<PreviewPluginInterface*>(pluginObject))
                    previewPlugin = pluginIFace->create(mimeType);
                break;
            }
        }
    }

    if (nullptr == previewPlugin)
        previewPlugin = m_generalPreviewPlugin;

    return previewPlugin;
}

void PreviewPluginManager::clearPluginInfo()
{
    for (auto pluginInfo : m_plugins) {
        if (pluginInfo.pPlugin)
            delete pluginInfo.pPlugin;
    }

    m_plugins.clear();
}

bool PreviewPluginManager::readPluginConfig()
{
    // 获取预览插件路径
#ifndef QT_DEBUG
    const char *defaultPath = realpath("./", nullptr);
#else
    auto defaultPath = PLUGIN_PREVIEW_DIR;
#endif
    static_assert(std::is_same<decltype(defaultPath), const char *>::value, "PLUGIN_PREVIEW_DIR is not a string.");

    // 设置预览插件路径
    setPluginPath({QString(defaultPath)});

    // 从预览插件路径加载预览插件信息
    return loadConfig();
}

bool PreviewPluginManager::loadConfig()
{
    // 先清空已有插件信息
    clearPluginInfo();

    for (const QString &path : m_paths) {
        QDir dir(path);
        if (!dir.isReadable())
            continue;

        auto entrys = dir.entryInfoList({"*.conf"}, QDir::Files, QDir::Name);
        for (const QFileInfo &entry : entrys) {
            GrandSearch::PreviewPluginInfo info;
            if (readInfo(entry.absoluteFilePath(), info)) {
                // 检查插件版本是否向下兼容
                if (!downwardCompatibility(info.version)) {
                    qWarning() << QString("do not support this version(%1), plugin name:%2 plugin version:%3").arg(m_mainVersion).arg(info.name).arg(info.version);
                    continue;
                }

                qInfo() << "add plugin info" << entry.fileName() << info.name;
                m_plugins.push_back(info);
            } else {
                qWarning() << "plugin info error:" << entry.absoluteFilePath();
            }
        }
    }

    return true;
}

bool PreviewPluginManager::readInfo(const QString &path, GrandSearch::PreviewPluginInfo &info)
{
    qDebug() << "load conf" << path;
    QSettings conf(path, QSettings::IniFormat);

    if (!conf.childGroups().contains(PREVIEW_PLUGINIFACE_CONF_ROOT))
        return false;

    conf.beginGroup(PREVIEW_PLUGINIFACE_CONF_ROOT);

    // 插件名称
    info.name = conf.value(PREVIEW_PLUGINIFACE_CONF_NAME, "").toString();
    if (info.name.isEmpty())
        return false;
    if (getPreviewPlugin(info.name)) {
        qWarning() << "duplicate plugin name" << path << info.name;
        return false;
    }

    // 插件iid
    info.iid = conf.value(PREVIEW_PLUGINIFACE_CONF_IID, "").toString();
    if (info.name.isEmpty() || getPreviewPluginInfoById(info.iid))
        return false;

    // 插件版本号
    info.version = conf.value(PREVIEW_PLUGINIFACE_CONF_VERSION, "").toString();
    if (info.version.isEmpty())
        return false;

    // 支持预览的mimetype列表
    info.mimeTypes = conf.value(PREVIEW_PLUGINIFACE_CONF_MIMETYPES, "").toString().split(',');
    if (info.mimeTypes.isEmpty())
        return false;

    // 插件可用性
    info.bValid = conf.value(PREVIEW_PLUGINIFACE_CONF_VALID, "").toBool();

    // 填入插件所属路径
    QFileInfo fi(path);
    info.path = QDir::cleanPath(fi.path() + QLatin1Char('/') + "/lib" + fi.fileName() + ".so");

    return true;
}

GrandSearch::PreviewPluginInfo *PreviewPluginManager::getPreviewPlugin(const QString &name)
{
    if (name.isEmpty())
        return nullptr;

    int nPluginCount = m_plugins.count();
    for (int i = 0; i < nPluginCount; i++) {
      if(m_plugins[i].name == name)
          return &m_plugins[i];
    }

    return nullptr;
}

GrandSearch::PreviewPluginInfo *PreviewPluginManager::getPreviewPluginInfoById(const QString &iid)
{
    if (iid.isEmpty())
        return nullptr;

    int nPluginCount = m_plugins.count();
    for (int i = 0; i < nPluginCount; i++) {
      if(m_plugins[i].iid == iid)
          return &m_plugins[i];
    }

    return nullptr;
}

void PreviewPluginManager::setPluginPath(const QStringList &dirPaths)
{
    QStringList paths;
    for (const QString &path : dirPaths) {
        QDir dir(path);
        if (dir.isReadable()) {
            qDebug() << "add plugin path:" << path;
            paths << path;
        }
        else
            qWarning() << "invaild plugin path:" << path;
    }

    qDebug() << "update plugin paths" << paths.size();
    m_paths = paths;
}

bool PreviewPluginManager::downwardCompatibility(const QString &version)
{
    Q_UNUSED(version);

    return true;
}
