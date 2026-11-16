// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UNITPROVIDER_H
#define UNITPROVIDER_H

#include <QString>
#include <QList>

#include "resultbuilder.h"

// 单位换算：所有单位以「基准单位」的系数表示，换算 = value * fromFactor / toFactor。
// 温度特殊处理（非线性）。
class UnitProvider {
public:
    // 返回该查询（value + from + [to]）的结果卡片；无匹配返回空列表。
    static QList<ResultBuilder::Item> convert(double value, const QString &from,
                                              const QString &to);

private:
    // 长度（基准：米）
    static double lengthFactor(const QString &u, bool &ok);
    // 重量（基准：千克）
    static double massFactor(const QString &u, bool &ok);
    // 面积（基准：平方米）
    static double areaFactor(const QString &u, bool &ok);
    // 体积（基准：升）
    static double volumeFactor(const QString &u, bool &ok);
    // 速度（基准：km/h）
    static double speedFactor(const QString &u, bool &ok);
    // 数据（基准：字节）
    static double dataFactor(const QString &u, bool &ok);

    // 温度换算返回目标标度下的数值；标度：c/f/k
    static bool tempConvert(double value, const QString &fromScale,
                            const QString &toScale, double &out);
};

#endif // UNITPROVIDER_H
