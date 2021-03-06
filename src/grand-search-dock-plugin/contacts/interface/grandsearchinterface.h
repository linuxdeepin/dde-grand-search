/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp com.deepin.dde.GrandSearch.xml -c GrandSearchInterface -p grandsearchinterface
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef GRANDSEARCHINTERFACE_H
#define GRANDSEARCHINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.deepin.dde.GrandSearch
 */
class GrandSearchInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "com.deepin.dde.GrandSearch"; }
    static inline const char *staticServiceName()
    { return "com.deepin.dde.GrandSearch"; }
    static inline const char *staticObjectPath()
    { return "/com/deepin/dde/GrandSearch"; }


public:
    GrandSearchInterface(QObject *parent = nullptr);

    ~GrandSearchInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<bool> IsVisible()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("IsVisible"), argumentList);
    }

    inline QDBusPendingReply<> SetVisible(bool visible)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(visible);
        return asyncCallWithArgumentList(QStringLiteral("SetVisible"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void VisibleChanged(bool vidible);
};

namespace com {
  namespace deepin {
    namespace dde {
      typedef ::GrandSearchInterface GrandSearch;
    }
  }
}
#endif
