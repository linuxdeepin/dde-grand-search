// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#include "colorprovider.h"
#include "i18n.h"

#include <QRegularExpression>
#include <QHash>
#include <QRandomGenerator>

namespace {

static QString colorLabel(const QString &key)
{
    return I18n::miscName(key);
}


struct RGB { int r = 0, g = 0, b = 0; };
struct HSL { int h = 0, s = 0, l = 0; };

// 常见色名 → RGB（中/英文名，用于快捷输入）
struct NameColor { QString zh; QString en; RGB rgb; };
static QList<NameColor> namedColors()
{
    return {
        {"红色", "red", {255, 0, 0}}, {"橙色", "orange", {255, 165, 0}},
        {"黄色", "yellow", {255, 255, 0}}, {"绿色", "green", {0, 128, 0}},
        {"青色", "cyan", {0, 255, 255}}, {"蓝色", "blue", {0, 0, 255}},
        {"紫色", "purple", {128, 0, 128}}, {"黑色", "black", {0, 0, 0}},
        {"白色", "white", {255, 255, 255}}, {"灰色", "gray", {128, 128, 128}},
        {"粉色", "pink", {255, 192, 203}}, {"棕色", "brown", {165, 42, 42}},
    };
}

// 匹配中文/英文色名，命中返回对应 RGB；否则返回 false。
static bool matchNamedColor(const QString &s, RGB &out)
{
    QString lower = s.toLower();
    for (const NameColor &nc : namedColors()) {
        if (lower.contains(nc.zh) || lower.contains(nc.en)) {
            out = nc.rgb;
            return true;
        }
    }
    return false;
}


QString toHex(const RGB &c)
{
    return QString("#%1%2%3")
        .arg(c.r, 2, 16, QChar('0'))
        .arg(c.g, 2, 16, QChar('0'))
        .arg(c.b, 2, 16, QChar('0'));
}

QString toRgbText(const RGB &c)
{
    return QString("rgb(%1, %2, %3)").arg(c.r).arg(c.g).arg(c.b);
}

HSL rgbToHsl(const RGB &c)
{
    double r = c.r / 255.0, g = c.g / 255.0, b = c.b / 255.0;
    double max = qMax(r, qMax(g, b)), min = qMin(r, qMin(g, b));
    double h = 0, s = 0, l = (max + min) / 2.0;
    double d = max - min;
    if (d > 1e-9) {
        s = (l > 0.5) ? d / (2.0 - max - min) : d / (max + min);
        if (max == r) h = (g - b) / d + (g < b ? 6.0 : 0.0);
        else if (max == g) h = (b - r) / d + 2.0;
        else h = (r - g) / d + 4.0;
        h /= 6.0;
    }
    HSL out;
    out.h = qRound(h * 360.0);
    out.s = qRound(s * 100.0);
    out.l = qRound(l * 100.0);
    return out;
}

RGB hslToRgb(const HSL &c)
{
    double h = c.h / 360.0, s = c.s / 100.0, l = c.l / 100.0;
    double r, g, b;
    if (s <= 1e-9) {
        r = g = b = l;
    } else {
        auto hue2rgb = [](double p, double q, double t) -> double {
            if (t < 0) t += 1;
            if (t > 1) t -= 1;
            if (t < 1.0/6) return p + (q - p) * 6 * t;
            if (t < 1.0/2) return q;
            if (t < 2.0/3) return p + (q - p) * (2.0/3 - t) * 6;
            return p;
        };
        double q = (l < 0.5) ? l * (1 + s) : l + s - l * s;
        double p = 2 * l - q;
        r = hue2rgb(p, q, h + 1.0/3);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1.0/3);
    }
    RGB out;
    out.r = qRound(r * 255.0);
    out.g = qRound(g * 255.0);
    out.b = qRound(b * 255.0);
    return out;
}

// 互补色：色相 +180°（保持饱和/亮度）
RGB complement(const RGB &c)
{
    HSL hsl = rgbToHsl(c);
    hsl.h = (hsl.h + 180) % 360;
    return hslToRgb(hsl);
}

} // namespace

QList<ResultBuilder::Item> ColorProvider::convert(const QString &colorRaw)
{
    QList<ResultBuilder::Item> items;
    QString s = colorRaw.toLower().remove(' ');

    RGB rgb;
    bool parsed = false;

    // 随机色：颜色 随机 / color random
    if (s.contains("随机") || s.contains("random")) {
        rgb.r = QRandomGenerator::global()->bounded(256);
        rgb.g = QRandomGenerator::global()->bounded(256);
        rgb.b = QRandomGenerator::global()->bounded(256);
        parsed = true;
        s.clear(); // 跳过后续标准串匹配
    }
    // 色名快捷：红色 / blue 等 → 预设色板
    if (!parsed && matchNamedColor(s, rgb)) {
        parsed = true;
        s.clear();
    }

    // #rgb 或 #rrggbb
    QRegularExpression hexRe(R"(^#([0-9a-f]{3}|[0-9a-f]{6})$)");
    QRegularExpressionMatch hm = hexRe.match(s);
    if (hm.hasMatch()) {
        QString hex = hm.captured(1);
        if (hex.size() == 3) {
            rgb.r = hex.mid(0,1).toInt(nullptr,16) * 17;
            rgb.g = hex.mid(1,1).toInt(nullptr,16) * 17;
            rgb.b = hex.mid(2,1).toInt(nullptr,16) * 17;
        } else {
            rgb.r = hex.mid(0,2).toInt(nullptr,16);
            rgb.g = hex.mid(2,2).toInt(nullptr,16);
            rgb.b = hex.mid(4,2).toInt(nullptr,16);
        }
        parsed = true;
    }
    // rgb(r,g,b)
    QRegularExpression rgbRe(R"(^rgb\((\d{1,3}),(\d{1,3}),(\d{1,3})\)$)");
    QRegularExpressionMatch rm = rgbRe.match(s);
    if (rm.hasMatch()) {
        rgb.r = rm.captured(1).toInt();
        rgb.g = rm.captured(2).toInt();
        rgb.b = rm.captured(3).toInt();
        parsed = true;
    }
    // hsl(h,s%,l%)
    QRegularExpression hslRe(R"(^hsl\((\d{1,3}),(\d{1,3})%,(\d{1,3})%\)$)");
    QRegularExpressionMatch sm = hslRe.match(s);
    if (sm.hasMatch()) {
        HSL hsl;
        hsl.h = sm.captured(1).toInt();
        hsl.s = sm.captured(2).toInt();
        hsl.l = sm.captured(3).toInt();
        rgb = hslToRgb(hsl);
        parsed = true;
    }

    if (!parsed)
        return items;

    HSL hsl = rgbToHsl(rgb);

    auto add = [&](const QString &key, const QString &name, const QString &type) {
        ResultBuilder::Item it;
        it.key = key;
        it.name = name;
        it.icon = "preferences-color";
        it.type = type;
        items.append(it);
    };
    add("color-hex", colorLabel("HEX") + ": " + toHex(rgb), "convert/color-hex");
    add("color-rgb", colorLabel("RGB") + ": " + toRgbText(rgb), "convert/color-rgb");
    add("color-hsl", colorLabel("HSL") + QString(": hsl(%1, %2%, %3%)").arg(hsl.h).arg(hsl.s).arg(hsl.l),
        "convert/color-hsl");

    // 互补色参考（辅助配色）
    RGB comp = complement(rgb);
    HSL compHsl = rgbToHsl(comp);
    add("color-comp", QString("互补色: %1  /  %2  /  hsl(%3, %4%, %5%)")
        .arg(toHex(comp)).arg(toRgbText(comp)).arg(compHsl.h).arg(compHsl.s).arg(compHsl.l),
        "convert/color-comp");
    return items;
}
