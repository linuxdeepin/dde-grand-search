/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#ifndef FILESTATISTICSTHREAD_H
#define FILESTATISTICSTHREAD_H

#include <QThread>

class QTimer;
class FileStatisticsThread : public QThread
{
    Q_OBJECT
public:
    enum State {
        StoppedState,
        RunningState
    };

    explicit FileStatisticsThread(QObject *parent = nullptr);
    ~FileStatisticsThread() Q_DECL_OVERRIDE;

public slots:
    void start(const QString &sourceFile);
    void stop();
signals:
    void dataNotify(qint64 size);
protected:
    void run() Q_DECL_OVERRIDE;
private:
    void setState(State state);
    bool stateCheck();
    void processFile(const QString &file, QQueue<QString> &dirQueue);
private:
    QTimer *m_notifyDataTimer = nullptr;
    QAtomicInteger<qint64> m_totalSize = 0;
    QString m_sourceFile;
    State m_state = StoppedState;
};

#endif // FILESTATISTICSTHREAD_H
