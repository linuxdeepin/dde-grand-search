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

#include "configer_p.h"
#include "preferenceitem.h"
#include "global/searchconfigdefine.h"

#include <QStandardPaths>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QReadLocker>
#include <QDebug>

class ConfigerGlobal : public Configer {};
Q_GLOBAL_STATIC(ConfigerGlobal, configerGlobal)

ConfigerPrivate::ConfigerPrivate(Configer *parent)
    : q(parent)
{

}

UserPreferencePointer ConfigerPrivate::defaultSearcher()
{
    QVariantHash data = {
        #ifdef ENABLE_DEEPINANYTHING
                        {GRANDSEARCH_CLASS_FILE_DEEPIN, true},
        #endif
        #ifdef ENABLE_FSEARCH
                         {GRANDSEARCH_CLASS_FILE_FSEARCH, true},
        #endif
                         {GRANDSEARCH_CLASS_APP_DESKTOP, true},
                         {GRANDSEARCH_CLASS_SETTING_CONTROLCENTER, true},
                         {GRANDSEARCH_CLASS_WEB_STATICTEXT, true}
                        };

    return UserPreferencePointer(new UserPreference(data));
}

UserPreferencePointer ConfigerPrivate::fileSearcher()
{
    QVariantHash data = {{GRANDSEARCH_GROUP_FOLDER, true},
                         {GRANDSEARCH_GROUP_FILE, true},
                         {GRANDSEARCH_GROUP_FILE_VIDEO, true},
                         {GRANDSEARCH_GROUP_FILE_AUDIO, true},
                         {GRANDSEARCH_GROUP_FILE_PICTURE, true},
                         {GRANDSEARCH_GROUP_FILE_DOCUMNET, true},
                        };

    return UserPreferencePointer(new UserPreference(data));
}

UserPreferencePointer ConfigerPrivate::tailerData()
{
    QVariantHash data = {
        {GRANDSEARCH_TAILER_PARENTDIR, false},
        {GRANDSEARCH_TAILER_TIMEMODEFIED, true}
    };

    return UserPreferencePointer(new UserPreference(data));
}

UserPreferencePointer ConfigerPrivate::blacklist()
{
    QVariantHash data = {
        {GRANDSEARCH_BLACKLIST_PATH, QStringList("")}
    };

    return UserPreferencePointer(new UserPreference(data));
}

bool ConfigerPrivate::updateConfig1(QSettings *set)
{
    if (!set || m_root.isNull())
        return false;

    set->beginGroup(GRANDSEARCH_SEARCH_GROUP);
    UserPreferencePointer searcherConfig = m_root->group(GRANDSEARCH_PREF_SEARCHERENABLED);
    Q_ASSERT(searcherConfig);

    //文件搜索相关配置
    {
        //初始化文件搜索的子类目
    #ifdef ENABLE_DEEPINANYTHING
        if (UserPreferencePointer conf = m_root->group(GRANDSEARCH_CLASS_FILE_DEEPIN)) {
            //若所有的文件类搜索都关闭，则关闭文件搜索项
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

            //设置是否启用文件搜索项
            searcherConfig->setValue(GRANDSEARCH_CLASS_FILE_DEEPIN, on);
        } else {
            qWarning() << "no shuch config:" << GRANDSEARCH_CLASS_FILE_DEEPIN;
        }
    #endif

    #ifdef ENABLE_FSEARCH
        if (UserPreferencePointer conf = m_root->group(GRANDSEARCH_CLASS_FILE_FSEARCH)) {
            //若所有的文件类搜索都关闭，则关闭文件搜索项
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

            //设置是否启用文件搜索项
            searcherConfig->setValue(GRANDSEARCH_CLASS_FILE_FSEARCH, on);
        } else {
            qWarning() << "no shuch config:" << GRANDSEARCH_CLASS_FILE_FSEARCH;
        }
    #endif
    }

    //设置是否启用设置搜索项
    {
        bool on = set->value(GRANDSEARCH_GROUP_SETTING, true).toBool();
        searcherConfig->setValue(GRANDSEARCH_CLASS_SETTING_CONTROLCENTER, on);
    }

    //设置是否启用应用搜索项
    {
        bool on = set->value(GRANDSEARCH_GROUP_APP, true).toBool();
        searcherConfig->setValue(GRANDSEARCH_CLASS_APP_DESKTOP, on);
    }

    //设置是否启用设置搜索项
    {
        bool on = set->value(GRANDSEARCH_GROUP_SETTING, true).toBool();
        searcherConfig->setValue(GRANDSEARCH_CLASS_SETTING_CONTROLCENTER, on);
    }

    //设置是否启用web搜索项
    {
        bool on = set->value(GRANDSEARCH_GROUP_WEB, true).toBool();
        searcherConfig->setValue(GRANDSEARCH_CLASS_WEB_STATICTEXT, on);
    }

    set->endGroup();

    // 拖尾
    set->beginGroup(GRANDSEARCH_TAILER_GROUP);
    if (UserPreferencePointer conf = m_root->group(GRANDSEARCH_TAILER_GROUP)) {
        bool ret = set->value(GRANDSEARCH_TAILER_PARENTDIR, false).toBool();
        conf->setValue(GRANDSEARCH_TAILER_PARENTDIR, ret);

        ret = set->value(GRANDSEARCH_TAILER_TIMEMODEFIED, true).toBool();
        conf->setValue(GRANDSEARCH_TAILER_TIMEMODEFIED, ret);
    } else {
        qWarning() << "no shuch config:" << GRANDSEARCH_TAILER_GROUP;
    }

    set->endGroup();

    // 路径黑名单
    set->beginGroup(GRANDSEARCH_BLACKLIST_GROUP);
    if (UserPreferencePointer conf = m_root->group(GRANDSEARCH_BLACKLIST_GROUP)) {
        auto blacklist = set->value(GRANDSEARCH_BLACKLIST_PATH, QStringList()).toStringList();
        for (auto &path : blacklist) {
            path = QByteArray::fromBase64(path.toLocal8Bit());
        }

        conf->setValue(GRANDSEARCH_BLACKLIST_PATH, blacklist);
    } else {
        qWarning() << "no shuch config:" << GRANDSEARCH_BLACKLIST_GROUP;
    }
    set->endGroup();
    return true;
}

Configer::Configer(QObject *parent) : QObject(parent)
  , d(new ConfigerPrivate(this))
{
    d->m_delayLoad.setSingleShot(true);
    d->m_delayLoad.setInterval(50);
    connect(&d->m_delayLoad, &QTimer::timeout, this, &Configer::onLoadConfig);
}

Configer::~Configer()
{
    delete d;
    d = nullptr;
}

Configer *Configer::instance()
{
    return configerGlobal;
}

bool Configer::init()
{
    initDefault();

    auto configPath = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first();
    configPath = configPath
                 + "/" + QCoreApplication::organizationName()
                 + "/" + QCoreApplication::applicationName()
                 + "/" + QCoreApplication::applicationName() + ".conf";

    QFileInfo configFile(configPath);
    if (!configFile.exists()) {
        configFile.absoluteDir().mkpath(".");

        //生成文件
        QFile file(configPath);
        file.open(QFile::NewOnly);
        file.close();
        qInfo() << "create conf " << configPath;
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
    QVariantHash rootData;

    //初始化搜索项是否可用
    rootData.insert(GRANDSEARCH_PREF_SEARCHERENABLED, QVariant::fromValue(d->defaultSearcher()));

    //初始化文件搜索的子类目
#ifdef ENABLE_DEEPINANYTHING
    rootData.insert(GRANDSEARCH_CLASS_FILE_DEEPIN, QVariant::fromValue(d->fileSearcher()));
#endif

#ifdef ENABLE_FSEARCH
    rootData.insert(GRANDSEARCH_CLASS_FILE_FSEARCH, QVariant::fromValue(d->fileSearcher()));
#endif

    // 拖尾
    rootData.insert(GRANDSEARCH_TAILER_GROUP, QVariant::fromValue(d->tailerData()));

    // 路径黑名单
    rootData.insert(GRANDSEARCH_BLACKLIST_GROUP, QVariant::fromValue(d->blacklist()));

    {
        UserPreferencePointer root(new UserPreference(rootData));
        QWriteLocker lk(&d->m_rwLock);
        d->m_root = root;
    }
}

void Configer::onFileChanged(const QString &path)
{
    qDebug() << "config-file changed" << path;
    d->m_delayLoad.start();
}

void Configer::onLoadConfig()
{
    qDebug() << __FUNCTION__;
    if (d->m_configPath.isEmpty())
        return;

    QFileInfo configFile(d->m_configPath);
    if (!configFile.exists()) {
        qWarning() << "config file losted";
        return;
    }

    QSettings set(d->m_configPath, QSettings::IniFormat);
    if (set.status() != QSettings::NoError) {
        qWarning() << "config file error" << set.status();
        return;
    }

    if (!set.contains("Version_Group/version.config"))
        return;

    QString ver = set.value("Version_Group/version.config", QString()).toString();
    if (ver.isEmpty()) {
        qWarning() << "config file error: no version.";
        return;
    }

    qInfo() << "config file version is" << ver;
    d->updateConfig1(&set);
}
