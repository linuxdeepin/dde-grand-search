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
#ifndef SEARCHERGROUPPRIVATE_H
#define SEARCHERGROUPPRIVATE_H

#include "searchergroup.h"
#include "searchplugin/pluginmanager.h"
#include "app/desktopappsearcher.h"
#include "extend/extendsearcher.h"
#include "setting/controlcentersearcher.h"
#include "web/statictextechoer.h"

#ifdef ENABLE_DEEPINANYTHING
    #include "file/filenamesearcher.h"
#endif

#ifdef ENABLE_FSEARCH
    #include "file/fssearcher.h"
#endif

class SearcherGroupPrivate : public QObject
{
    Q_OBJECT
    friend class SearcherGroup;
public:
    explicit SearcherGroupPrivate(SearcherGroup *parent);

    void initBuiltin();
    bool initPluinManager();
    void initExtendSearcher();
    bool addExtendSearcher(const GrandSearch::SearchPluginInfo &pluginInfo);
private slots:
    void onActivatePlugin(const QString &name, bool &ret);
private:
    SearcherGroup *q;
    QList<Searcher *> m_builtin;
    QList<Searcher *> m_extend;
    PluginManager *m_pluginManager = nullptr;
};

#endif // SEARCHERGROUPPRIVATE_H
