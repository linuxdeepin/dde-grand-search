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
#ifndef GRANDSEARCHSERVICE_H
#define GRANDSEARCHSERVICE_H

#include <QObject>
#include <QScopedPointer>

#define GrandSearchViewServiceName          "com.deepin.dde.GrandSearch"
#define GrandSearchViewServicePath          "/com/deepin/dde/GrandSearch"
#define GrandSearchViewServiceInterface     "com.deepin.dde.GrandSearch"

class GrandSearchServicePrivate;
class GrandSearchService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", GrandSearchViewServiceInterface)
public:
    explicit GrandSearchService(QObject *parent = nullptr);
    ~GrandSearchService();

    Q_SCRIPTABLE bool IsVisible() const;
    Q_SCRIPTABLE void SetVisible(const bool visible);

signals:
    Q_SCRIPTABLE void VisibleChanged(const bool vidible);

private:
    QScopedPointer<GrandSearchServicePrivate> d_p;
};

#endif // GRANDSEARCHSERVICE_H
