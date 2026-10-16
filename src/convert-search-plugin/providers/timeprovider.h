// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIMEPROVIDER_H
#define TIMEPROVIDER_H

#include <QString>
#include <QList>

#include "resultbuilder.h"

// 时区时间查询：城市/关键词 → IANA 时区 → 当前时间 + 与本地时差。
class TimeProvider {
public:
    // 返回该城市查询的结果卡片；无匹配返回空列表。
    static QList<ResultBuilder::Item> query(const QString &cityKeyword);
};

#endif // TIMEPROVIDER_H
