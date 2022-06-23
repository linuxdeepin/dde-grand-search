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
#ifndef PLUGINMANAGER_P_H
#define PLUGINMANAGER_P_H

#include "pluginmanager.h"
#include "loader/pluginloader.h"
#include "process/pluginprocess.h"

namespace GrandSearch {

class PluginManagerPrivate : public QObject
{
    Q_OBJECT
    friend class PluginManager;
public:
    explicit PluginManagerPrivate(PluginManager *parent);
    bool readConf();
    void prepareProcess();
signals:

public slots:
private:
    PluginManager *q;
    PluginLoader *m_loader = nullptr;
    PluginProcess *m_process = nullptr;
};

}

#endif // PLUGINMANAGER_P_H
