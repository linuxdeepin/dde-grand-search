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

#ifndef EXTENDSEARCHER_H
#define EXTENDSEARCHER_H

#include "searcher/searcher.h"

class ExtendSearcherPrivate;
class ExtendSearcher : public Searcher
{
    Q_OBJECT
    friend class ExtendSearcherPrivate;
public:
    explicit ExtendSearcher(const QString &name, QObject *parent = nullptr);
    void setService(const QString &service, const QString &address,
                    const QString &interface, const QString &ver);
    void setActivatable(bool);
    QString name() const;
    bool isActive() const;
    bool activate();
    ProxyWorker *createWorker() const;
    bool action(const QString &action, const QString &item);
signals:
    void activateRequest(const QString &name, bool &ret);
private:
    ExtendSearcherPrivate *d;
};

#endif // EXTENDSEARCHER_H
