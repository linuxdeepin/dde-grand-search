// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#include "i18n.h"

#include <QHash>

namespace I18n {
namespace {

// 单位本地化说明：key 为内部归一化键，value 为 [中文, 英文]
struct UnitText { QString zh; QString en; };
const QHash<QString, UnitText> &unitTable()
{
    static const QHash<QString, UnitText> t = {
        // 长度
        {"m",    {"米", "meter"}},
        {"km",   {"千米/公里", "kilometer"}},
        {"cm",   {"厘米", "centimeter"}},
        {"mm",   {"毫米", "millimeter"}},
        {"um",   {"微米", "micrometer"}},
        {"nm",   {"纳米", "nanometer"}},
        {"mi",   {"英里", "mile"}},
        {"yd",   {"码", "yard"}},
        {"ft",   {"英尺", "foot"}},
        {"in",   {"英寸", "inch"}},
        {"nmi",  {"海里", "nautical mile"}},
        {"li",   {"里", "li (Chinese mile)"}},
        {"chi",  {"尺", "chi (Chinese foot)"}},
        {"cun",  {"寸", "cun (Chinese inch)"}},
        // 重量
        {"kg",   {"千克/公斤", "kilogram"}},
        {"g",    {"克", "gram"}},
        {"mg",   {"毫克", "milligram"}},
        {"t",    {"吨", "tonne"}},
        {"jin",  {"斤", "jin (0.5 kg)"}},
        {"liang",{"两", "liang (50 g)"}},
        {"lb",   {"磅", "pound"}},
        {"oz",   {"盎司", "ounce"}},
        {"ct",   {"克拉", "carat"}},
        // 面积
        {"sqm",  {"平方米", "square meter"}},
        {"sqkm", {"平方千米", "square kilometer"}},
        {"hectare", {"公顷", "hectare"}},
        {"mu",   {"亩", "mu (Chinese acre)"}},
        {"acre", {"英亩", "acre"}},
        {"sqft", {"平方英尺", "square foot"}},
        {"sqin", {"平方英寸", "square inch"}},
        {"sqmi", {"平方英里", "square mile"}},
        {"sqyd", {"平方码", "square yard"}},
        // 体积
        {"l",    {"升", "liter"}},
        {"ml",   {"毫升", "milliliter"}},
        {"m3",   {"立方米", "cubic meter"}},
        {"cm3",  {"立方厘米", "cubic centimeter"}},
        {"gal",  {"加仑", "gallon"}},
        {"qt",   {"夸脱", "quart"}},
        {"pt",   {"品脱", "pint"}},
        {"cup",  {"杯", "cup"}},
        {"floz", {"液量盎司", "fluid ounce"}},
        // 速度
        {"kmh",  {"千米/时", "km/h"}},
        {"ms",   {"米/秒", "m/s"}},
        {"miph", {"英里/时", "mph"}},
        {"kn",   {"节", "knot"}},
        {"mach", {"马赫", "mach"}},
        // 数据
        {"b",    {"字节", "byte"}},
        {"kb",   {"千字节", "kilobyte"}},
        {"mb",   {"兆字节", "megabyte"}},
        {"gb",   {"吉字节", "gigabyte"}},
        {"tb",   {"太字节", "terabyte"}},
        {"pb",   {"拍字节", "petabyte"}},
        {"bit",  {"比特", "bit"}},
        {"kbit", {"千比特", "kilobit"}},
        {"mbit", {"兆比特", "megabit"}},
        {"gbit", {"吉比特", "gigabit"}},
        // 温度
        {"c",    {"摄氏度", "Celsius"}},
        {"f",    {"华氏度", "Fahrenheit"}},
        {"k",    {"开尔文", "Kelvin"}},
        // 热量
        {"kcal", {"千卡/大卡", "kilocalorie"}},
        {"kj",   {"千焦", "kilojoule"}},
        {"cal",  {"卡", "calorie"}},
        {"j",    {"焦耳", "joule"}},
    };
    return t;
}

// 货币本地化中文/英文说明
const QHash<QString, UnitText> &currencyTable()
{
    static const QHash<QString, UnitText> t = {
        {"CNY", {"人民币", "Chinese Yuan"}},
        {"USD", {"美元", "US Dollar"}},
        {"EUR", {"欧元", "Euro"}},
        {"JPY", {"日元", "Japanese Yen"}},
        {"GBP", {"英镑", "British Pound"}},
        {"HKD", {"港币", "Hong Kong Dollar"}},
        {"KRW", {"韩元", "South Korean Won"}},
        {"RUB", {"卢布", "Russian Ruble"}},
        {"AUD", {"澳元", "Australian Dollar"}},
        {"CAD", {"加元", "Canadian Dollar"}},
        {"SGD", {"新加坡元", "Singapore Dollar"}},
        {"THB", {"泰铢", "Thai Baht"}},
        {"INR", {"印度卢比", "Indian Rupee"}},
        {"TWD", {"新台币", "New Taiwan Dollar"}},
        {"MOP", {"澳门元", "Macau Pataca"}},
    };
    return t;
}

QString wrap(bool zh, const QString &orig, const QString &name)
{
    if (zh)
        return QString("%1（%2）").arg(orig).arg(name);
    return QString("%1 (%2)").arg(orig).arg(name);
}

} // namespace

QString unitName(const QString &key)
{
    const QHash<QString, UnitText> &t = unitTable();
    auto it = t.find(key.toLower());
    if (it == t.end())
        return key;
    bool zh = isChinese();
    return wrap(zh, key, zh ? it->zh : it->en);
}

QString currencyName(const QString &code)
{
    const QHash<QString, UnitText> &t = currencyTable();
    auto it = t.find(code.toUpper());
    if (it == t.end())
        return code;
    bool zh = isChinese();
    return wrap(zh, code, zh ? it->zh : it->en);
}

// 进制 / 颜色等杂项标签本地化
const QHash<QString, UnitText> &miscTable()
{
    static const QHash<QString, UnitText> t = {
        {"HEX", {"十六进制", "hexadecimal"}},
        {"BIN", {"二进制", "binary"}},
        {"OCT", {"八进制", "octal"}},
        {"DEC", {"十进制", "decimal"}},
        {"RGB", {"红绿蓝", "RGB (red/green/blue)"}},
        {"HSL", {"色相/饱和度/亮度", "HSL (hue/saturation/lightness)"}},
    };
    return t;
}

QString miscName(const QString &key)
{
    const QHash<QString, UnitText> &t = miscTable();
    auto it = t.find(key.toUpper());
    if (it == t.end())
        return key;
    bool zh = isChinese();
    return wrap(zh, key, zh ? it->zh : it->en);
}

// 分组名本地化（key 为英文标识）
const QHash<QString, UnitText> &groupTable()
{
    static const QHash<QString, UnitText> t = {
        {"currency",   {"汇率", "Currency"}},
        {"unit",       {"单位", "Unit"}},
        {"time",       {"时间", "Time"}},
        {"calculator", {"计算器", "Calculator"}},
        {"developer",  {"程序员工具", "Developer"}},
        {"date",       {"日期", "Date"}},
        {"color",      {"颜色", "Color"}},
        {"energy",     {"热量", "Energy"}},
        {"help",       {"万能转换使用说明", "Help"}},
    };
    return t;
}

QString groupName(const QString &key)
{
    const QHash<QString, UnitText> &t = groupTable();
    auto it = t.find(key.toLower());
    if (it == t.end())
        return key;
    return isChinese() ? it->zh : it->en;
}

} // namespace I18n
