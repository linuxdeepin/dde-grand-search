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
