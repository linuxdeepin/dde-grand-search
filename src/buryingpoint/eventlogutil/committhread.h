// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMITTHREAD_H
#define COMMITTHREAD_H

#include <QObject>

namespace GrandSearch {

namespace burying_point {

class CommitLog : public QObject
{
    Q_OBJECT
public:
    using InitEventLog = bool (*)(const std::string &, bool);
    using WriteEventLog = void (*)(const std::string &);

    explicit CommitLog(QObject *parent = nullptr);
    ~CommitLog();

public Q_SLOTS:
    void commit(const QString &data);
    bool init();

private:
    InitEventLog m_initEventLog = nullptr;
    WriteEventLog m_writeEventLog = nullptr;
};

}
}

#endif   // COMMITTHREAD_H
