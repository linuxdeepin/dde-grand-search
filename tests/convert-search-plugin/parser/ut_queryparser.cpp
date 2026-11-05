// SPDX-FileCopyrightText: 2026 testerxydw
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "queryparser.h"

#include <gtest/gtest.h>
#include <QStringList>

using QueryParser::Parsed;
using QueryParser::Type;

namespace {

Parsed parseOf(const QString &input)
{
    return QueryParser::parse(input);
}

} // namespace

// ── 使用说明 ──────────────────────────────────────────────────────────
TEST(QueryParserHelp, RecognizesHelpTokens)
{
    for (const QString &s : QStringList{"help", "?", "？", "使用说明", "帮助", "用法", "怎么用", "功能", "menu",
                             "HELP", "Help"}) {
        EXPECT_EQ(parseOf(s).type, Type::Help) << "input: " << qPrintable(s);
    }
}

// ── 货币 ──────────────────────────────────────────────────────────────
TEST(QueryParserCurrency, NumberThenCode)
{
    Parsed p = parseOf("100usd");
    EXPECT_EQ(p.type, Type::Currency);
    EXPECT_DOUBLE_EQ(p.value, 100.0);
    EXPECT_EQ(p.from, "USD");
}

TEST(QueryParserCurrency, NumberThenSymbol)
{
    Parsed p = parseOf("$50");
    EXPECT_EQ(p.type, Type::Currency);
    EXPECT_DOUBLE_EQ(p.value, 50.0);
    EXPECT_EQ(p.from, "USD");
}

// 回归：代码在前的写法（usd100）此前只被「数字+符号」分支覆盖
TEST(QueryParserCurrency, CodeThenNumber)
{
    Parsed p = parseOf("usd100");
    EXPECT_EQ(p.type, Type::Currency);
    EXPECT_DOUBLE_EQ(p.value, 100.0);
    EXPECT_EQ(p.from, "USD");
}

TEST(QueryParserCurrency, TargetCurrencyWithEqualsQuestion)
{
    Parsed p = parseOf("100美元=?人民币");
    EXPECT_EQ(p.type, Type::Currency);
    EXPECT_EQ(p.from, "USD");
    EXPECT_EQ(p.to, "CNY");
}

// 回归：静态汇率表支持但早期无别名的币种必须能解析出代码
TEST(QueryParserCurrency, SupportsStaticTableAliases)
{
    struct Case { const char *input; const char *from; };
    const Case cases[] = {
        {"100aud", "AUD"}, {"100cad", "CAD"}, {"100sgd", "SGD"}, {"100thb", "THB"},
        {"100inr", "INR"}, {"100rub", "RUB"}, {"100krw", "KRW"}, {"100twd", "TWD"},
        {"100mop", "MOP"}, {"100日元", "JPY"}, {"100港币", "HKD"}, {"100英镑", "GBP"},
    };
    for (const Case &c : cases) {
        Parsed p = parseOf(QString::fromUtf8(c.input));
        EXPECT_EQ(p.type, Type::Currency) << "input: " << c.input;
        EXPECT_EQ(p.from, QLatin1String(c.from)) << "input: " << c.input;
    }
}

// ── 温度 ──────────────────────────────────────────────────────────────
// 回归：负温度此前被从子串「5c」命中而丢掉符号
TEST(QueryParserTemperature, KeepsNegativeSign)
{
    Parsed p = parseOf("-5c");
    EXPECT_EQ(p.type, Type::Unit);
    EXPECT_DOUBLE_EQ(p.value, -5.0);
    EXPECT_EQ(p.from, "c");
}

TEST(QueryParserTemperature, PositiveAndTargetScale)
{
    Parsed p = parseOf("25c to f");
    EXPECT_EQ(p.type, Type::Unit);
    EXPECT_DOUBLE_EQ(p.value, 25.0);
    EXPECT_EQ(p.from, "c");
    EXPECT_EQ(p.to, "f");
}

// ── 热量 ──────────────────────────────────────────────────────────────
// kcal/kj 不能被温度的 k(?!cal|j) 或单位分支吞掉
TEST(QueryParserEnergy, kcalIsEnergyNotUnitOrTemperature)
{
    Parsed p = parseOf("100 kcal");
    EXPECT_EQ(p.type, Type::Energy);
    EXPECT_DOUBLE_EQ(p.value, 100.0);
    EXPECT_EQ(p.from, "kcal");
}

TEST(QueryParserEnergy, ChineseEnergyUnits)
{
    for (const QString &s : QStringList{"100千卡", "100大卡", "100千焦", "100卡", "100焦耳"}) {
        EXPECT_EQ(parseOf(s).type, Type::Energy) << "input: " << qPrintable(s);
    }
}

// ── 单位 ──────────────────────────────────────────────────────────────
// 回归：复合速度单位不能被截断成 km（长度误判）
TEST(QueryParserUnit, CompoundSpeedUnitKeepsSlash)
{
    Parsed p = parseOf("100km/h to mph");
    EXPECT_EQ(p.type, Type::Unit);
    EXPECT_DOUBLE_EQ(p.value, 100.0);
    EXPECT_EQ(p.from, "km/h");
    EXPECT_EQ(p.to, "mph");
}

TEST(QueryParserUnit, KphAliasAlsoKeptAsSpeed)
{
    Parsed p = parseOf("10kph");
    EXPECT_EQ(p.type, Type::Unit);
    EXPECT_EQ(p.from, "kph");
}

TEST(QueryParserUnit, TargetWithEqualsQuestion)
{
    Parsed p = parseOf("1kg=?斤");
    EXPECT_EQ(p.type, Type::Unit);
    EXPECT_DOUBLE_EQ(p.value, 1.0);
    EXPECT_EQ(p.from, "kg");
    EXPECT_EQ(p.to, "斤");
}

TEST(QueryParserUnit, TargetSeparatedBySpaceOnly)
{
    Parsed p = parseOf("1kg 斤");
    EXPECT_EQ(p.type, Type::Unit);
    EXPECT_EQ(p.from, "kg");
    EXPECT_EQ(p.to, "斤");
}

TEST(QueryParserUnit, LengthAreaVolumeData)
{
    struct Case { const char *input; const char *from; };
    const Case cases[] = {
        {"10inch", "inch"}, {"5m2", "m2"}, {"2gal", "gal"}, {"100mb", "mb"}, {"3英里", "英里"},
    };
    for (const Case &c : cases) {
        Parsed p = parseOf(QString::fromUtf8(c.input));
        EXPECT_EQ(p.type, Type::Unit) << "input: " << c.input;
        EXPECT_EQ(p.from, QString::fromUtf8(c.from)) << "input: " << c.input;
    }
}

// 回归：面积/体积的上标单位曾被单位字符类截断成 m / km（按长度误判），
// 且中文单位（平方米）可用而 ASCII 写法（m2）不可用
TEST(QueryParserUnit, KeepsSquaredAndCubicUnitSuffix)
{
    Parsed area = parseOf("5m2");
    EXPECT_EQ(area.type, Type::Unit);
    EXPECT_DOUBLE_EQ(area.value, 5.0);
    EXPECT_EQ(area.from, "m2");

    Parsed toArea = parseOf("5m2 to cm2");
    EXPECT_EQ(toArea.from, "m2");
    EXPECT_EQ(toArea.to, "cm2");

    Parsed vol = parseOf("2m3");
    EXPECT_EQ(vol.type, Type::Unit);
    EXPECT_EQ(vol.from, "m3");

    // 单位后的数字若不属于上标（2/3）不能被吞进单位：10斤5两 仍取「斤」
    Parsed mixed = parseOf("10斤5两");
    EXPECT_EQ(mixed.from, QStringLiteral("斤"));
}

// ── 时间 ──────────────────────────────────────────────────────────────
TEST(QueryParserTime, CityKeyword)
{
    Parsed p = parseOf("北京时间");
    EXPECT_EQ(p.type, Type::Time);
    EXPECT_TRUE(p.city.contains(QStringLiteral("北京")));
}

TEST(QueryParserTime, EnglishCityAndTimezone)
{
    EXPECT_EQ(parseOf("tokyo time").type, Type::Time);
    EXPECT_EQ(parseOf("timezone").type, Type::Time);
}

// ── 颜色 ──────────────────────────────────────────────────────────────
TEST(QueryParserColor, ColorLiterals)
{
    struct Case { const char *input; const char *text; };
    const Case cases[] = {
        {"#ff8800", "#ff8800"}, {"#f80", "#f80"},
        {"rgb(255,136,0)", "rgb(255,136,0)"},
        {"hsl(200,50%,60%)", "hsl(200,50%,60%)"},
    };
    for (const Case &c : cases) {
        Parsed p = parseOf(QString::fromUtf8(c.input));
        EXPECT_EQ(p.type, Type::Color) << "input: " << c.input;
        EXPECT_EQ(p.text, QLatin1String(c.text)) << "input: " << c.input;
    }
}

// 回归：单字色名此前不在调色板内，与 colorprovider 的色名表不对齐
TEST(QueryParserColor, SingleCharColorNames)
{
    for (const QString &s : QStringList{"红", "蓝", "绿", "黄", "紫", "黑", "白", "灰", "粉", "棕",
                             "红色", "蓝色", "red", "blue", "随机", "random"}) {
        EXPECT_EQ(parseOf(s).type, Type::Color) << "input: " << qPrintable(s);
    }
}

// ── 程序员工具 ────────────────────────────────────────────────────────
TEST(QueryParserProgram, Base64DoesNotHitCalcTrigger)
{
    // "base" 是计算器触发词，但 base64 是整词，必须归 Program
    Parsed p = parseOf("base64 encode hello");
    EXPECT_EQ(p.type, Type::Program);
    EXPECT_EQ(p.text, QStringLiteral("base64 encode hello"));
}

// 回归：触发词被剥离后 provider 会丢失动作词，故回退时保留完整原文
TEST(QueryParserProgram, BareHashKeepsActionWord)
{
    Parsed p = parseOf("hash hello");
    EXPECT_EQ(p.type, Type::Program);
    EXPECT_EQ(p.text, QStringLiteral("hash hello"));
}

// 回归：raw/text 保留原始大小写（ascii A 与 ascii a 结果不同）
TEST(QueryParserProgram, PreservesOriginalCase)
{
    Parsed p = parseOf("ascii A");
    EXPECT_EQ(p.type, Type::Program);
    EXPECT_EQ(p.text, QStringLiteral("ascii A"));

    Parsed q = parseOf("MD5 hello");
    EXPECT_EQ(q.type, Type::Program);
    EXPECT_EQ(q.text, QStringLiteral("MD5 hello"));
}

TEST(QueryParserProgram, SecondLevelTimestampIsProgram)
{
    Parsed p = parseOf("1760000000");
    EXPECT_EQ(p.type, Type::Program);
    EXPECT_DOUBLE_EQ(p.value, 0.0);
    EXPECT_EQ(p.text, QStringLiteral("1760000000"));
}

TEST(QueryParserProgram, MillisecondTimestampIsProgram)
{
    EXPECT_EQ(parseOf("1760000000000").type, Type::Program);
}

TEST(QueryParserProgram, UrlKeyword)
{
    EXPECT_EQ(parseOf("url encode https://example.com").type, Type::Program);
}

// ── 日期 ──────────────────────────────────────────────────────────────
TEST(QueryParserDateTime, CountdownAndDiff)
{
    EXPECT_EQ(parseOf("距2027-01-01还有几天").type, Type::DateTime);
    EXPECT_EQ(parseOf("2026-01-01 到 2026-12-31 相差几天").type, Type::DateTime);
    EXPECT_EQ(parseOf("2026年8月20日 星期几").type, Type::DateTime);
}

TEST(QueryParserDateTime, TimestampConversionTriggerWinsOverProgram)
{
    // 「转时间戳」比「时间戳」更长，前缀匹配取最长者 → 归 DateTime
    Parsed p = parseOf("转时间戳 2026-08-20 15:03:28");
    EXPECT_EQ(p.type, Type::DateTime);
}

// ── 计算器 ────────────────────────────────────────────────────────────
TEST(QueryParserCalc, ArithmeticWithTriggerPrefix)
{
    Parsed p = parseOf("计算:12*8+sqrt(16)");
    EXPECT_EQ(p.type, Type::Calc);
    EXPECT_EQ(p.text, QStringLiteral("12*8+sqrt(16)"));
}

TEST(QueryParserCalc, ChineseSingleCharTriggerAllowsNoSpace)
{
    // 中文触发词不受整词限制，「算12*8」连写可用
    Parsed p = parseOf("算12*8");
    EXPECT_EQ(p.type, Type::Calc);
    EXPECT_EQ(p.text, QStringLiteral("12*8"));
}

TEST(QueryParserCalc, RadixLiteralsAndConversion)
{
    EXPECT_EQ(parseOf("0xff").type, Type::Calc);
    EXPECT_EQ(parseOf("255 to hex").type, Type::Calc);
    EXPECT_EQ(parseOf("0b1010").type, Type::Calc);
}

// ── 触发词剥离与回退 ──────────────────────────────────────────────────
TEST(QueryParserTrigger, StripsDelimiterAndKeepsLongestMatch)
{
    // 「时间戳」同时是 Program 与 DateTime 的前缀，最长者优先，不能误判为「时间」
    EXPECT_EQ(parseOf("时间戳").type, Type::Program);
    EXPECT_EQ(parseOf("换算1kg").type, Type::Unit);
    EXPECT_EQ(parseOf("热量100kcal").type, Type::Energy);
    EXPECT_EQ(parseOf("颜色 随机").type, Type::Color);
}

// ── 无法识别 ──────────────────────────────────────────────────────────
TEST(QueryParserUnknown, ReturnsNone)
{
    for (const QString &s : QStringList{"hello", "你好世界", "", "   ", "dde grand search"}) {
        EXPECT_EQ(parseOf(s).type, Type::None) << "input: [" << qPrintable(s) << "]";
    }
}

TEST(QueryParserUnknown, KeepsTrimmedRawOnly)
{
    Parsed p = parseOf("   ");
    EXPECT_TRUE(p.raw.isEmpty());
    EXPECT_EQ(p.type, Type::None);
}
