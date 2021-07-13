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

#include "searchergroup.h"
#include "searchergroupprivate.h"

#include <QDebug>

SearcherGroupPrivate::SearcherGroupPrivate(SearcherGroup *parent)
    : QObject(parent)
    , q(parent)
{
}

SearcherGroup::SearcherGroup(QObject *parent)
    : QObject(parent)
    , d(new SearcherGroupPrivate(this))
{
}

bool SearcherGroup::init()
{
#ifdef ENABLE_DEEPINANYTHING
    qInfo() << "create FileNameSearcher";
    auto fileSearcher = new FileNameSearcher(this);
    d->m_builtin << fileSearcher;
#endif

#ifdef ENABLE_FSEARCH
    qInfo() << "create FsSearcher.";
    auto fSearcher = new FsSearcher(this);
    d->m_builtin << fSearcher;
#endif

    qInfo() << "create DesktopAppSearcher.";
    auto appSearcher = new DesktopAppSearcher(this);
    appSearcher->asyncInit();
    d->m_builtin << appSearcher;

    //todo 设置搜索，全文搜索
    return true;
}

bool SearcherGroup::addExtendSearcher(const QVariant &pluginInfo)
{

}

QList<Searcher *> SearcherGroup::searchers() const
{
    //todo 排序
    return d->m_builtin + d->m_extend;
}

Searcher *SearcherGroup::searcher(const QString &name) const
{

}
