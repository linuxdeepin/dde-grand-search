// SPDX-FileCopyrightText: 2026 testerxydw
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "providers/programprovider.h"
#include "resultbuilder.h"

#include <gtest/gtest.h>

namespace {

const ResultBuilder::Item &first(const QList<ResultBuilder::Item> &items)
{
    return items.first();
}

bool hasKey(const QList<ResultBuilder::Item> &items, const QString &key)
{
    for (const ResultBuilder::Item &it : items) {
        if (it.key == key)
            return true;
    }
    return false;
}

QString nameForKey(const QList<ResultBuilder::Item> &items, const QString &key)
{
    for (const ResultBuilder::Item &it : items) {
        if (it.key == key)
            return it.name;
    }
    return QString();
}

} // namespace

// ── Base64 ────────────────────────────────────────────────────────────
TEST(ProgramBase64, Encode)
{
    QList<ResultBuilder::Item> items = ProgramProvider::run(QStringLiteral("base64 encode hello"));
    ASSERT_EQ(items.size(), 2); // 结果 + 解码验证
    EXPECT_EQ(first(items).key, "prog-base64-encode");
    EXPECT_EQ(first(items).type, "convert/prog-base64");
    EXPECT_TRUE(first(items).name.contains(QStringLiteral("aGVsbG8="))) << qPrintable(first(items).name);
    EXPECT_TRUE(nameForKey(items, "prog-base64-verify").contains(QStringLiteral("hello")));
}

TEST(ProgramBase64, Decode)
{
    QList<ResultBuilder::Item> items = ProgramProvider::run(QStringLiteral("base64 decode aGVsbG8="));
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(first(items).key, "prog-base64-decode");
    EXPECT_TRUE(first(items).name.contains(QStringLiteral("hello"))) << qPrintable(first(items).name);
}

// 回归：中文载荷此前被按 Latin1 处理而损坏
TEST(ProgramBase64, HandlesNonAsciiPayload)
{
    QList<ResultBuilder::Item> items = ProgramProvider::run(QStringLiteral("base64 编码 你好"));
    ASSERT_EQ(items.size(), 2);
    EXPECT_TRUE(first(items).name.contains(QStringLiteral("5L2g5aW9"))) << qPrintable(first(items).name);
    EXPECT_TRUE(nameForKey(items, "prog-base64-verify").contains(QStringLiteral("你好")));

    QList<ResultBuilder::Item> back = ProgramProvider::run(QStringLiteral("base64 decode 5L2g5aW9"));
    ASSERT_FALSE(back.isEmpty());
    EXPECT_TRUE(back.first().name.contains(QStringLiteral("你好"))) << qPrintable(back.first().name);
}

// ── URL 编解码 ────────────────────────────────────────────────────────
TEST(ProgramUrl, EncodeKeepsUnreservedAndEscapesRest)
{
    QList<ResultBuilder::Item> items = ProgramProvider::run(QStringLiteral("url encode a-b_c.d~e"));
    ASSERT_FALSE(items.isEmpty());
    EXPECT_TRUE(first(items).name.contains(QStringLiteral("a-b_c.d~e"))) << qPrintable(first(items).name);
}

// 回归：非 Latin1 字符（中文）早期被 toLatin1() 截断损坏
TEST(ProgramUrl, EncodeKeepsUtf8Multibyte)
{
    QList<ResultBuilder::Item> items = ProgramProvider::run(QStringLiteral("url encode 中文"));
    ASSERT_EQ(items.size(), 2);
    EXPECT_TRUE(first(items).name.contains(QStringLiteral("%E4%B8%AD%E6%96%87")))
        << qPrintable(first(items).name);
    EXPECT_TRUE(nameForKey(items, "prog-url-verify").contains(QStringLiteral("中文")));
}

TEST(ProgramUrl, DecodeRestoresUtf8)
{
    QList<ResultBuilder::Item> items =
        ProgramProvider::run(QStringLiteral("url decode %E4%B8%AD%E6%96%87"));
    ASSERT_FALSE(items.isEmpty());
    EXPECT_EQ(first(items).key, "prog-url-decode");
    EXPECT_TRUE(first(items).name.contains(QStringLiteral("中文"))) << qPrintable(first(items).name);
}

TEST(ProgramUrl, DecodeKeepsLiteralPercentWhenNotAnEscape)
{
    QList<ResultBuilder::Item> items = ProgramProvider::run(QStringLiteral("url decode 100%xx"));
    ASSERT_FALSE(items.isEmpty());
    EXPECT_TRUE(first(items).name.contains(QStringLiteral("100%"))) << qPrintable(first(items).name);
}

// ── 哈希 ──────────────────────────────────────────────────────────────
TEST(ProgramHash, ExplicitAlgorithm)
{
    QList<ResultBuilder::Item> items = ProgramProvider::run(QStringLiteral("md5 hello"));
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(first(items).key, "prog-hash");
    EXPECT_TRUE(first(items).name.contains(QStringLiteral("5d41402abc4b2a76b9719d911017c592")))
        << qPrintable(first(items).name);
}

// 回归：大写命令词此前因大小写敏感而返回空结果
TEST(ProgramHash, AcceptsUppercaseCommand)
{
    QList<ResultBuilder::Item> items = ProgramProvider::run(QStringLiteral("MD5 hello"));
    ASSERT_EQ(items.size(), 1);
    EXPECT_TRUE(first(items).name.contains(QStringLiteral("5d41402abc4b2a76b9719d911017c592")))
        << qPrintable(first(items).name);

    QList<ResultBuilder::Item> sha = ProgramProvider::run(QStringLiteral("SHA256 abc"));
    ASSERT_EQ(sha.size(), 1);
    EXPECT_TRUE(sha.first().name.contains(
        QStringLiteral("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")))
        << qPrintable(sha.first().name);
}

TEST(ProgramHash, BareKeywordReturnsAllThree)
{
    QList<ResultBuilder::Item> items = ProgramProvider::run(QStringLiteral("hash hello"));
    ASSERT_EQ(items.size(), 3);
    EXPECT_TRUE(hasKey(items, "prog-hash-md5"));
    EXPECT_TRUE(hasKey(items, "prog-hash-sha1"));
    EXPECT_TRUE(hasKey(items, "prog-hash-sha256"));
}

TEST(ProgramHash, PayloadCaseIsSignificant)
{
    const QString lower = nameForKey(ProgramProvider::run(QStringLiteral("md5 hello")), "prog-hash");
    const QString upper = nameForKey(ProgramProvider::run(QStringLiteral("md5 Hello")), "prog-hash");
    EXPECT_NE(lower, upper);
}

// ── ASCII ─────────────────────────────────────────────────────────────
TEST(ProgramAscii, CharToCode)
{
    QList<ResultBuilder::Item> items = ProgramProvider::run(QStringLiteral("ascii A"));
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(first(items).key, "prog-ascii");
    EXPECT_TRUE(first(items).name.contains(QStringLiteral("65"))) << qPrintable(first(items).name);
    EXPECT_TRUE(first(items).name.contains(QStringLiteral("0x41"))) << qPrintable(first(items).name);
}

TEST(ProgramAscii, CodeToChar)
{
    QList<ResultBuilder::Item> items = ProgramProvider::run(QStringLiteral("ascii 97"));
    ASSERT_EQ(items.size(), 1);
    EXPECT_TRUE(first(items).name.contains(QStringLiteral("'a'"))) << qPrintable(first(items).name);
}

// 回归：非 ASCII 字符早期被 toLatin1() 折成假码值，现如实报 Unicode 码点
TEST(ProgramAscii, NonAsciiReportsCodepointNotFakeAscii)
{
    QList<ResultBuilder::Item> items = ProgramProvider::run(QStringLiteral("ascii 中"));
    ASSERT_EQ(items.size(), 1);
    EXPECT_TRUE(first(items).name.contains(QStringLiteral("U+4E2D"))) << qPrintable(first(items).name);
    EXPECT_FALSE(first(items).name.contains(QStringLiteral("= ASCII"))) << qPrintable(first(items).name);
}

TEST(ProgramAscii, MultiCharMapping)
{
    QList<ResultBuilder::Item> items = ProgramProvider::run(QStringLiteral("ascii ab"));
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(first(items).key, "prog-ascii-multi");
    EXPECT_TRUE(first(items).name.contains(QStringLiteral("'a'=97"))) << qPrintable(first(items).name);
    EXPECT_TRUE(first(items).name.contains(QStringLiteral("'b'=98"))) << qPrintable(first(items).name);
}

TEST(ProgramAscii, OutOfRangeNumberFallsBackToPerChar)
{
    QList<ResultBuilder::Item> items = ProgramProvider::run(QStringLiteral("ascii 300"));
    ASSERT_EQ(items.size(), 1);
    EXPECT_EQ(first(items).key, "prog-ascii-multi");
}

TEST(ProgramAscii, EmptyContentReturnsNothing)
{
    EXPECT_TRUE(ProgramProvider::run(QStringLiteral("ascii")).isEmpty());
}

// ── 时间戳 ────────────────────────────────────────────────────────────
TEST(ProgramTimestamp, BareKeywordGivesCurrentTime)
{
    QList<ResultBuilder::Item> items = ProgramProvider::run(QStringLiteral("时间戳"));
    ASSERT_EQ(items.size(), 2);
    EXPECT_TRUE(hasKey(items, "prog-ts-sec"));
    EXPECT_TRUE(hasKey(items, "prog-ts-ms"));
}

TEST(ProgramTimestamp, TenDigitSecondsIsReversed)
{
    QList<ResultBuilder::Item> items = ProgramProvider::run(QStringLiteral("1760000000"));
    ASSERT_EQ(items.size(), 2);
    EXPECT_EQ(first(items).key, "prog-ts-date");
    EXPECT_TRUE(first(items).name.contains(QStringLiteral("1760000000")));
    EXPECT_EQ(items.at(1).key, "prog-ts-date-sec");
    EXPECT_TRUE(items.at(1).name.contains(QStringLiteral("1760000000")));
}

TEST(ProgramTimestamp, ThirteenDigitsMillisecondsIsReversed)
{
    QList<ResultBuilder::Item> items = ProgramProvider::run(QStringLiteral("1760000000000"));
    ASSERT_EQ(items.size(), 2);
    EXPECT_EQ(first(items).key, "prog-ts-date");
    // 毫秒级换算到秒后与 1760000000 一致
    EXPECT_TRUE(items.at(1).name.contains(QStringLiteral("1760000000"))) << qPrintable(items.at(1).name);
}

TEST(ProgramTimestamp, KeywordVariants)
{
    EXPECT_FALSE(ProgramProvider::run(QStringLiteral("当前时间戳")).isEmpty());
    EXPECT_FALSE(ProgramProvider::run(QStringLiteral("timestamp")).isEmpty());
}

// ── 不误判 ────────────────────────────────────────────────────────────
TEST(ProgramUnknown, ReturnsNoItems)
{
    EXPECT_TRUE(ProgramProvider::run(QStringLiteral("你好")).isEmpty());
    EXPECT_TRUE(ProgramProvider::run(QStringLiteral("")).isEmpty());
    EXPECT_TRUE(ProgramProvider::run(QStringLiteral("dde grand search")).isEmpty());
}

TEST(ProgramUnknown, ElevenDigitsIsNotATimestamp)
{
    // 仅 10 位/13 位纯数字按时间戳处理
    EXPECT_TRUE(ProgramProvider::run(QStringLiteral("17600000000")).isEmpty());
}
