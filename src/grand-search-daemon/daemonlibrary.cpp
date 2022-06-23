/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#include "daemonlibrary.h"

#include <QLibrary>
#include <QDebug>

using namespace GrandSearch;

DaemonLibrary::DaemonLibrary(const QString &fileName, QObject *parent)
    : QObject(parent)
    , m_fileName(fileName)
{

}

bool DaemonLibrary::load()
{
    if (m_lib)
        return true;
    m_lib = new QLibrary(m_fileName, this);
    if (!m_lib->load()) {
        qCritical() << "can not load" << m_fileName << m_lib->errorString();
        delete m_lib;
        m_lib = nullptr;
        return false;
    }

    verFunc = (VerDaemon)m_lib->resolve("grandSearchDaemonAppVersion");
    if (!verFunc) {
        qCritical() << "no such api grandSearchDaemonAppVersion in" << m_fileName;
        delete m_lib;
        m_lib = nullptr;
        return false;
    }

    startFunc = (StartDaemon)m_lib->resolve("startGrandSearchDaemon");
    if (!startFunc) {
        qCritical() << "no such api startGrandSearchDaemon in" << m_fileName;
        delete m_lib;
        m_lib = nullptr;
        return false;
    }

    stopFunc = (StopDaemon)m_lib->resolve("stopGrandSearchDaemon");
    if (!stopFunc) {
        qCritical() << "no such api stopGrandSearchDaemon in" << m_fileName;
        delete m_lib;
        m_lib = nullptr;
        return false;
    }

    return true;
}

void DaemonLibrary::unload()
{
    if (m_lib) {
        m_lib->unload();
        delete m_lib;
        m_lib = nullptr;
        startFunc = nullptr;
        stopFunc = nullptr;
    }
}

int DaemonLibrary::start(int argc, char *argv[])
{
    if (!startFunc) {
        qCritical() << "no such api startGrandSearchDaemon";
        return -1;
    }
    return startFunc(argc, argv);
}

int DaemonLibrary::stop()
{
    if (!stopFunc) {
        qCritical() << "no such api stopGrandSearchDaemon";
        return -1;
    }
    return stopFunc();
}

QString DaemonLibrary::version()
{
    if (!verFunc) {
        qCritical() << "no such api grandSearchDaemonAppVersion";
        return "";
    }
    return QString(verFunc());
}
