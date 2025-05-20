// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginprocess.h"
#include "utils/specialtools.h"

#include <QProcess>
#include <QTimerEvent>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

using namespace GrandSearch;

#define MAXRESTARTCOUNT 3

PluginProcess::PluginProcess(QObject *parent)
    : QObject(parent)
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

    // 解析出可执行程序与参数
    QString exec;
    QStringList args;
    {
        if (!SpecialTools::splitCommand(path, exec, args)) {
            qCWarning(logDaemon) << "Invalid command format:" << path;
            return false;
        }
    }

    if (m_programs.contains(name)) {
        qCWarning(logDaemon) << "Program already exists:" << name;
        return false;
    }

    qCDebug(logDaemon) << "Adding program - Name:" << name << "Path:" << path;
    m_programs.insert(name, path);

    // 创建进程
    QProcess *process = new QProcess;
    process->setProgram(exec);
    process->setArguments(args);
    m_processes.insert(name, process);

    // 进程状态改变,必须使用队列，否则在槽函数中继续启动进程会导致无法发出进程状态的信号
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
    qCDebug(logDaemon) << "Clearing all programs and processes";
    m_programs.clear();
    m_watch.clear();
    m_restartCount.clear();
    auto processes = std::move(m_processes);

    for (QProcess *process : processes.values()) {
        process->terminate();
        delete process;
    }

    auto checklist = std::move(m_checklist);
    for (int id : checklist.values()) {
        killTimer(id);
    }
}

void PluginProcess::timerEvent(QTimerEvent *event)
{
    int id = event->timerId();
    if (auto process = m_checklist.key(id, nullptr)) {
        qCDebug(logDaemon) << "Checking process stability - Timer ID:" << id
                           << "Program:" << process->program();
        // 检查
        checkStability(process);

        // 只检查一次，移除timer
        removeChecklist(process);
        return;
    }
    QObject::timerEvent(event);
}

bool PluginProcess::startProgram(const QString &name)
{
    if (!m_processes.contains(name)) {
        qCWarning(logDaemon) << "Program not found:" << name;
        return false;
    }

    auto process = m_processes.value(name);
    Q_ASSERT(process);

    QMap<QProcess *, int>::iterator iter = m_restartCount.find(process);
    if (iter != m_restartCount.end()) {
        iter.value() += 1;
        if (iter.value() > MAXRESTARTCOUNT) {
            if (iter.value() == (MAXRESTARTCOUNT + 1))
                qCWarning(logDaemon) << "Rejecting program start - Name:" << name
                                     << "Program:" << process->program()
                                     << "Too many failures:" << iter.value();
            return false;
        }
    } else {
        m_restartCount.insert(process, 1);
    }

    process->terminate();
    process->start();
    qCInfo(logDaemon) << "Starting program - Name:" << name
                      << "Program:" << process->program()
                      << "Arguments:" << process->arguments();
    return true;
}

void PluginProcess::terminate(const QString &name)
{
    auto iter = m_processes.find(name);
    if (iter != m_processes.end()) {
        QProcess *process = iter.value();
        Q_ASSERT(process);
        if (process->state() == QProcess::NotRunning) {
            qCDebug(logDaemon) << "Process already terminated:" << name;
            return;
        }

        // 取消状态监控，防止被重启
        disconnect(process, nullptr, this, nullptr);

        qCInfo(logDaemon) << "Terminating process:" << name;
        process->terminate();

        // 等待1s退出
        if (!process->waitForFinished(1000)) {
            // 没有退出，直接kill
            process->kill();
            qCWarning(logDaemon) << "Process killed - Name:" << name;
        }
        qCInfo(logDaemon) << "Process terminated successfully:" << name;

        // 清理启动状态
        removeChecklist(process);
        m_restartCount.remove(process);

        // 再次连接，进程状态改变,必须使用队列，否则在槽函数中继续启动进程会导致无法发出进程状态的信号
        connect(process, &QProcess::stateChanged, this, &PluginProcess::processStateChanged, Qt::QueuedConnection);
    }
}

void PluginProcess::processStateChanged()
{
    QProcess *process = qobject_cast<QProcess *>(sender());
    if (!process)
        return;

    auto state = process->state();
    qCInfo(logDaemon) << "Process state changed - Name:" << m_processes.key(process)
                      << "State:" << state
                      << "PID:" << process->processId();

    if (state == QProcess::Running) {
        // 校验启动成功的时间
        addChecklist(process);
        return;
    }

    if (state == QProcess::NotRunning) {
        auto name = m_processes.key(process);
        // 守护程序，自动启动
        if (m_watch.contains(name)) {
            qCDebug(logDaemon) << "Auto-restarting watched process:" << name;
            startProgram(name);
        }
    }
}

void PluginProcess::removeChecklist(QProcess *process)
{
    auto iter = m_checklist.find(process);
    if (iter != m_checklist.end()) {
        // 停止定时器
        if (iter.value() > 0)
            killTimer(iter.value());

        m_checklist.remove(process);
    }
}

void PluginProcess::addChecklist(QProcess *process)
{
    Q_ASSERT(process);

    // 先移除旧的
    removeChecklist(process);

    // 1min后检查进程是否已稳定运行
    int id = startTimer(60 * 1000);
    if (id > 0) {
        m_checklist.insert(process, id);
        qCDebug(logDaemon) << "Added stability check - Timer ID:" << id
                           << "Process:" << m_processes.key(process);
    } else {
        qCCritical(logDaemon) << "Failed to start stability check timer - ID:" << id
                              << "Process:" << m_processes.key(process);
    }
}

void PluginProcess::checkStability(QProcess *process)
{
    Q_ASSERT(process);
    if (process->state() == QProcess::Running) {
        qCInfo(logDaemon) << "Process is stable - Name:" << m_processes.key(process)
                          << "PID:" << process->processId()
                          << "Restart count:" << m_restartCount.value(process);
        m_restartCount.remove(process);
    } else {
        qCWarning(logDaemon) << "Process is unstable - Name:" << m_processes.key(process)
                             << "Program:" << process->program()
                             << "Restart count:" << m_restartCount.value(process);
    }
}
