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
#ifndef HANDLEVISIBILITY_H
#define HANDLEVISIBILITY_H

#include <com_deepin_sessionmanager.h>

#include <DRegionMonitor>

#include <QObject>

namespace GrandSearch {

class MainWindow;

class HandleVisibility : public QObject
{
    Q_OBJECT
public:
    explicit HandleVisibility(MainWindow *mainWindow, QObject *parent);

public slots:
    void onApplicationStateChanged(const Qt::ApplicationState state);
    void onLockedChanged(const bool locked);
    void onCloseWindow();

    void registerRegion(const bool isRegister);
    void regionMousePress(const QPoint &p, const int flag);

private:
    void init();
private:
    MainWindow *m_mainWindow = nullptr;
    Dtk::Gui::DRegionMonitor *m_regionMonitor = nullptr;
    __SessionManager *m_sessionManagerInter = nullptr;
};

}

#endif // HANDLEVISIBILITY_H
