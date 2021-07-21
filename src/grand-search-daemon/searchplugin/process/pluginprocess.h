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
#ifndef PLUGINPROCESS_H
#define PLUGINPROCESS_H

#include <QMap>
#include <QObject>

class QProcess;
class PluginProcess : public QObject
{
    Q_OBJECT
public:
    explicit PluginProcess(QObject *parent = nullptr);
    ~PluginProcess();
    bool addProgram(const QString &name, const QString &path);
    void setWatched(const QString &name, bool watch);
    void clear();
protected:
    void timerEvent(QTimerEvent *event);
signals:

public slots:
    bool startProgram(const QString &name);
    void terminate(const QString &name);
private slots:
    void processStateChanged();
private:
    void removeChecklist(QProcess *process);
    void addChecklist(QProcess *process);
    void checkStability(QProcess *process);
private:
    QMap<QString, QString> m_programs;
    QMap<QString, QProcess *> m_processes;
    QMap<QString, bool> m_watch;
    QMap<QProcess*, int> m_restartCount;
    QMap<QProcess*, int> m_checklist;
};

#endif // PLUGINPROCESS_H
