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
#ifndef CONFIGER_P_H
#define CONFIGER_P_H

#include "configuration/configer.h"
#include "userpreference.h"

#include <QPointer>
#include <QReadWriteLock>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QSettings>

namespace GrandSearch {

class ConfigerPrivate
{
    friend class Configer;
public:
    ConfigerPrivate(Configer *parent);
    static UserPreferencePointer defaultSearcher();
    static UserPreferencePointer fileSearcher();
    static UserPreferencePointer tailerData();
    static UserPreferencePointer blacklist();
    static UserPreferencePointer webSearchEngine();
    bool updateConfig1(QSettings *);
    void resetPath(QString &path) const;
private:
    Configer *q;
    mutable QReadWriteLock m_rwLock;
    UserPreferencePointer m_root;
    QString m_configPath;
    QFileSystemWatcher *m_watcher = nullptr;
    QTimer m_delayLoad;
};

}

#endif // CONFIGER_P_H
