// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewplugininterface.h"
#include "previewpluginmanager.h"
#include "generalpreviewplugin.h"
#include "utils/previewpluginconf.h"
#include "utils/utils.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

using namespace GrandSearch;

PreviewPluginManager::PreviewPluginManager()
    : QObject()
{
    qCDebug(logGrandSearch) << "Creating PreviewPluginManager";
    readPluginConfig();
    qCDebug(logGrandSearch) << "PreviewPluginManager created - Total plugins:" << m_plugins.size();
}

PreviewPluginManager::~PreviewPluginManager()
{
    qCDebug(logGrandSearch) << "Destroying PreviewPluginManager";
    clearPluginInfo();
}

PreviewPlugin *PreviewPluginManager::getPreviewPlugin(const MatchedItem &item)
{
    qCDebug(logGrandSearch) << "Getting preview plugin for item - Name:" << item.name
                            << "Type:" << item.type;

    PreviewPlugin *previewPlugin = nullptr;

    QString mimeType = item.type;
    if (mimeType.isEmpty())
        mimeType = Utils::getFileMimetype(item.item);

    if (!mimeType.isEmpty()) {
        for (auto pluginInfo : m_plugins) {
            if (!pluginInfo.bValid)
                continue;

            // 需支持正则表达式 todo
            if (isMimeTypeMatch(mimeType, pluginInfo.mimeTypes)) {
                if (nullptr == pluginInfo.pPlugin) {
                    // 加载预览插件
                    QPluginLoader *loader = new QPluginLoader(pluginInfo.path, this);
                    if (!loader->load()) {
                        qCWarning(logGrandSearch) << "Failed to load preview plugin - Path:" << pluginInfo.path << "Error:" << loader->errorString();
                        loader->deleteLater();
                        continue;
                    }

                    pluginInfo.pPlugin = loader;
                }

                // 从预览插件创建或获取预览界面
                QObject *pluginObject = pluginInfo.pPlugin->instance();
                if (PreviewPluginInterface *pluginIFace = qobject_cast<PreviewPluginInterface *>(pluginObject)) {
                    previewPlugin = pluginIFace->create(mimeType);
                    qCDebug(logGrandSearch) << "Created preview plugin - Name:" << pluginInfo.name
                                            << "MimeType:" << mimeType;
                }
                break;
            }
        }
    }

    return previewPlugin;
}

bool PreviewPluginManager::isMimeTypeMatch(const QString &mimetype, const QStringList &supportMimeTypes)
{
    bool match = false;
    for (const QString &mt : supportMimeTypes) {
        if (mimetype.compare(mt, Qt::CaseInsensitive) == 0) {
            match = true;
            break;
        }

        int starPos = mt.indexOf("*");
        if (starPos > 0 && mimetype.size() > starPos) {
            if (mt.left(starPos).compare(mimetype.left(starPos)) == 0) {
                match = true;
                break;
            }
        }
    }
    return match;
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
#ifdef QT_DEBUG
    char path[PATH_MAX] = { 0 };
    const char *defaultPath = realpath("./", path);
#else
    auto defaultPath = PLUGIN_PREVIEW_DIR;
#endif
    static_assert(std::is_same<decltype(defaultPath), const char *>::value, "PLUGIN_PREVIEW_DIR is not a string.");

    // 设置预览插件路径
    setPluginPath({ QString(defaultPath) });

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

        auto entrys = dir.entryInfoList({ "*.conf" }, QDir::Files, QDir::Name);
        for (const QFileInfo &entry : entrys) {
            PreviewPluginInfo info;
            if (readInfo(entry.absoluteFilePath(), info)) {
                // 检查插件版本是否向下兼容
                if (!downwardCompatibility(info.version)) {
                    qCWarning(logGrandSearch) << "Plugin version not supported - Version:" << info.version
                                              << "Plugin:" << info.name
                                              << "Required version:" << m_mainVersion;
                    continue;
                }

                qCInfo(logGrandSearch) << "Plugin loaded successfully - File:" << entry.fileName() << "Name:" << info.name;
                m_plugins.push_back(info);
            } else {
                qCWarning(logGrandSearch) << "Failed to read plugin configuration - Path:" << entry.absoluteFilePath();
            }
        }
    }

    return true;
}

bool PreviewPluginManager::readInfo(const QString &path, PreviewPluginInfo &info)
{
    qCDebug(logGrandSearch) << "Loading plugin configuration - Path:" << path;
    QSettings conf(path, QSettings::IniFormat);

    if (!conf.childGroups().contains(PREVIEW_PLUGINIFACE_CONF_ROOT))
        return false;

    conf.beginGroup(PREVIEW_PLUGINIFACE_CONF_ROOT);

    // 插件名称
    info.name = conf.value(PREVIEW_PLUGINIFACE_CONF_NAME, "").toString();
    if (info.name.isEmpty())
        return false;
    if (getPreviewPlugin(info.name)) {
        qCWarning(logGrandSearch) << "Duplicate plugin name found - Path:" << path << "Name:" << info.name;
        return false;
    }

    // 插件版本号
    info.version = conf.value(PREVIEW_PLUGINIFACE_CONF_VERSION, "").toString();
    if (info.version.isEmpty())
        return false;

        // 支持预览的mimetype列表
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    info.mimeTypes = conf.value(PREVIEW_PLUGINIFACE_CONF_MIMETYPES, "").toString().split(':', QString::SkipEmptyParts);
#else
    info.mimeTypes = conf.value(PREVIEW_PLUGINIFACE_CONF_MIMETYPES, "").toString().split(':', Qt::SkipEmptyParts);
#endif
    if (info.mimeTypes.isEmpty())
        return false;

    // 插件可用性
    info.bValid = conf.value(PREVIEW_PLUGINIFACE_CONF_VALID, "").toBool();

    // 填入插件所属路径
    QFileInfo fi(path);
    info.path = QDir::cleanPath(fi.path() + QLatin1Char('/') + "/lib" + fi.baseName() + ".so");

    return true;
}

PreviewPluginInfo *PreviewPluginManager::getPreviewPlugin(const QString &name)
{
    if (name.isEmpty())
        return nullptr;

    int nPluginCount = m_plugins.count();
    for (int i = 0; i < nPluginCount; i++) {
        if (m_plugins[i].name == name)
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
            qCDebug(logGrandSearch) << "Adding plugin path - Path:" << path;
            paths << path;
        } else {
            qCWarning(logGrandSearch) << "Invalid plugin path - Path:" << path;
        }
    }

    qCDebug(logGrandSearch) << "Updated plugin paths - Total paths:" << paths.size();
    m_paths = paths;
}

bool PreviewPluginManager::downwardCompatibility(const QString &version)
{
    Q_UNUSED(version);

    return true;
}
