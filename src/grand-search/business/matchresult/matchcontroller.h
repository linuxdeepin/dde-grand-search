// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MATCHCONTROLLER_H
#define MATCHCONTROLLER_H

#include <QObject>
#include <QScopedPointer>
#include "global/matcheditem.h"

class MatchControllerPrivate;
class MatchController : public QObject
{
    Q_OBJECT
public:
    explicit MatchController(QObject *parent = nullptr);
    ~MatchController();

    void onMissionChanged(const QString &missionId, const QString &missionContent);

signals:
    void matchedResult(const GrandSearch::MatchedItemMap &);
    void searchCompleted();

private:
    QScopedPointer<MatchControllerPrivate> d_p;
};

#endif // MATCHCONTROLLER_H
