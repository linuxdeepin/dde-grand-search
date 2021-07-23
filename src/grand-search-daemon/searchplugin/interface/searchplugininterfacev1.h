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

#ifndef SEARCHPLUGININTERFACEV1_H
#define SEARCHPLUGININTERFACEV1_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

class SearchPluginInterfaceV1: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    SearchPluginInterfaceV1(const QString &service, const QString &path, const char *interface,
                            const QDBusConnection &connection, QObject *parent = nullptr);

    ~SearchPluginInterfaceV1();

public slots:
    inline QDBusPendingReply<QString> Action(const QString &json)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(json);
        return asyncCallWithArgumentList(QStringLiteral("Action"), argumentList);
    }

    inline QDBusPendingReply<QString> Search(const QString &json)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(json);
        return asyncCallWithArgumentList(QStringLiteral("Search"), argumentList);
    }

    inline QDBusPendingReply<bool> Stop(const QString &json)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(json);
        return asyncCallWithArgumentList(QStringLiteral("Stop"), argumentList);
    }
};

#endif
