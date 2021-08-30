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

#include "controlcentersearcher.h"
#include "controlcentersearcherprivate.h"
#include "global/builtinsearch.h"
#include "utils/chineseletterhelper.h"
#include "controlcenterworker.h"

ControlCenterSearcherPrivate::ControlCenterSearcherPrivate(ControlCenterSearcher *parent)
    : q(parent)
{

}

void ControlCenterSearcherPrivate::createIndex(ControlCenterSearcherPrivate *self)
{
    //todo 解析
#ifdef QT_DEBUG
    //test
    qDebug() << "test createIndex...";
    SettingItemPointer info(new GrandSearch::MatchedItem());
    info->name = "显示设置";
    info->item = "display";
    info->icon = "application-x-desktop";//若没有图标，则使用默认"application-x-desktop"
    info->type = "application/x-dde-control-center"; //此项写固定的自定义类型,application/x-dde-control-center
    info->searcher = self->q->name();

    self->m_lock.lockForWrite();
    self->m_indexTable.insert(info->name,{info});
    self->m_lock.unlock();
#endif
    self->m_inited = true;
    self->m_creating = false;
}

ControlCenterSearcher::ControlCenterSearcher(QObject *parent)
    : Searcher(parent)
    , d(new ControlCenterSearcherPrivate(this))
{

}

ControlCenterSearcher::~ControlCenterSearcher()
{
    delete d;
    d = nullptr;
}

void ControlCenterSearcher::asyncInit()
{
    if (d->m_inited || d->m_creating)
        return;

    //开始遍历目录创建索引
    d->m_creating = true;
    d->m_creatingIndex = QtConcurrent::run(ControlCenterSearcherPrivate::createIndex, d);

    //todo 监视变化
}

QString ControlCenterSearcher::name() const
{
    return GRANDSEARCH_CLASS_SETTING_CONTROLCENTER;
}

bool ControlCenterSearcher::isActive() const
{
    return d->m_inited;
}

bool ControlCenterSearcher::activate()
{
    return false;;
}

ProxyWorker *ControlCenterSearcher::createWorker() const
{
    auto worker = new ControlCenterWorker(name());
    {
        QWriteLocker lk(&d->m_lock);
        worker->setIndexTable(d->m_indexTable);
    }

    return worker;
}

bool ControlCenterSearcher::action(const QString &action, const QString &item)
{
    Q_UNUSED(item);
    qWarning() << "no such action:" << action << ".";
    return false;
}
