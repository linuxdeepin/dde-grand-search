// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINCONTROLLER_P_H
#define MAINCONTROLLER_P_H

#include "maincontroller/maincontroller.h"
#include "searcher/searchergroup.h"
#include "task/taskcommander.h"

class MainControllerPrivate : public QObject
{
    Q_OBJECT
    friend class MainController;
public:
    MainControllerPrivate(MainController *parent);
    ~MainControllerPrivate();
    void buildWorker(TaskCommander *task);
    QStringList checkSearcher(const QStringList &groupList, const QStringList &suffixList, const QStringList &keywordList);
    QString buildKeywordInJson(const QStringList &groupList, const QStringList &suffixList, const QStringList &keywordList);
private slots:
    void dormancy();
private:
    MainController *q;
    SearcherGroup *m_searchers = nullptr;
    TaskCommander *m_currentTask = nullptr;
    QTimer m_dormancy;
};


#endif // MAINCONTROLLER_P_H
