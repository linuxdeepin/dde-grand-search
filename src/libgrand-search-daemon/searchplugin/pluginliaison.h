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
#ifndef PLUGINLIAISON_H
#define PLUGINLIAISON_H

#include "global/matcheditem.h"

#include <QObject>

namespace GrandSearch {

class PluginLiaisonPrivate;
class PluginLiaison : public QObject
{
    Q_OBJECT
public:
    explicit PluginLiaison(QObject *parent = nullptr);
    bool init(const QString &service, const QString &address,
              const QString &interface, const QString &ver,
              const QString &pluginName);
    bool isVaild() const;
    bool search(const QString &task, const QString &context);
    bool stop(const QString &task);
    bool action(const QString &type, const QString &item);
signals:
    void searchFinished(const MatchedItemMap &ret);
    void ready();
private:
    PluginLiaisonPrivate *d;
};

}

#endif // PLUGINLIAISON_H
