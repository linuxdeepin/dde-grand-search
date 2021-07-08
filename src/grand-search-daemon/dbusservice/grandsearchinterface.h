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

#ifndef GRANDSEARCHINTERFACE_H
#define GRANDSEARCHINTERFACE_H

#include <QObject>
#include <QMap>

//后端程序DBus服务
#define GrandSearchServiceName          "com.deepin.dde.daemon.GrandSearch"
#define GrandSearchServicePath          "/com/deepin/dde/daemon/GrandSearch"
#define GrandSearchServiceInterface     "com.deepin.dde.daemon.GrandSearch"

class GrandSearchInterfacePrivate;
class GrandSearchInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", GrandSearchServiceInterface)
    friend class GrandSearchInterfacePrivate;
public:
    explicit GrandSearchInterface(QObject *parent = nullptr);
    ~GrandSearchInterface();
    bool init();
public slots:
    Q_SCRIPTABLE bool Search(const QString &session, const QString &key);
    Q_SCRIPTABLE void Terminate();
    Q_SCRIPTABLE QString MatchingResults(const QString &session);
    Q_SCRIPTABLE QString MatchingBuffer(const QString &session);
    Q_SCRIPTABLE bool OpenWithPlugin(const QString &item);
    Q_SCRIPTABLE bool Configure(const QMap<QString,QString> &);
    Q_SCRIPTABLE QMap<QString,QString> Configuration() const;
    Q_SCRIPTABLE bool SetFeedBackStrategy(const QMap<QString,QString> &);
    Q_SCRIPTABLE QMap<QString,QString> FeedBackStrategy() const;
    Q_SCRIPTABLE bool KeepAlive(const QString &session);
signals:
    Q_SCRIPTABLE void Matched(const QString &session);
    Q_SCRIPTABLE void SearchCompleted(const QString &session);
private:
    GrandSearchInterfacePrivate *d;
};

#endif // GRANDSEARCHINTERFACE_H
