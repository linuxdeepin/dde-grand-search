// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QUERYPARSER_H
#define QUERYPARSER_H

#include <QString>

// 解析全局搜索关键词，判定查询类型并抽取参数。
// 设计上各类 provider 业务概念独立，这里只做「识别 + 抽取」，
// 不持有换算逻辑（换算在各 provider 内）。
namespace QueryParser {

enum class Type {
    None, Currency, Unit, Time,
    Calc,      // 科学/程序员计算器（表达式、进制转换）
    Program,   // 程序员工具（ASCII/Base64/URL/哈希/时间戳）
    DateTime,  // 日期差、倒数日、时间戳
    Color,     // 颜色转换（RGB/HEX/HSL）
    Energy,    // 热量转换（千卡/千焦/卡/焦耳）
    Help       // 使用说明（输入 help/?/使用说明/帮助 查看功能示例）
};

struct Parsed {
    Type type = Type::None;
    QString raw;        // 原始输入
    // 通用数值（货币金额 / 单位数值 / 温度值 / 计算器数值）
    double value = 0.0;
    // 货币：from/to 币种代码（如 USD/CNY）；单位：from/to 单位键
    QString from;
    QString to;
    // 时间：城市/时区关键词；日期：日期关键词
    QString city;
    // 通用自由文本（计算器表达式 / 程序员工具输入 / 颜色串 / 日期串）
    QString text;
};

// 判定并解析输入；无法识别返回 type=Null
Parsed parse(const QString &input);

} // namespace QueryParser

#endif // QUERYPARSER_H
