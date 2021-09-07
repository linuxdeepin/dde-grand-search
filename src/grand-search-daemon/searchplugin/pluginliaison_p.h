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
#ifndef PLUGINLIAISON_P_H
#define PLUGINLIAISON_P_H

#include "pluginliaison.h"
#include "interface/searchplugininterfacev1.h"

class PluginLiaisonPrivate : public QObject
{
    Q_OBJECT
    friend class PluginLiaison;
public:
    explicit PluginLiaisonPrivate(PluginLiaison *parent);
    ~PluginLiaisonPrivate();
    bool isVaild() const;
    static void parseResult(const QString &json, PluginLiaisonPrivate *d);
private slots:
    void onSearchReplied();
private:
    PluginLiaison *q;
    SearchPluginInterfaceV1 *m_inteface = nullptr;
    QString m_ver;
    QDBusPendingCallWatcher *m_replyWatcher = nullptr;
    QFuture<void> m_parseThread;
    QAtomicInteger<bool> m_searching = false;
    QString m_pluginName;
};

#endif // PLUGINLIAISON_P_H
