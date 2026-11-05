// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#include "unitprovider.h"
#include "resultbuilder.h"
#include "i18n.h"

#include <QRegularExpression>

namespace {
// 规范化单位键（去空格、转小写，中文保留）
QString norm(const QString &u)
{
    QString s = u.toLower();
    s.remove(' ');
    s.remove('.');
    return s;
}

// 输出格式：原单位（本地化名），便于用户理解缩写含义，跟随系统语言
static QString unitDisplay(const QString &u)
{
    return I18n::unitName(norm(u));
}
} // namespace

double UnitProvider::lengthFactor(const QString &u, bool &ok)
{
    ok = true;
    QString s = norm(u);
    if (s == "m" || s == "米") return 1.0;
    if (s == "km" || s == "千米" || s == "公里") return 1000.0;
    if (s == "cm" || s == "厘米") return 0.01;
    if (s == "mm" || s == "毫米") return 0.001;
    if (s == "inch" || s == "in" || s == "英寸") return 0.0254;
    if (s == "feet" || s == "ft" || s == "英尺") return 0.3048;
    if (s == "yard" || s == "yd" || s == "码") return 0.9144;
    if (s == "mile" || s == "英里") return 1609.344;
    ok = false; return 0.0;
}

double UnitProvider::massFactor(const QString &u, bool &ok)
{
    ok = true;
    QString s = norm(u);
    if (s == "kg" || s == "千克" || s == "公斤") return 1.0;
    if (s == "g" || s == "克") return 0.001;
    if (s == "mg" || s == "毫克") return 0.000001;
    if (s == "lb" || s == "lbs" || s == "磅") return 0.45359237;
    if (s == "oz" || s == "盎司") return 0.0283495231;
    if (s == "斤") return 0.5;          // 市斤 = 0.5 kg
    if (s == "两") return 0.05;         // 市两 = 50 g
    if (s == "吨" || s == "t") return 1000.0;
    ok = false; return 0.0;
}

double UnitProvider::areaFactor(const QString &u, bool &ok)
{
    ok = true;
    QString s = norm(u);
    if (s == "m2" || s == "平方米") return 1.0;
    if (s == "km2" || s == "平方千米") return 1e6;
    if (s == "cm2" || s == "平方厘米") return 0.0001;
    if (s == "公顷" || s == "ha") return 10000.0;
    if (s == "亩") return 666.6666667;
    if (s == "acre" || s == "英亩") return 4046.8564224;
    ok = false; return 0.0;
}

double UnitProvider::volumeFactor(const QString &u, bool &ok)
{
    ok = true;
    QString s = norm(u);
    if (s == "l" || s == "升") return 1.0;
    if (s == "ml" || s == "毫升") return 0.001;
    if (s == "gal" || s == "加仑") return 3.785411784;     // 美制加仑
    if (s == "m3" || s == "立方米") return 1000.0;
    ok = false; return 0.0;
}

double UnitProvider::speedFactor(const QString &u, bool &ok)
{
    ok = true;
    QString s = norm(u);
    if (s == "kmh" || s == "kph" || s == "km/h" || s == "千米每小时") return 1.0;
    if (s == "mph" || s == "英里每小时") return 1.609344;
    if (s == "mps" || s == "米每秒") return 3.6;
    if (s == "节" || s == "knot") return 1.852;
    ok = false; return 0.0;
}

double UnitProvider::dataFactor(const QString &u, bool &ok)
{
    ok = true;
    QString s = norm(u);
    if (s == "b" || s == "字节") return 1.0;
    if (s == "kb" || s == "千字节") return 1024.0;
    if (s == "mb" || s == "兆字节") return 1024.0 * 1024.0;
    if (s == "gb" || s == "吉字节") return 1024.0 * 1024.0 * 1024.0;
    if (s == "tb" || s == "太字节") return 1024.0 * 1024.0 * 1024.0 * 1024.0;
    ok = false; return 0.0;
}

bool UnitProvider::tempConvert(double value, const QString &fromScale,
                               const QString &toScale, double &out)
{
    QString f = norm(fromScale);
    QString t = norm(toScale);
    auto toK = [](double v, const QString &sc) -> double {
        if (sc == "c") return v + 273.15;
        if (sc == "f") return (v - 32.0) * 5.0 / 9.0 + 273.15;
        return v; // k
    };
    auto fromK = [](double k, const QString &sc) -> double {
        if (sc == "c") return k - 273.15;
        if (sc == "f") return (k - 273.15) * 9.0 / 5.0 + 32.0;
        return k;
    };
    double k = toK(value, f);
    out = fromK(k, t);
    return true;
}

static QString tempLabel(const QString &scale)
{
    QString s = norm(scale);
    if (s == "c") return QString("°C %1").arg(I18n::unitName("c"));
    if (s == "f") return QString("°F %1").arg(I18n::unitName("f"));
    if (s == "k") return QString("K %1").arg(I18n::unitName("k"));
    return scale;
}

QList<ResultBuilder::Item> UnitProvider::convert(double value, const QString &from,
                                                 const QString &to)
{
    QList<ResultBuilder::Item> items;
    QString f = norm(from);

    // 温度分支
    if (f == "c" || f == "f" || f == "k") {
        QStringList targets;
        if (!to.isEmpty() && (norm(to) == "c" || norm(to) == "f" || norm(to) == "k"))
            targets << norm(to);
        else
            targets << "c" << "f" << "k"; // 温度仅三标度，全列
        for (const QString &ts : targets) {
            if (ts == f) continue;
            double out = 0.0;
            if (!tempConvert(value, f, ts, out)) continue;
            ResultBuilder::Item it;
            it.key = QString("unit-temp-%1-%2").arg(value).arg(ts);
            it.name = QString("%1%2 = %3%4")
                          .arg(ResultBuilder::formatNumber(value, 1))
                          .arg(tempLabel(f))
                          .arg(ResultBuilder::formatNumber(out, 1))
                          .arg(tempLabel(ts));
            it.icon = "preferences-system";
            it.type = "convert/unit-temp";
            items.append(it);
        }
        return items;
    }

    // 每类别全部可转换单位（省略目标时全列，不截断）
    static const QHash<QString, QStringList> kCategoryTargets = {
        {"len",   {"m", "cm", "km", "inch", "feet", "yard", "mile", "li", "chi", "cun"}},
        {"mass",  {"kg", "g", "mg", "lb", "oz", "斤", "两", "吨", "t"}},
        {"area",  {"m2", "km2", "cm2", "公顷", "亩", "acre"}},
        {"vol",   {"l", "ml", "gal", "m3"}},
        {"speed", {"kmh", "mph", "mps", "节", "mach"}},
        {"data",  {"b", "kb", "mb", "gb", "tb", "bit", "kbit", "mbit", "gbit"}},
    };

    // 线性单位：确定类别并求系数
    auto tryCategory = [&](const QString &cat,
                           std::function<double(const QString &, bool &)> factorFn,
                           const QString &type) -> bool {
        bool okF = false;
        double ff = factorFn(f, okF);
        if (!okF) return false;
        QStringList targets;
        if (!to.isEmpty()) targets << to;
        else targets = kCategoryTargets.value(cat); // 省略目标 → 该类别全列
        for (const QString &tt : targets) {
            bool okT = false;
            double tf = factorFn(tt, okT);
            if (!okT) continue;
            if (norm(tt) == f) continue;
            double out = value * ff / tf;
            ResultBuilder::Item it;
            it.key = QString("unit-%1-%2-%3").arg(cat).arg(value).arg(norm(tt));
            it.name = QString("%1 %2 = %3 %4")
                          .arg(ResultBuilder::formatNumber(value, 2)).arg(unitDisplay(from))
                          .arg(ResultBuilder::formatNumber(out, 4)).arg(unitDisplay(tt));
            it.icon = "preferences-system";
            it.type = type;
            items.append(it);
        }
        return true;
    };

    if (tryCategory("len", lengthFactor, "convert/unit-length")) return items;
    if (tryCategory("mass", massFactor, "convert/unit-mass")) return items;
    if (tryCategory("area", areaFactor, "convert/unit-area")) return items;
    if (tryCategory("vol", volumeFactor, "convert/unit-volume")) return items;
    if (tryCategory("speed", speedFactor, "convert/unit-speed")) return items;
    if (tryCategory("data", dataFactor, "convert/unit-data")) return items;

    return items;
}
