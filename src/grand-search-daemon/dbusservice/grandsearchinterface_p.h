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

#ifndef GRANDSEARCHINTERFACE_P_H
#define GRANDSEARCHINTERFACE_P_H

#include "grandsearchinterface.h"
#include "maincontroller/maincontroller.h"

#include <QTimer>
#include <QDBusMessage>

class GrandSearchInterfacePrivate : public QObject
{
    Q_OBJECT
    friend class GrandSearchInterface;
public:
    explicit GrandSearchInterfacePrivate(GrandSearchInterface *parent);
    ~GrandSearchInterfacePrivate();

    inline bool vaildSession(const QString &session) {
        if (session.isEmpty())
            return false;
        return m_session == session;
    }
    bool isAccessable(const QDBusMessage &msg) const;
private slots:
    void terminate();
    void onMatched();
    void onSearchCompleted();
private:
    GrandSearchInterface *q;
    MainController *m_main = nullptr;
    QString m_session;
    QTimer m_deadline;
    QHash<QString, bool> m_permit;
};

#endif // GRANDSEARCHINTERFACE_P_H
