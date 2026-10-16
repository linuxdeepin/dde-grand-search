// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENERGYPROVIDER_H
#define ENERGYPROVIDER_H

#include <QString>
#include <QList>

#include "resultbuilder.h"

// 热量转换：千卡/千焦/卡/焦耳互转，并给出常见食物热量参照，助力减肥人群。
class EnergyProvider {
public:
    // 返回该查询（value + from + [to]）的结果卡片；无匹配返回空列表。
    static QList<ResultBuilder::Item> convert(double value, const QString &from,
                                              const QString &to);

private:
    // 统一换算到千卡的系数（千卡为基准）
    static double toKcalFactor(const QString &u, bool &ok);
};

#endif // ENERGYPROVIDER_H
