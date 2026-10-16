// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROGRAMPROVIDER_H
#define PROGRAMPROVIDER_H

#include <QString>
#include <QList>

#include "resultbuilder.h"

// 程序员工具：Base64 / URL 编解码、ASCII 码、哈希、当前时间戳。
// 各功能业务概念独立，按输入关键词分流。
class ProgramProvider {
public:
    // 返回该查询的结果卡片；无匹配返回空列表。
    static QList<ResultBuilder::Item> run(const QString &text);
};

#endif // PROGRAMPROVIDER_H
