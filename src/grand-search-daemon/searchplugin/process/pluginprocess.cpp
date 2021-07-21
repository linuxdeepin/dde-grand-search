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
#include "pluginprocess.h"
#include "utils/utiltools.h"

#include <QProcess>

PluginProcess::PluginProcess(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QProcess::ProcessState>();
}

PluginProcess::~PluginProcess()
{
    clear();
}

bool PluginProcess::addProgram(const QString &name, const QString &path)
{
    Q_ASSERT(!name.isEmpty());
    Q_ASSERT(!path.isEmpty());

    //解析出可执行程序与参数
    QString exec;
    QStringList args;
    {
        if (!GrandSearch::UtilTools::splitCommand(path, exec, args)) {
            qWarning() << "error cmd:" << path;
            return false;
        }
    }

    if (m_programs.contains(name))
        return false;

    m_programs.insert(name, path);

    //创建进程
    QProcess *process = new QProcess;
    process->setProgram(exec);
    process->setArguments(args);
    m_processes.insert(name, process);

    //进程状态改变
    connect(process, &QProcess::stateChanged, this, &PluginProcess::processStateChanged, Qt::QueuedConnection);
    return true;
}

void PluginProcess::setWatched(const QString &name, bool watch)
{
    if (watch) {
        m_watch.insert(name, watch);
    } else {
        m_watch.remove(name);
    }
}

void PluginProcess::clear()
{
    m_programs.clear();
    m_watch.clear();
    auto processes = std::move(m_processes);

    for (QProcess *process : processes.values()) {
        process->terminate();
        delete process;
    }
}

void PluginProcess::processStateChanged()
{
    QProcess *process = qobject_cast<QProcess *>(sender());
    if (!process)
        return;

    auto state = process->state();
    qInfo() << "Process state:" << m_processes.key(process) << process->program()  << state << process->pid();
}
