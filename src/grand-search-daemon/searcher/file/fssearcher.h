/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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

extern "C" {
#include "fsearch.h"
}

#include <QFuture>
#include <QTime>

class FsSearcher : public Searcher
{
    Q_OBJECT
public:
    explicit FsSearcher(QObject *parent = nullptr);
    ~FsSearcher() Q_DECL_OVERRIDE;
    QString name() const Q_DECL_OVERRIDE;
    bool isActive() const Q_DECL_OVERRIDE;
    bool activate() Q_DECL_OVERRIDE;
    ProxyWorker *createWorker() const Q_DECL_OVERRIDE;
    bool action(const QString &action, const QString &item) Q_DECL_OVERRIDE;
    void asyncInitDataBase();
private:
    static void loadDataBase(FsSearcher *fs);
    static void updateDataBase(FsSearcher *fs);
    void saveDataBase(Database *db);
private:
    bool m_isInited = false;
    bool m_isLoading = false;
    QFuture<void> m_loadFuture;

    FsearchApplication *m_app = nullptr;
    mutable Database *m_databaseForUpdate = nullptr;
    mutable QTime m_updateTime;
    mutable bool m_isUpdating = false;
    mutable QFuture<void> m_updateFuture;

    QTime m_databaseSaveTime;   // 索引保存计时
    int m_lastSaveTime = 0;
};

#endif // FSSEARCHER_H
