// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "configer_p.h"
#include "preferenceitem.h"
#include "global/searchconfigdefine.h"
#include "global/commontools.h"

#include <QStandardPaths>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QReadLocker>
#include <QDebug>
#include <QDBusInterface>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

class ConfigerGlobal : public Configer
{
};
Q_GLOBAL_STATIC(ConfigerGlobal, configerGlobal)

ConfigerPrivate::ConfigerPrivate(Configer *parent)
    : q(parent)
{
}

UserPreferencePointer ConfigerPrivate::defaultSearcher()
{
    QVariantHash data = {
                        {GRANDSEARCH_CLASS_FILE_DEEPIN, true},
                        {GRANDSEARCH_CLASS_APP_DESKTOP, true},
                        {GRANDSEARCH_CLASS_SETTING_CONTROLCENTER, true},
                        {GRANDSEARCH_CLASS_WEB_STATICTEXT, true},
                        {GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC, true}
                        };

    return UserPreferencePointer(new UserPreference(data));
}

UserPreferencePointer ConfigerPrivate::fileSearcher()
{
    QVariantHash data = {
        { GRANDSEARCH_GROUP_FOLDER, true },
        { GRANDSEARCH_GROUP_FILE, true },
        { GRANDSEARCH_GROUP_FILE_VIDEO, true },
        { GRANDSEARCH_GROUP_FILE_AUDIO, true },
        { GRANDSEARCH_GROUP_FILE_PICTURE, true },
        { GRANDSEARCH_GROUP_FILE_DOCUMNET, true },
    };

    return UserPreferencePointer(new UserPreference(data));
}

UserPreferencePointer ConfigerPrivate::tailerData()
{
    QVariantHash data = {
        { GRANDSEARCH_TAILER_PARENTDIR, false },
        { GRANDSEARCH_TAILER_TIMEMODEFIED, true }
    };

    return UserPreferencePointer(new UserPreference(data));
}

UserPreferencePointer ConfigerPrivate::blacklist()
{
    QVariantHash data = {
        { GRANDSEARCH_BLACKLIST_PATH, QStringList("") }
    };

    return UserPreferencePointer(new UserPreference(data));
}

UserPreferencePointer ConfigerPrivate::webSearchEngine()
{
    QVariantHash data { { GRANDSEARCH_WEB_SEARCHENGINE, "" } };

    return UserPreferencePointer(new UserPreference(data));
}

UserPreferencePointer ConfigerPrivate::semanticEngine()
{
    QVariantHash data { { GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_ANALYSIS, true }, { GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_VECTOR, true } };

    return UserPreferencePointer(new UserPreference(data));
}

bool ConfigerPrivate::updateConfig1(QSettings *set)
{
    if (!set || m_root.isNull()) {
        qCWarning(logDaemon) << "Configuration update failed - Invalid parameters: set=" << (set != nullptr) << "root=" << !m_root.isNull();
        return false;
    }

    qCDebug(logDaemon) << "Starting configuration update from settings file";

    set->beginGroup(GRANDSEARCH_SEARCH_GROUP);
    UserPreferencePointer searcherConfig = m_root->group(GRANDSEARCH_PREF_SEARCHERENABLED);

    // 文件搜索相关配置
    {
        //初始化文件搜索的子类目
        if (UserPreferencePointer conf = m_root->group(GRANDSEARCH_CLASS_FILE_DEEPIN)) {
            // 若所有的文件类搜索都关闭，则关闭文件搜索项
            bool on = false;
            bool ret = set->value(GRANDSEARCH_GROUP_FOLDER, true).toBool();
            conf->setValue(GRANDSEARCH_GROUP_FOLDER, ret);
            on |= ret;

            ret = set->value(GRANDSEARCH_GROUP_FILE, true).toBool();
            conf->setValue(GRANDSEARCH_GROUP_FILE, ret);
            on |= ret;

            ret = set->value(GRANDSEARCH_GROUP_FILE_VIDEO, true).toBool();
            conf->setValue(GRANDSEARCH_GROUP_FILE_VIDEO, ret);
            on |= ret;

            ret = set->value(GRANDSEARCH_GROUP_FILE_AUDIO, true).toBool();
            conf->setValue(GRANDSEARCH_GROUP_FILE_AUDIO, ret);
            on |= ret;

            ret = set->value(GRANDSEARCH_GROUP_FILE_PICTURE, true).toBool();
            conf->setValue(GRANDSEARCH_GROUP_FILE_PICTURE, ret);
            on |= ret;

            ret = set->value(GRANDSEARCH_GROUP_FILE_DOCUMNET, true).toBool();
            conf->setValue(GRANDSEARCH_GROUP_FILE_DOCUMNET, ret);
            on |= ret;

            // 设置是否启用文件搜索项
            searcherConfig->setValue(GRANDSEARCH_CLASS_FILE_DEEPIN, on);
            qCDebug(logDaemon) << "File search configuration updated - Enabled:" << on;
        } else {
            qCWarning(logDaemon) << "Configuration not found:" << GRANDSEARCH_CLASS_FILE_DEEPIN;
        }
    }

    // 设置是否启用设置搜索项
    {
        bool on = set->value(GRANDSEARCH_GROUP_SETTING, true).toBool();
        searcherConfig->setValue(GRANDSEARCH_CLASS_SETTING_CONTROLCENTER, on);
        qCDebug(logDaemon) << "Settings search configuration updated - Enabled:" << on;
    }

    // 设置是否启用应用搜索项
    {
        bool on = set->value(GRANDSEARCH_GROUP_APP, true).toBool();
        searcherConfig->setValue(GRANDSEARCH_CLASS_APP_DESKTOP, on);
        qCDebug(logDaemon) << "Application search configuration updated - Enabled:" << on;
    }

    // 设置是否启用设置搜索项
    {
        bool on = set->value(GRANDSEARCH_GROUP_SETTING, true).toBool();
        searcherConfig->setValue(GRANDSEARCH_CLASS_SETTING_CONTROLCENTER, on);
    }

    // 设置是否启用web搜索项
    {
        bool on = set->value(GRANDSEARCH_GROUP_WEB, true).toBool();
        searcherConfig->setValue(GRANDSEARCH_CLASS_WEB_STATICTEXT, on);
        qCDebug(logDaemon) << "Web search configuration updated - Enabled:" << on;
    }

    set->endGroup();

    // AI搜索子项开关
    {
        set->beginGroup(GRANDSEARCH_SEMANTIC_GROUP);
        if (UserPreferencePointer conf = m_root->group(GRANDSEARCH_SEMANTIC_GROUP)) {
            bool on = false;
            bool ret = set->value(GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_ANALYSIS, true).toBool();
            conf->setValue(GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_ANALYSIS, ret);
            on |= ret;

            ret = set->value(GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_VECTOR, true).toBool();
            conf->setValue(GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_VECTOR, ret);
            on |= ret;

            ret = set->value(GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_FULLTEXT, false).toBool();
            conf->setValue(GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_FULLTEXT, ret);

            // 设置是否启用AI搜索项
            searcherConfig->setValue(GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC, on);
            qCDebug(logDaemon) << "Semantic search configuration updated - Enabled:" << on;
        } else {
            qCWarning(logDaemon) << "Configuration not found:" << GRANDSEARCH_SEMANTIC_GROUP;
        }

        set->endGroup();
    }

    // 拖尾
    set->beginGroup(GRANDSEARCH_TAILER_GROUP);
    if (UserPreferencePointer conf = m_root->group(GRANDSEARCH_TAILER_GROUP)) {
        bool ret = set->value(GRANDSEARCH_TAILER_PARENTDIR, false).toBool();
        conf->setValue(GRANDSEARCH_TAILER_PARENTDIR, ret);

        ret = set->value(GRANDSEARCH_TAILER_TIMEMODEFIED, true).toBool();
        conf->setValue(GRANDSEARCH_TAILER_TIMEMODEFIED, ret);
        qCDebug(logDaemon) << "Tailer configuration updated - Parent dir:" << conf->value<bool>(GRANDSEARCH_TAILER_PARENTDIR, false)
                           << "Time modified:" << conf->value<bool>(GRANDSEARCH_TAILER_TIMEMODEFIED, true);
    } else {
        qCWarning(logDaemon) << "Configuration not found:" << GRANDSEARCH_TAILER_GROUP;
    }

    set->endGroup();

    // 路径黑名单
    set->beginGroup(GRANDSEARCH_BLACKLIST_GROUP);
    if (UserPreferencePointer conf = m_root->group(GRANDSEARCH_BLACKLIST_GROUP)) {
        const QStringList blacklist = set->value(GRANDSEARCH_BLACKLIST_PATH, QStringList()).toStringList();
        QStringList vaild;
        qCDebug(logDaemon) << "Processing blacklist paths - Original count:" << blacklist.size();

        for (QString path : blacklist) {
            path = QByteArray::fromBase64(path.toLocal8Bit());
            resetPath(path);
            bool isChild = false;
            for (const auto &p : vaild) {
                if (path.startsWith(p)) {
                    isChild = true;
                    break;
                }
                if (p.startsWith(path))
                    vaild.removeOne(p);
            }
            if (!isChild)
                vaild.append(path);
        }

        conf->setValue(GRANDSEARCH_BLACKLIST_PATH, vaild);
        qCDebug(logDaemon) << "Blacklist configuration updated - Valid paths count:" << vaild.size();
    } else {
        qCWarning(logDaemon) << "Configuration not found:" << GRANDSEARCH_BLACKLIST_GROUP;
    }
    set->endGroup();

    // 搜索引擎
    set->beginGroup(GRANDSEARCH_WEB_GROUP);
    if (UserPreferencePointer conf = m_root->group(GRANDSEARCH_WEB_GROUP)) {
        auto searchEngine = set->value(GRANDSEARCH_WEB_SEARCHENGINE).toString();
        conf->setValue(GRANDSEARCH_WEB_SEARCHENGINE, searchEngine);

        auto searchEngineCustomAddr = set->value(GRANDSEARCH_WEB_SEARCHENGINE_CUSTOM_ADDR).toString();
        conf->setValue(GRANDSEARCH_WEB_SEARCHENGINE_CUSTOM_ADDR, searchEngineCustomAddr);
        qCDebug(logDaemon) << "Web search engine configuration updated - Engine:" << searchEngine
                           << "Custom address:" << searchEngineCustomAddr;
    } else {
        qCWarning(logDaemon) << "Configuration not found:" << GRANDSEARCH_WEB_SEARCHENGINE;
    }
    set->endGroup();

    qCDebug(logDaemon) << "Configuration update completed successfully";
    return true;
}

void ConfigerPrivate::resetPath(QString &path) const
{
    // If the path is the root directory, do nothing
    if (path == QString("/"))
        path = QString("");

    const auto &homeBindPath = CommonTools::bindPathTransform("/home", true);
    if (path.startsWith(homeBindPath))
        path.replace(homeBindPath, "/home");

    // Add "/" at the end of path to distinguish the paths with the same URL part
    path += "/";
}

Configer::Configer(QObject *parent)
    : QObject(parent),
      d(new ConfigerPrivate(this))
{
    qCDebug(logDaemon) << "Configer constructor - Initializing configuration manager";
    d->m_delayLoad.setSingleShot(true);
    d->m_delayLoad.setInterval(50);
    connect(&d->m_delayLoad, &QTimer::timeout, this, &Configer::onLoadConfig);
}

Configer::~Configer()
{
    qCDebug(logDaemon) << "Configer destructor - Cleaning up configuration manager";
    delete d;
    d = nullptr;
}

Configer *Configer::instance()
{
    return configerGlobal;
}

bool Configer::init()
{
    qCDebug(logDaemon) << "Configuration manager initialization started";

    initDefault();

    auto configPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
    configPath = configPath
            + "/" + QCoreApplication::organizationName()
            + "/" + GRANDSEARCH_DAEMON_NAME
            + "/" + GRANDSEARCH_DAEMON_NAME + ".conf";

    qCDebug(logDaemon) << "Configuration file path:" << configPath;

    QFileInfo configFile(configPath);
    if (!configFile.exists()) {
        configFile.absoluteDir().mkpath(".");

        // 生成文件
        QFile file(configPath);
        file.open(QFile::NewOnly);
        file.close();
        qCInfo(logDaemon) << "Created configuration file:" << configPath;
    }
    d->m_configPath = configFile.absoluteFilePath();

    if (d->m_watcher)
        delete d->m_watcher;

    d->m_watcher = new QFileSystemWatcher(this);
    d->m_watcher->addPath(configFile.absolutePath());
    d->m_watcher->addPath(configFile.absoluteFilePath());
    connect(d->m_watcher, &QFileSystemWatcher::fileChanged, this, &Configer::onFileChanged);
    connect(d->m_watcher, &QFileSystemWatcher::directoryChanged, this, &Configer::onFileChanged);

    onLoadConfig();
    qCDebug(logDaemon) << "Configuration manager initialization completed successfully";
    return true;
}

UserPreferencePointer Configer::group(const QString &name) const
{
    QReadLocker lk(&d->m_rwLock);
    if (d->m_root.isNull())
        return nullptr;

    return d->m_root->group(name);
}

void Configer::initDefault()
{
    qCDebug(logDaemon) << "Initializing default configuration values";

    QVariantHash rootData;

    // 初始化搜索项是否可用
    rootData.insert(GRANDSEARCH_PREF_SEARCHERENABLED, QVariant::fromValue(d->defaultSearcher()));

    //初始化文件搜索的子类目
    rootData.insert(GRANDSEARCH_CLASS_FILE_DEEPIN, QVariant::fromValue(d->fileSearcher()));

    // 拖尾
    rootData.insert(GRANDSEARCH_TAILER_GROUP, QVariant::fromValue(d->tailerData()));

    // 路径黑名单
    rootData.insert(GRANDSEARCH_BLACKLIST_GROUP, QVariant::fromValue(d->blacklist()));

    // 搜索引擎
    rootData.insert(GRANDSEARCH_WEB_GROUP, QVariant::fromValue(d->webSearchEngine()));

    // AI搜索子项
    rootData.insert(GRANDSEARCH_SEMANTIC_GROUP, QVariant::fromValue(d->semanticEngine()));

    {
        UserPreferencePointer root(new UserPreference(rootData));
        QWriteLocker lk(&d->m_rwLock);
        d->m_root = root;
    }

    qCDebug(logDaemon) << "Default configuration values initialized successfully";
}

void Configer::onFileChanged(const QString &path)
{
    qCDebug(logDaemon) << "Configuration file changed:" << path;
    d->m_delayLoad.start();
}

void Configer::onLoadConfig()
{
    qCDebug(logDaemon) << "Loading configuration";
    if (d->m_configPath.isEmpty()) {
        qCWarning(logDaemon) << "Configuration path is empty, cannot load config";
        return;
    }

    QFileInfo configFile(d->m_configPath);
    if (!configFile.exists()) {
        qCWarning(logDaemon) << "Configuration file not found:" << d->m_configPath;
        return;
    }

    QSettings set(d->m_configPath, QSettings::IniFormat);
    if (set.status() != QSettings::NoError) {
        qCWarning(logDaemon) << "Configuration file error - Status:" << set.status();
        return;
    }

    if (!set.contains("Version_Group/version.config")) {
        qCDebug(logDaemon) << "Configuration file does not contain version information, skipping load";
        return;
    }

    QString ver = set.value("Version_Group/version.config", QString()).toString();
    if (ver.isEmpty()) {
        qCWarning(logDaemon) << "Configuration file error: Version not found";
        return;
    }

    qCInfo(logDaemon) << "Configuration file version:" << ver;
    d->updateConfig1(&set);
}
