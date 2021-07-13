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
#include "grandsearchservice_p.h"
#include "grandsearchservice.h"
#include "gui/mainwindow.h"

#include <QDebug>

GrandSearchServicePrivate::GrandSearchServicePrivate(GrandSearchService *parent)
    : q_p(parent)
{

}

GrandSearchService::GrandSearchService(QObject *parent)
    : QObject(parent)
    , d_p(new GrandSearchServicePrivate(this))
{
    connect(MainWindow::instance(), &MainWindow::visibleChanged, this, &GrandSearchService::VisibleChanged);
}

GrandSearchService::~GrandSearchService()
{

}

bool GrandSearchService::IsVisible() const
{
    bool isvisble = MainWindow::instance()->isVisible();
    qDebug() << "get by dbus and isvisible:" << isvisble;
    return isvisble;
}

void GrandSearchService::SetVisible(const bool visible)
{
    if (MainWindow::instance()->isVisible() == visible)
        return;
    if (!visible) {
        qInfo() << "set visible is:" << visible <<",and exit it.";
        MainWindow::instance()->close();
    } else {
        MainWindow::instance()->setVisible(visible);
    }
}
