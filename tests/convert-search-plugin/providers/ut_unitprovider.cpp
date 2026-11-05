// SPDX-FileCopyrightText: 2026 testerxydw
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "providers/unitprovider.h"

#include <gtest/gtest.h>

#include <cmath>

namespace {

double lenOf(const QString &u)
{
    bool ok = false;
    double v = UnitProvider::lengthFactor(u, ok);
    EXPECT_TRUE(ok) << "unknown length unit: " << qPrintable(u);
    return v;
}

double massOf(const QString &u)
{
    bool ok = false;
    double v = UnitProvider::massFactor(u, ok);
    EXPECT_TRUE(ok) << "unknown mass unit: " << qPrintable(u);
    return v;
}

double speedOf(const QString &u)
{
    bool ok = false;
    double v = UnitProvider::speedFactor(u, ok);
    EXPECT_TRUE(ok) << "unknown speed unit: " << qPrintable(u);
    return v;
}

double tempOf(double value, const QString &from, const QString &to)
{
    double out = 0.0;
    EXPECT_TRUE(UnitProvider::tempConvert(value, from, to, out));
    return out;
}

} // namespace

// ── 系数表 ────────────────────────────────────────────────────────────
TEST(UnitFactors, Length)
{
    EXPECT_DOUBLE_EQ(lenOf("m"), 1.0);
    EXPECT_DOUBLE_EQ(lenOf("km"), 1000.0);
    EXPECT_DOUBLE_EQ(lenOf("公里"), 1000.0);
    EXPECT_DOUBLE_EQ(lenOf("cm"), 0.01);
    EXPECT_DOUBLE_EQ(lenOf("inch"), 0.0254);
    EXPECT_DOUBLE_EQ(lenOf("mile"), 1609.344);
}

TEST(UnitFactors, Mass)
{
    EXPECT_DOUBLE_EQ(massOf("kg"), 1.0);
    EXPECT_DOUBLE_EQ(massOf("g"), 0.001);
    EXPECT_DOUBLE_EQ(massOf("斤"), 0.5);
    EXPECT_DOUBLE_EQ(massOf("两"), 0.05);
    EXPECT_DOUBLE_EQ(massOf("吨"), 1000.0);
    EXPECT_NEAR(massOf("lb"), 0.45359237, 1e-12);
}

// 回归：复合速度单位（km/h）与缩写（kmh/kph）都要命中同一基准，
// 早期 km/h 被长度单位截断，导致速度换算整体不可用
TEST(UnitFactors, SpeedAcceptsSlashAndAbbreviations)
{
    EXPECT_DOUBLE_EQ(speedOf("km/h"), 1.0);
    EXPECT_DOUBLE_EQ(speedOf("kmh"), 1.0);
    EXPECT_DOUBLE_EQ(speedOf("kph"), 1.0);
    EXPECT_DOUBLE_EQ(speedOf("千米每小时"), 1.0);
    EXPECT_NEAR(speedOf("mph"), 1.609344, 1e-9);
    EXPECT_NEAR(speedOf("节"), 1.852, 1e-9);
}

TEST(UnitFactors, NormalizesSpacesAndDots)
{
    bool okA = false;
    bool okB = false;
    const double a = UnitProvider::speedFactor("km / h", okA);
    const double b = UnitProvider::speedFactor("KM/H", okB);
    EXPECT_TRUE(okA);
    EXPECT_TRUE(okB);
    EXPECT_DOUBLE_EQ(a, b);
}

TEST(UnitFactors, UnknownUnitReportsFailure)
{
    bool ok = true;
    EXPECT_DOUBLE_EQ(UnitProvider::lengthFactor("xx", ok), 0.0);
    EXPECT_FALSE(ok);
    ok = true;
    EXPECT_DOUBLE_EQ(UnitProvider::speedFactor("km/hh", ok), 0.0);
    EXPECT_FALSE(ok);
}

// ── 温度：非线性换算 ──────────────────────────────────────────────────
TEST(UnitTemp, FreezingAndBoilingPoints)
{
    EXPECT_NEAR(tempOf(0, "c", "f"), 32.0, 1e-9);
    EXPECT_NEAR(tempOf(100, "c", "f"), 212.0, 1e-9);
    EXPECT_NEAR(tempOf(0, "c", "k"), 273.15, 1e-9);
}

// 回归：负温度必须保持符号
TEST(UnitTemp, KeepsNegativeCelsius)
{
    EXPECT_NEAR(tempOf(-40, "c", "f"), -40.0, 1e-6);
    EXPECT_NEAR(tempOf(-5, "c", "f"), 23.0, 1e-9);
    EXPECT_NEAR(tempOf(-5, "c", "k"), 268.15, 1e-9);
}

TEST(UnitTemp, RoundTrips)
{
    const double f = tempOf(36.6, "c", "f");
    EXPECT_NEAR(tempOf(f, "f", "c"), 36.6, 1e-9);
}

// ── 结果卡片 ──────────────────────────────────────────────────────────
TEST(UnitConvert, SpeedWithExplicitTarget)
{
    QList<ResultBuilder::Item> items = UnitProvider::convert(100, "km/h", "mph");
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(items.at(0).key, "unit-speed-100-mph");
    EXPECT_EQ(items.at(0).type, "convert/unit-speed");
    // 100 km/h = 62.1371 mph
    EXPECT_TRUE(items.at(0).name.contains(QStringLiteral("62.1371"))) << qPrintable(items.at(0).name);
}

TEST(UnitConvert, MassWithChineseTarget)
{
    QList<ResultBuilder::Item> items = UnitProvider::convert(1, "kg", "斤");
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(items.at(0).type, "convert/unit-mass");
    EXPECT_TRUE(items.at(0).name.contains(QStringLiteral("2"))) << qPrintable(items.at(0).name);
}

TEST(UnitConvert, OmittedTargetListsWholeCategory)
{
    QList<ResultBuilder::Item> items = UnitProvider::convert(1, "km", "");
    ASSERT_FALSE(items.isEmpty());
    for (const ResultBuilder::Item &it : items)
        EXPECT_EQ(it.type, "convert/unit-length");
    // 基准单位自身不重复出现
    for (const ResultBuilder::Item &it : items)
        EXPECT_NE(it.key, "unit-len-1-km");
}

TEST(UnitConvert, TemperatureOmitsSourceScale)
{
    QList<ResultBuilder::Item> items = UnitProvider::convert(100, "c", "");
    ASSERT_EQ(items.size(), 2); // f 与 k
    for (const ResultBuilder::Item &it : items)
        EXPECT_EQ(it.type, "convert/unit-temp");
}

TEST(UnitConvert, NegativeTemperatureKeepsSignInResult)
{
    QList<ResultBuilder::Item> items = UnitProvider::convert(-5, "c", "f");
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(items.at(0).key, "unit-temp--5-f");
    EXPECT_TRUE(items.at(0).name.contains(QStringLiteral("23"))) << qPrintable(items.at(0).name);
}

TEST(UnitConvert, AreaWithAsciiSquaredUnit)
{
    QList<ResultBuilder::Item> items = UnitProvider::convert(1, "m2", "cm2");
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(items.at(0).type, "convert/unit-area");
    EXPECT_EQ(items.at(0).key, "unit-area-1-cm2");
    // 1 m2 = 10000 cm2
    EXPECT_TRUE(items.at(0).name.contains(QStringLiteral("10000"))) << qPrintable(items.at(0).name);
}

TEST(UnitConvert, VolumeWithAsciiCubicUnit)
{
    QList<ResultBuilder::Item> items = UnitProvider::convert(1, "m3", "l");
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(items.at(0).type, "convert/unit-volume");
    // 1 m3 = 1000 l
    EXPECT_TRUE(items.at(0).name.contains(QStringLiteral("1000"))) << qPrintable(items.at(0).name);
}

TEST(UnitConvert, UnknownUnitReturnsNoItems)
{
    EXPECT_TRUE(UnitProvider::convert(1, "xx", "yy").isEmpty());
    EXPECT_TRUE(UnitProvider::convert(1, "", "").isEmpty());
}
