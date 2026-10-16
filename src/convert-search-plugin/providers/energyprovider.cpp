// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#include "energyprovider.h"
#include "i18n.h"
#include "resultbuilder.h"

#include <QRegularExpression>

namespace {

QString norm(const QString &u)
{
    QString s = u.toLower();
    s.remove(' ');
    s.remove('.');
    return s;
}

// 常见食物热量参照（单位：千卡/100g 或 每份），用于直观理解「吃进去多少」。
struct FoodRef { QString nameEn; QString nameZh; double kcal; QString unit; };
const QList<FoodRef> &foodRefs()
{
    static const QList<FoodRef> list = {
        {"apple (1 medium)",        "苹果(1个中等)",    52,   "kcal"},
        {"white rice (1 bowl)",     "白米饭(1碗)",     116,  "kcal"},
        {"steamed bun (1)",         "馒头(1个)",       223,  "kcal"},
        {"instant noodle (1 pack)", "方便面(1包)",     470,  "kcal"},
        {"cola (1 can 330ml)",      "可乐(1罐330ml)",  139,  "kcal"},
        {"milk tea (1 cup)",        "奶茶(1杯)",       300,  "kcal"},
        {"fried chicken (1 pc)",    "炸鸡(1块)",       280,  "kcal"},
        {"egg (1 boiled)",          "鸡蛋(1个煮)",      70,  "kcal"},
        {"banana (1 medium)",       "香蕉(1根中等)",    89,  "kcal"},
        {"chocolate (100g)",        "巧克力(100g)",    546,  "kcal"},
    };
    return list;
}

QString foodName(const FoodRef &f)
{
    return I18n::isChinese() ? f.nameZh : f.nameEn;
}

} // namespace

double EnergyProvider::toKcalFactor(const QString &u, bool &ok)
{
    ok = true;
    QString s = norm(u);
    if (s == "kcal" || s == "大卡" || s == "千卡") return 1.0;
    if (s == "kj" || s == "千焦") return 1.0 / 4.184;
    if (s == "cal" || s == "卡") return 1.0 / 1000.0;
    if (s == "j" || s == "焦" || s == "焦耳") return 1.0 / 4184.0;
    ok = false; return 0.0;
}

QList<ResultBuilder::Item> EnergyProvider::convert(double value, const QString &from,
                                                   const QString &to)
{
    QList<ResultBuilder::Item> items;

    bool okF = false;
    double fKcal = toKcalFactor(from, okF);
    if (!okF) return items; // 不是热量单位

    double kcal = value * fKcal;

    // 1) 单位互转（kcal/kj/cal/j）
    QStringList targets;
    if (!to.isEmpty()) targets << to;
    else targets << "kcal" << "kj" << "cal" << "j";

    auto displayUnit = [](double v, const QString &u) -> QString {
        QString label = I18n::unitName(norm(u));
        return QString("%1 %2").arg(ResultBuilder::formatNumber(v, 2)).arg(label);
    };

    for (const QString &tt : targets) {
        bool okT = false;
        double tf = toKcalFactor(tt, okT);
        if (!okT) continue;
        QString nt = norm(tt);
        if (nt == norm(from)) continue;
        double out = kcal / tf;
        ResultBuilder::Item it;
        it.key = QString("energy-%1-%2").arg(kcal).arg(nt);
        it.name = QString("%1 = %2")
            .arg(displayUnit(value, from)).arg(displayUnit(out, tt));
        it.icon = "firewall-applet";
        it.type = "convert/energy-unit";
        items.append(it);
    }

    // 2) 常见食物参照：这顿热量 ≈ 多少份某种食物
    for (const FoodRef &f : foodRefs()) {
        double portions = kcal / f.kcal;
        if (portions <= 0) continue;
        ResultBuilder::Item it;
        it.key = QString("energy-food-%1-%2").arg(kcal).arg(foodName(f));
        it.name = QString("%1 ≈ %2 %3")
            .arg(displayUnit(value, from))
            .arg(ResultBuilder::formatNumber(portions, 1))
            .arg(foodName(f));
        it.icon = "firewall-applet";
        it.type = "convert/energy-food";
        items.append(it);
    }

    return items;
}
