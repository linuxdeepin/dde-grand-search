// SPDX-FileCopyrightText: 2026 testerxydw
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "providers/calcprovider.h"
#include "resultbuilder.h"

#include <gtest/gtest.h>

#include <cmath>

namespace {

bool hasKey(const QList<ResultBuilder::Item> &items, const QString &key)
{
    for (const ResultBuilder::Item &it : items) {
        if (it.key == key)
            return true;
    }
    return false;
}

bool evalOk(const QString &expr, double expect, double tol = 1e-9)
{
    double out = 0.0;
    if (!CalcProvider::compute(expr, out))
        return false;
    return std::fabs(out - expect) <= tol;
}

} // namespace

// ── 求值器：运算符优先级与结合性 ──────────────────────────────────────
TEST(CalcCompute, PrecedenceAndParentheses)
{
    EXPECT_TRUE(evalOk("2+3*4", 14));
    EXPECT_TRUE(evalOk("(2+3)*4", 20));
    EXPECT_TRUE(evalOk("10-2*3", 4));
    EXPECT_TRUE(evalOk("10/4", 2.5));
}

TEST(CalcCompute, PowerIsRightAssociative)
{
    EXPECT_TRUE(evalOk("2^3^2", 512));
}

TEST(CalcCompute, ModAndBitwise)
{
    EXPECT_TRUE(evalOk("5%3", 2));
    EXPECT_TRUE(evalOk("0xff&0x0f", 15));
    EXPECT_TRUE(evalOk("12|5", 13));
    EXPECT_TRUE(evalOk("~0", -1));
}

TEST(CalcCompute, UnarySign)
{
    EXPECT_TRUE(evalOk("-5+8", 3));
    EXPECT_TRUE(evalOk("-(2+3)", -5));
}

// ── 求值器：进制字面量 ────────────────────────────────────────────────
TEST(CalcCompute, RadixLiterals)
{
    EXPECT_TRUE(evalOk("0xff", 255));
    EXPECT_TRUE(evalOk("0b1010", 10));
    EXPECT_TRUE(evalOk("0o17", 15));
    EXPECT_TRUE(evalOk("0xFF+1", 256));
}

// ── 求值器：函数与常量 ────────────────────────────────────────────────
TEST(CalcCompute, FunctionsAndConstants)
{
    EXPECT_TRUE(evalOk("sqrt(16)", 4));
    EXPECT_TRUE(evalOk("log(100)", 2));
    EXPECT_TRUE(evalOk("abs(-3)", 3));
    EXPECT_TRUE(evalOk("floor(2.7)", 2));
    EXPECT_TRUE(evalOk("round(2.5)", 3));
    EXPECT_TRUE(evalOk("pi*2", 2 * M_PI, 1e-9));
}

TEST(CalcCompute, FunctionComposition)
{
    EXPECT_TRUE(evalOk("sqrt(16)+2*8", 20));
    EXPECT_TRUE(evalOk("sqrt(sqrt(81))", 3));
}

// ── 求值器：畸形输入必须拒绝而非返回前缀结果 ─────────────────────────
// 回归：「12*8 abc」这类尾随 token 早期被静默丢弃并返回 96
TEST(CalcCompute, RejectsTrailingTokens)
{
    double out = 0.0;
    EXPECT_FALSE(CalcProvider::compute("12*8 abc", out));
    EXPECT_FALSE(CalcProvider::compute("2 +", out));
    EXPECT_FALSE(CalcProvider::compute("12++", out));
    EXPECT_FALSE(CalcProvider::compute("(2+3", out));
    EXPECT_FALSE(CalcProvider::compute("abc", out));
    EXPECT_FALSE(CalcProvider::compute("sqrt(", out));
    EXPECT_FALSE(CalcProvider::compute("1 @ 2", out));
    EXPECT_FALSE(CalcProvider::compute("", out));
}

TEST(CalcCompute, RejectsUnknownFunction)
{
    double out = 0.0;
    EXPECT_FALSE(CalcProvider::compute("foo(3)", out));
}

// ── 结果卡片：整数结果附进制行 ────────────────────────────────────────
TEST(CalcEval, IntegerResultAddsRadixItems)
{
    QList<ResultBuilder::Item> items = CalcProvider::eval("2+3*4");
    ASSERT_EQ(items.size(), 4);
    EXPECT_EQ(items.at(0).key, "calc-result");
    EXPECT_EQ(items.at(0).type, "convert/calc");
    EXPECT_TRUE(items.at(0).name.contains(QStringLiteral("14")));
    EXPECT_TRUE(hasKey(items, "calc-hex"));
    EXPECT_TRUE(hasKey(items, "calc-bin"));
    EXPECT_TRUE(hasKey(items, "calc-oct"));
}

TEST(CalcEval, FractionalResultAddsScienceAndPercent)
{
    QList<ResultBuilder::Item> items = CalcProvider::eval("10/4");
    ASSERT_EQ(items.size(), 3);
    EXPECT_EQ(items.at(0).key, "calc-result");
    EXPECT_TRUE(hasKey(items, "calc-sci"));
    EXPECT_TRUE(hasKey(items, "calc-pct"));
    EXPECT_FALSE(hasKey(items, "calc-hex"));
}

// ── 结果卡片：非有限值给错误卡 ────────────────────────────────────────
TEST(CalcEval, DivisionByZeroGivesErrorCard)
{
    QList<ResultBuilder::Item> items = CalcProvider::eval("1/0");
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(items.at(0).key, "calc-error");
    EXPECT_EQ(items.at(0).type, "convert/calc-error");
}

TEST(CalcEval, NegativeSqrtGivesErrorCard)
{
    QList<ResultBuilder::Item> items = CalcProvider::eval("sqrt(-1)");
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(items.at(0).key, "calc-error");
}

// 回归：超出 qint64 范围的整数结果不能强转（UB），按浮点展示
TEST(CalcEval, OverflowStaysFloatingPoint)
{
    QList<ResultBuilder::Item> items =
        CalcProvider::eval("99999999999999999999*99999999999999999999");
    ASSERT_FALSE(items.isEmpty());
    EXPECT_EQ(items.at(0).key, "calc-result");
    EXPECT_EQ(items.at(0).type, "convert/calc");
    EXPECT_FALSE(hasKey(items, "calc-hex"));
    EXPECT_TRUE(hasKey(items, "calc-sci"));
}

// ── 结果卡片：进制转换 ────────────────────────────────────────────────
TEST(CalcEval, ExplicitRadixConversion)
{
    QList<ResultBuilder::Item> items = CalcProvider::eval("255 to hex");
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(items.at(0).type, "convert/calc-base");
    EXPECT_TRUE(items.at(0).name.contains(QStringLiteral("0xff")));
}

TEST(CalcEval, RadixKeywordGivesAllBases)
{
    QList<ResultBuilder::Item> items = CalcProvider::eval("255 进制");
    ASSERT_EQ(items.size(), 4);
    EXPECT_EQ(items.at(0).key, "calc-base-dec");
    EXPECT_TRUE(items.at(0).name.contains(QStringLiteral("255")));
    EXPECT_EQ(items.at(1).key, "calc-base-hex");
    EXPECT_EQ(items.at(2).key, "calc-base-bin");
    EXPECT_EQ(items.at(3).key, "calc-base-oct");
}

TEST(CalcEval, ZeroConversionDoesNotReportFailure)
{
    QList<ResultBuilder::Item> items = CalcProvider::eval("0 to hex");
    ASSERT_EQ(items.size(), 1);
    EXPECT_TRUE(items.at(0).name.contains(QStringLiteral("0x0"))
                || items.at(0).name.contains(QStringLiteral("0")));
}

TEST(CalcEval, MalformedExpressionGivesHintNotSilence)
{
    QList<ResultBuilder::Item> items = CalcProvider::eval("12*8 abc");
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(items.at(0).key, "calc-error");
    EXPECT_FALSE(items.at(0).name.isEmpty());
}
