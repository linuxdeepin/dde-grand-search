// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CALCPROVIDER_H
#define CALCPROVIDER_H

#include <QString>
#include <QList>

#include "resultbuilder.h"

// 科学/程序员计算器：安全表达式求值 + 进制转换。
// 不依赖系统计算器，内置受限递归下降求值器，避免 eval 风险。
class CalcProvider {
public:
    // 返回表达式计算结果卡片；含进制转换请求时同时给出 hex/bin/oct/dec。
    static QList<ResultBuilder::Item> eval(const QString &expr);

private:
    // 受限表达式求值（支持 + - * / % ^ & | ~ 以及括号、进制字面量、基础函数）。
    // 成功返回 true 并写入 out；失败返回 false。
    static bool compute(const QString &expr, double &out);
};

#endif // CALCPROVIDER_H
