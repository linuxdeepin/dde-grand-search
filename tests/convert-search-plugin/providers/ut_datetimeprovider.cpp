// SPDX-FileCopyrightText: 2026 testerxydw
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "providers/datetimeprovider.h"
#include "resultbuilder.h"

#include <gtest/gtest.h>

#include <QDate>

namespace {

QString firstKeyOf(const QString &text)
{
    QList<ResultBuilder::Item> items = DateTimeProvider::run(text);
    return items.isEmpty() ? QString() : items.first().key;
}

} // namespace

// ── 星期几 ────────────────────────────────────────────────────────────
TEST(DateTimeWeekday, KnownDate)
{
    QList<ResultBuilder::Item> items = DateTimeProvider::run(QStringLiteral("2026-08-20 星期几"));
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(items.at(0).key, "date-weekday");
    EXPECT_EQ(items.at(0).type, "convert/date-weekday");
    EXPECT_TRUE(items.at(0).name.contains(QStringLiteral("2026-08-20")));
}

// 回归：非法日期此前被静默回退成「今天的星期」，给出误导结果
TEST(DateTimeWeekday, InvalidDateDoesNotFallBackToToday)
{
    QList<ResultBuilder::Item> items = DateTimeProvider::run(QStringLiteral("2026-02-30 星期几"));
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(items.at(0).key, "date-weekday-invalid");
    EXPECT_NE(items.at(0).key, "date-weekday");
}

TEST(DateTimeWeekday, LeapDayIsValid)
{
    EXPECT_EQ(firstKeyOf(QStringLiteral("2028-02-29 星期几")), "date-weekday");
}

TEST(DateTimeWeekday, BareKeywordUsesToday)
{
    QList<ResultBuilder::Item> items = DateTimeProvider::run(QStringLiteral("今天星期几"));
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(items.at(0).key, "date-weekday");
    EXPECT_TRUE(items.at(0).name.contains(QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"))));
}

// ── 日期偏移 ──────────────────────────────────────────────────────────
TEST(DateTimeOffset, TodayPlusDays)
{
    QList<ResultBuilder::Item> items = DateTimeProvider::run(QStringLiteral("今天加30天"));
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(items.at(0).key, "date-offset");
    EXPECT_TRUE(items.at(0).name.contains(QStringLiteral("30")));
}

TEST(DateTimeOffset, DateMinusDaysUsesHyphen)
{
    EXPECT_EQ(firstKeyOf(QStringLiteral("2026-08-20 -100天")), "date-offset");
}

// ── 日期时间转时间戳 ──────────────────────────────────────────────────
TEST(DateTimeToTimestamp, StandardAndChineseForms)
{
    QList<ResultBuilder::Item> std_ = DateTimeProvider::run(QStringLiteral("2026-08-20 15:03:28"));
    ASSERT_EQ(std_.size(), 1);
    EXPECT_EQ(std_.at(0).key, "date-to-ts");
    EXPECT_EQ(std_.at(0).type, "convert/date-to-timestamp");

    EXPECT_EQ(firstKeyOf(QStringLiteral("2026年08月20日 15:03:28")), "date-to-ts");
}

TEST(DateTimeToTimestamp, SecondsOptional)
{
    EXPECT_EQ(firstKeyOf(QStringLiteral("2026-08-20 15:03")), "date-to-ts");
}

// ── 日期差 / 倒数日 / 单日期 ─────────────────────────────────────────
TEST(DateTimeDiff, TwoDates)
{
    QList<ResultBuilder::Item> items =
        DateTimeProvider::run(QStringLiteral("2026-01-01 至 2026-12-31 相差几天"));
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(items.at(0).key, "date-diff");
    EXPECT_TRUE(items.at(0).name.contains(QStringLiteral("364"))) << qPrintable(items.at(0).name);
}

TEST(DateTimeCountdown, FutureDate)
{
    // 相对今天计算，只断言命中倒数分支且天数为正
    QList<ResultBuilder::Item> items =
        DateTimeProvider::run(QStringLiteral("距")
                              + QDate::currentDate().addDays(100).toString(QStringLiteral("yyyy-MM-dd"))
                              + QStringLiteral("还有几天"));
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(items.at(0).key, "date-countdown");
    EXPECT_TRUE(items.at(0).name.contains(QStringLiteral("还有 100 天"))) << qPrintable(items.at(0).name);
}

TEST(DateTimeCountdown, PastDateReportsElapsed)
{
    QList<ResultBuilder::Item> items =
        DateTimeProvider::run(QStringLiteral("距")
                              + QDate::currentDate().addDays(-30).toString(QStringLiteral("yyyy-MM-dd"))
                              + QStringLiteral("还有几天"));
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(items.at(0).key, "date-countdown");
    EXPECT_TRUE(items.at(0).name.contains(QStringLiteral("已过去 30 天"))) << qPrintable(items.at(0).name);
}

TEST(DateTimeInfo, SingleDateWithoutKeyword)
{
    EXPECT_EQ(firstKeyOf(QStringLiteral("2030-06-01")), "date-info");
}

TEST(DateTimeToday, KeywordOnly)
{
    EXPECT_EQ(firstKeyOf(QStringLiteral("今天")), "date-today");
    EXPECT_EQ(firstKeyOf(QStringLiteral("today")), "date-today");
}

// ── 分隔符兼容与不误判 ───────────────────────────────────────────────
TEST(DateTimeFormats, AlternativeSeparators)
{
    EXPECT_EQ(firstKeyOf(QStringLiteral("2026/08/20 星期几")), "date-weekday");
    EXPECT_EQ(firstKeyOf(QStringLiteral("2026.8.20 星期几")), "date-weekday");
}

TEST(DateTimeFormats, MonthDayDefaultsToCurrentYear)
{
    EXPECT_EQ(firstKeyOf(QStringLiteral("8月20日 星期几")), "date-weekday");
}

TEST(DateTimeFormats, UnrelatedTextReturnsNothing)
{
    EXPECT_TRUE(DateTimeProvider::run(QStringLiteral("你好")).isEmpty());
    EXPECT_TRUE(DateTimeProvider::run(QStringLiteral("")).isEmpty());
    EXPECT_TRUE(DateTimeProvider::run(QStringLiteral("dde grand search")).isEmpty());
}
