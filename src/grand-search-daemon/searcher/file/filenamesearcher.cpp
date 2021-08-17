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
#include "filenamesearcher.h"
#include "global/builtinsearch.h"
#include "filenameworker.h"

#include <QDBusInterface>
#include <QDBusReply>

FileNameSearcher::FileNameSearcher(QObject *parent) : Searcher(parent)
{
}

QString FileNameSearcher::name() const
{
    return GRANDSEARCH_CLASS_FILE_DEEPIN;
}

bool FileNameSearcher::isActive() const
{
    QDBusInterface interface("com.deepin.anything",
                             "/com/deepin/anything",
                             "com.deepin.anything",
                             QDBusConnection::systemBus());
    interface.setTimeout(500);
    if (!interface.isValid()) {
        qWarning() << QDBusConnection::systemBus().lastError().message();
        return false;
    }

    // fix bug 90645
    // 如果allPath返回为空，则调用refresh刷新一次
    static bool isAvailable = false;
    if (!isAvailable) {
        QDBusReply<QStringList> reply = interface.call("allPath");
        if (reply.isValid()) {
            auto paths = reply.value();
            if (paths.isEmpty()) {
                QDBusReply<QStringList> re = interface.call("refresh", QByteArray());
                if (re.isValid()) {
                    isAvailable = true;
                    qInfo() << "refresh result:" << re.value();
                } else {
                    qWarning() << "refresh method called failed," << re.error().message();
                }
            }
        } else {
            qWarning() << "allPath method called failed," << reply.error().message();
        }
    }

    return true;
}

bool FileNameSearcher::activate()
{
    return false;
}

ProxyWorker *FileNameSearcher::createWorker() const
{
    auto worker = new FileNameWorker(name());
    return worker;
}

bool FileNameSearcher::action(const QString &action, const QString &item)
{
    Q_UNUSED(item)
    qWarning() << "no such action:" << action << ".";
    return false;
}
