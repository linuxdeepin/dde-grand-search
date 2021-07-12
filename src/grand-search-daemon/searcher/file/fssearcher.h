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
#ifndef FSSEARCHER_H
#define FSSEARCHER_H

#include "searcher/searcher.h"

class FsSearcher : public Searcher
{
    Q_OBJECT
public:
    explicit FsSearcher(QObject *parent = nullptr);
    virtual QString name() const;
    virtual bool isActive() const;
    virtual bool activate();
    virtual ProxyWorker *createWorker() const;
    virtual void action(const QString &action, const QString &item);
};

#endif // FSSEARCHER_H
