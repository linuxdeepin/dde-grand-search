// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DATETIMEPROVIDER_H
#define DATETIMEPROVIDER_H

#include <QString>
#include <QList>

#include "resultbuilder.h"

// 日期/倒数日/日期差：办公党友好，支持「距元旦还有几天」「2025-10-01 到 2026-01-01」
class DateTimeProvider {
public:
    static QList<ResultBuilder::Item> run(const QString &text);
};

#endif // DATETIMEPROVIDER_H
