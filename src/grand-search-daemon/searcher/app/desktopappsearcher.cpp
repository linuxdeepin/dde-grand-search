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

#include "desktopappsearcher.h"
#include "desktopappsearcherprivate.h"
#include "desktopappworker.h"
#include "global/builtinsearch.h"

#include <QFileInfo>
#include <QDebug>

DCORE_USE_NAMESPACE

#define  APPLICATION_DIR_PATH "/usr/share/applications"

DesktopAppSearcherPrivate::DesktopAppSearcherPrivate(DesktopAppSearcher *parent)
    : q(parent)
{

}

DesktopAppSearcherPrivate::~DesktopAppSearcherPrivate()
{
    if (m_creating) {
        m_creating = false;

        qDebug() << "wait finished.";
        m_creatingIndex.waitForFinished();
        qDebug() << "DesktopAppSearcher finished.";
    }
}

void DesktopAppSearcherPrivate::createIndex(DesktopAppSearcherPrivate *d)
{
    QTime time;
    time.start();

    QHash<QString, QList<DesktopAppPointer>> indexTable;

    //获取所有应用
    QMap<QString, DesktopEntryPointer> apps = DesktopAppSearcherPrivate::scanDesktopFile(APPLICATION_DIR_PATH, d->m_creating);
    if (!d->m_creating)
        return;

    const QString locale = QLocale::system().name().simplified();
    for (auto app = apps.begin(); app != apps.end(); ++app) {
        //强制中断
        if (!d->m_creating)
            return;

        DesktopEntryPointer value = app.value();

        //获取索引关键字
        auto keys = DesktopAppSearcherPrivate::desktopIndex(value, locale);
        if (Q_UNLIKELY(keys.isEmpty()))
            continue;

        DesktopAppPointer info(new GrandSearch::MatchedItem());
        info->name = value->ddeDisplayName();
        info->item = app.key();
        info->icon = value->stringValue("Icon");
        info->type = "application/x-desktop"; //此项写固定值，.desktop文件的mimetype为application/x-desktop
        info->searcher = d->q->name();

        //放入索引表
        for (const QString &key : keys) {
            auto iter = indexTable.find(key);
            if (iter != indexTable.end() ) {
                iter->append(info);
            } else {
                indexTable.insert(key, {info});
            }
        }
    }

    //初始化完成
    QWriteLocker lk(&d->m_lock);
    d->m_indexTable = indexTable;
    d->m_inited = true;
    d->m_creating = false;

    qInfo() << "create index completed, spend" << time.elapsed() << "cout" << indexTable.size();
    //todo 开启监视器监控变化
}

QMap<QString, DesktopEntryPointer> DesktopAppSearcherPrivate::scanDesktopFile(const QString &path, volatile bool &runing)
{
    QMap<QString, DesktopEntryPointer> entrys;
    QDir dir(path);
    if (!dir.isReadable())
        return entrys;

    QFileInfoList entryInfoList = dir.entryInfoList({"*.desktop"}, QDir::Files, QDir::Name);
    for (const QFileInfo &fileInfo : entryInfoList) {
        //中断
        if (!runing)
            break;

        const QString path = fileInfo.absoluteFilePath();
        DesktopEntryPointer pointer(new DDesktopEntry(path));
        //正常解析时pointer->status()返回的不是NoError
        //if (pointer->status() == DDesktopEntry::NoError)
        entrys.insert(path, pointer);
    }

    return entrys;
}

QStringList DesktopAppSearcherPrivate::desktopIndex(const DesktopEntryPointer &app, const QString &locale)
{
    QStringList idxs;
    if (app.isNull())
        return idxs;

    static const QString XDeepinVendor = "X-Deepin-Vendor";
    static const QString deepin = "deepin";

    //dde名称,X-Deepin-Vendor=deepin
    bool useGeneric = app->stringValue(XDeepinVendor) == deepin;

    //默认名称
    QString defaultName = DesktopAppSearcherPrivate::desktopName(app, "", useGeneric);
    if (!defaultName.isEmpty())
        idxs << defaultName;

    //当前语言的名字
    QString localName = DesktopAppSearcherPrivate::desktopName(app, locale, useGeneric);
    if (!localName.isEmpty() && defaultName != localName)
        idxs << localName;

    //简体中文
    static const QString zhCN = "zh_CN";
    QString zhCNName;
    if (locale == zhCN)
        zhCNName = localName;
    else
        zhCNName = DesktopAppSearcherPrivate::desktopName(app, zhCN, useGeneric);

    if (!zhCNName.isEmpty()) {
        if (!idxs.contains(zhCNName, Qt::CaseInsensitive))
            idxs << zhCNName;

        //拼音

        //拼音首字母
    }

    return idxs;
}

QString DesktopAppSearcherPrivate::desktopName(const DesktopEntryPointer &app, const QString &locale, bool generic)
{
    QString name;
    if (app.isNull())
        return name;

    static const QString keyName = "Name";
    static const QString keyGenericName = "GenericName";

    //本地语言为空时获取默认值
    if (locale.isEmpty()) {
        if (generic)
            name = app->stringValue(keyGenericName);

        if (name.isEmpty())
            name = app->stringValue(keyName);
        return name;
    }

    //去掉_的本地标识
    const QString localeSplited = DesktopAppSearcherPrivate::splitLocaleName(locale);

    //找本地名称，GenericName[]
    if (generic) {
        name = app->stringValue(QString("%1[%2]").arg(keyGenericName).arg(locale));

        //没有则找简写的
        if (name.isEmpty() && !localeSplited.isEmpty())
            name = app->stringValue(QString("%1[%2]").arg(keyGenericName).arg(localeSplited));
    }

    //继续找Name[]
    if (name.isEmpty()) {
        name = app->stringValue(QString("%1[%2]").arg(keyName).arg(locale));

        //没有则找简写的
        if (name.isEmpty() && !localeSplited.isEmpty())
            name = app->stringValue(QString("%1[%2]").arg(keyName).arg(localeSplited));
    }

    return name;
}

QString DesktopAppSearcherPrivate::splitLocaleName(const QString &locale)
{
    QString ret;

    //去掉_后的本地语言标识
    QStringList localeList = locale.split("_");
    if (localeList.size() == 2 && !localeList.first().isEmpty())
        ret = localeList.first();

    return ret;
}

DesktopAppSearcher::DesktopAppSearcher(QObject *parent)
    : Searcher(parent)
    , d(new DesktopAppSearcherPrivate(this))
{
}

void DesktopAppSearcher::asyncInit()
{
    if (d->m_inited || d->m_creating)
        return;

    //开始遍历目录创建索引
    d->m_creating = true;
    d->m_creatingIndex = QtConcurrent::run(DesktopAppSearcherPrivate::createIndex, d);
}

QString DesktopAppSearcher::name() const
{
    return GRANDSEARCH_CLASS_APP_DESKTOP;
}

bool DesktopAppSearcher::isActive() const
{
    return d->m_inited;
}

bool DesktopAppSearcher::activate()
{
    return false;
}

ProxyWorker *DesktopAppSearcher::createWorker() const
{
    auto worker = new DesktopAppWorker(name());
    {
        QWriteLocker lk(&d->m_lock);
        worker->setIndexTable(d->m_indexTable);
    }

    return worker;
}

void DesktopAppSearcher::action(const QString &action, const QString &item)
{
    Q_UNUSED(item);
    qWarning() << "no such action:" << action << ".";
}
