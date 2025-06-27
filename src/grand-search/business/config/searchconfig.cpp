// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchconfig.h"
#include "global/builtinsearch.h"
#include "global/searchconfigdefine.h"

#include <QStandardPaths>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QMutexLocker>
#include <QtDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

using namespace GrandSearch;

class SearchConfigGlobal : public SearchConfig
{
};
Q_GLOBAL_STATIC(SearchConfigGlobal, searchConfigGlobal)

SearchConfig *SearchConfig::instance()
{
    return searchConfigGlobal;
}

SearchConfig::SearchConfig()
{
    auto configPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
    configPath = configPath
            + "/" + QApplication::organizationName()
            + "/" + GRANDSEARCH_DAEMON_NAME
            + "/" + GRANDSEARCH_DAEMON_NAME + ".conf";

    qCDebug(logGrandSearch) << "Initializing search configuration - Path:" << configPath;

    QFileInfo configFile(configPath);
    if (!configFile.exists()) {
        qCDebug(logGrandSearch) << "Creating configuration directory structure";
        configFile.absoluteDir().mkpath(".");

        // 根据要求，明确生成文件，而非等设置值时自动生成文件
        QFile file(configPath);
        file.open(QFile::NewOnly);
        file.close();
        qCInfo(logGrandSearch) << "Created new configuration file:" << configPath;
    }

    m_settings = new QSettings(configPath, QSettings::IniFormat);

    m_settings->beginGroup(GRANDSEARCH_VERSION_GROUP);
    if (m_settings->childKeys().contains(GRANDSEARCH_VERSION_CONFIG)) {
        QString curVersion = m_settings->value(GRANDSEARCH_VERSION_CONFIG).toString();
        if (curVersion != GRANDSEARCH_VERSION_CONFIG_CURRENT) {
            qCWarning(logGrandSearch) << "Configuration version mismatch - Current:" << curVersion
                                      << "Required:" << GRANDSEARCH_VERSION_CONFIG_CURRENT;
            qCDebug(logGrandSearch) << "Converting configuration file to latest version";
            convertConfigFile();
            m_settings->setValue(GRANDSEARCH_VERSION_CONFIG, GRANDSEARCH_VERSION_CONFIG_CURRENT);
        }
    } else {
        qCDebug(logGrandSearch) << "Setting initial configuration version:" << GRANDSEARCH_VERSION_CONFIG_CURRENT;
        m_settings->setValue(GRANDSEARCH_VERSION_CONFIG, GRANDSEARCH_VERSION_CONFIG_CURRENT);
    }
    m_settings->endGroup();
}

SearchConfig::~SearchConfig()
{
    delete m_settings;
    m_settings = nullptr;
}

void SearchConfig::convertConfigFile()
{
    // 配置文件升级（版本转换为最新版本）
    // 针对版本差异进行处理
}

QVariant SearchConfig::getConfig(const QString &group, const QString &key, const QVariant &defaultValue)
{
    QMutexLocker lk(&m_mutex);
    m_settings->beginGroup(group);
    QVariant result = m_settings->value(key, defaultValue);
    m_settings->endGroup();

    qCDebug(logGrandSearch) << "Config retrieved - Group:" << group << "Key:" << key
                            << "HasValue:" << !result.isNull() << "Type:" << result.typeName();
    return result;
}

void SearchConfig::setConfig(const QString &group, const QString &key, const QVariant &value)
{
    QMutexLocker lk(&m_mutex);
    m_settings->beginGroup(group);
    m_settings->setValue(key, value);
    m_settings->endGroup();

    qCDebug(logGrandSearch) << "Config set - Group:" << group << "Key:" << key
                            << "Value type:" << value.typeName();
}

void SearchConfig::removeConfig(const QString &group, const QString &key)
{
    QMutexLocker lk(&m_mutex);
    m_settings->beginGroup(group);
    m_settings->remove(key);
    m_settings->endGroup();
}

void SearchConfig::setConfigList(const QString &group, const QStringList &keys, const QVariantList &values)
{
    QMutexLocker lk(&m_mutex);
    m_settings->beginGroup(group);
    for (int i = 0; i < keys.length(); ++i) {
        m_settings->setValue(keys.value(i), values.value(i));
    }
    m_settings->endGroup();
    qCDebug(logGrandSearch) << "Config list set - Group:" << group << "Count:" << keys.size();
}

void SearchConfig::removeConfigList(const QString &group, const QStringList &keys)
{
    QMutexLocker lk(&m_mutex);
    m_settings->beginGroup(group);
    for (int i = 0; i < keys.length(); ++i) {
        m_settings->remove(keys.value(i));
    }
    m_settings->endGroup();
}
