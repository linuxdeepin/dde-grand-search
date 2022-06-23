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
#ifndef DESKTOPAPPSEARCHER_P_H
#define DESKTOPAPPSEARCHER_P_H

#include "desktopappsearcher.h"
#include "global/matcheditem.h"

#include <DDesktopEntry>

#include <QFuture>
#include <QtConcurrent>

namespace GrandSearch {

typedef QSharedPointer<DTK_CORE_NAMESPACE::DDesktopEntry> DesktopEntryPointer;
typedef QSharedPointer<MatchedItem> DesktopAppPointer;

class DesktopAppSearcherPrivate
{
    friend class DesktopAppSearcher;
public:
    explicit DesktopAppSearcherPrivate(DesktopAppSearcher *parent);
    ~DesktopAppSearcherPrivate();
public:
    static QSet<QString> desktopIndex(const DesktopEntryPointer &app, const QString &locale);
    static QString desktopName(const DesktopEntryPointer &app, const QString &locale, bool generic);
    static QString splitLocaleName(const QString &locale);
private:
    static void createIndex(DesktopAppSearcherPrivate *);
    static void updateIndex(DesktopAppSearcherPrivate *);
    static QMap<QString, DesktopEntryPointer> scanDesktopFile(const QStringList &paths, volatile bool &runing);
    static bool isHidden(DesktopEntryPointer pointer);
private:
    DesktopAppSearcher *q;
    bool m_inited = false;
    volatile bool m_creating = false;
    QFuture<void> m_creatingIndex;
    QFuture<void> m_updatingIndex;
    QStringList m_appDirs;
    //索引表
    QReadWriteLock m_lock;
    QHash<QString, QList<DesktopAppPointer>> m_indexTable;

    QFileSystemWatcher *m_fileWatcher = nullptr;
    volatile bool m_needUpdateIndex = false;
};

}

#endif // DESKTOPAPPSEARCHER_P_H
