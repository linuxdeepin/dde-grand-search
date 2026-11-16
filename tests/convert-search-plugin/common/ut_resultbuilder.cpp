// SPDX-FileCopyrightText: 2026 testerxydw
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resultbuilder.h"

#include <gtest/gtest.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

ResultBuilder::Item makeItem(const QString &key, const QString &name,
                             const QString &icon, const QString &type)
{
    ResultBuilder::Item it;
    it.key = key;
    it.name = name;
    it.icon = icon;
    it.type = type;
    return it;
}

} // namespace

// ── 数字格式化：消除固定精度产生的尾零 ────────────────────────────────
TEST(ResultBuilderFormat, TrimsTrailingZeros)
{
    EXPECT_EQ(ResultBuilder::formatNumber(200, 4), QStringLiteral("200"));
    EXPECT_EQ(ResultBuilder::formatNumber(0.333333, 4), QStringLiteral("0.3333"));
    EXPECT_EQ(ResultBuilder::formatNumber(2.5, 4), QStringLiteral("2.5"));
    EXPECT_EQ(ResultBuilder::formatNumber(-40, 1), QStringLiteral("-40"));
    EXPECT_EQ(ResultBuilder::formatNumber(62.13711922, 4), QStringLiteral("62.1371"));
}

TEST(ResultBuilderFormat, HandlesZeroAndNegativeSmall)
{
    EXPECT_EQ(ResultBuilder::formatNumber(0.0, 4), QStringLiteral("0"));
    EXPECT_EQ(ResultBuilder::formatNumber(0.5, 1), QStringLiteral("0.5"));
    EXPECT_EQ(ResultBuilder::formatNumber(-0.25, 2), QStringLiteral("-0.25"));
}

TEST(ResultBuilderFormat, ZeroDecimalsMeansNoFractionPart)
{
    EXPECT_EQ(ResultBuilder::formatNumber(3.7, 0), QStringLiteral("4"));
    EXPECT_EQ(ResultBuilder::formatNumber(1234, 0), QStringLiteral("1234"));
}

// ── 分组协议字段 ──────────────────────────────────────────────────────
TEST(ResultBuilderGroup, AddsGroupWithItems)
{
    QJsonObject root;
    const QList<ResultBuilder::Item> items = {
        makeItem("k1", "n1", "icon1", "convert/unit"),
        makeItem("k2", "n2", QString(), "convert/unit"),
    };
    ResultBuilder::addGroup(root, QStringLiteral("单位"), items);

    const QJsonArray cont = root["cont"].toArray();
    ASSERT_EQ(cont.size(), 1);
    const QJsonObject g = cont.at(0).toObject();
    EXPECT_EQ(g["group"].toString(), QStringLiteral("单位"));
    const QJsonArray arr = g["items"].toArray();
    ASSERT_EQ(arr.size(), 2);
    EXPECT_EQ(arr.at(0).toObject()["item"].toString(), QStringLiteral("k1"));
    EXPECT_EQ(arr.at(0).toObject()["name"].toString(), QStringLiteral("n1"));
    EXPECT_EQ(arr.at(0).toObject()["type"].toString(), QStringLiteral("convert/unit"));
    EXPECT_TRUE(arr.at(0).toObject().contains("icon"));
    // icon 为空时不下发该字段
    EXPECT_FALSE(arr.at(1).toObject().contains("icon"));
}

TEST(ResultBuilderGroup, SkipsEmptyItems)
{
    QJsonObject root;
    ResultBuilder::addGroup(root, QStringLiteral("单位"), {});
    EXPECT_TRUE(root.isEmpty());
}

// item/name/type 任一为空即不符合协议，必须被丢弃而非下发脏数据
TEST(ResultBuilderGroup, DropsProtocolIncompleteItems)
{
    QJsonObject root;
    const QList<ResultBuilder::Item> items = {
        makeItem("", "n1", "icon", "convert/unit"),
        makeItem("k2", "", "icon", "convert/unit"),
        makeItem("k3", "n3", "icon", ""),
    };
    ResultBuilder::addGroup(root, QStringLiteral("单位"), items);
    EXPECT_TRUE(root.isEmpty());
}

// 回归：QJsonObject::operator[] 会在 key 缺失时插入 null 值，
// 全部条目被丢弃时曾把 cont 覆盖成 null，破坏 V1.0 协议（cont 必须为数组）
TEST(ResultBuilderGroup, KeepsContAsArrayWhenAllItemsDropped)
{
    QJsonObject root = ResultBuilder::makeRoot(QStringLiteral("7"));
    ResultBuilder::addGroup(root, QStringLiteral("单位"),
                            { makeItem("", "n", "i", "t") });

    ASSERT_TRUE(root.contains(QStringLiteral("cont")));
    EXPECT_EQ(root.value(QStringLiteral("cont")).toArray(), QJsonArray());

    const QString json = QJsonDocument(root).toJson(QJsonDocument::Compact);
    EXPECT_TRUE(json.contains(QStringLiteral("\"cont\":[]"))) << qPrintable(json);
    EXPECT_FALSE(json.contains(QStringLiteral("null"))) << qPrintable(json);
}

TEST(ResultBuilderGroup, DropsGroupWhenAllItemsIncomplete)
{
    QJsonObject root;
    QJsonObject pre;
    ResultBuilder::addGroup(pre, QStringLiteral("已有"),
                             { makeItem("k", "n", "i", "t") });
    root = pre;
    ResultBuilder::addGroup(root, QStringLiteral("无效"),
                            { makeItem("", "n", "i", "t") });
    EXPECT_EQ(root["cont"].toArray().size(), 1);
}

// ── 空结果与序列化 ────────────────────────────────────────────────────
TEST(ResultBuilderJson, BuildEmptyCarriesVersionAndId)
{
    const QString json = ResultBuilder::buildEmpty(QStringLiteral("42"));
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
    ASSERT_EQ(err.error, QJsonParseError::NoError);
    ASSERT_TRUE(doc.isObject());
    const QJsonObject obj = doc.object();
    EXPECT_EQ(obj["ver"].toString(), QStringLiteral("1.0"));
    EXPECT_EQ(obj["mID"].toString(), QStringLiteral("42"));
    EXPECT_TRUE(obj["cont"].isArray());
    EXPECT_TRUE(obj["cont"].toArray().isEmpty());
}

TEST(ResultBuilderJson, ToJsonFillsMissingRoot)
{
    QJsonObject empty;
    const QString json = ResultBuilder::toJson(empty, QStringLiteral("7"));
    const QJsonObject obj = QJsonDocument::fromJson(json.toUtf8()).object();
    EXPECT_EQ(obj["mID"].toString(), QStringLiteral("7"));
    EXPECT_TRUE(obj["cont"].toArray().isEmpty());
}

TEST(ResultBuilderJson, ToJsonOverwritesRequestId)
{
    QJsonObject root;
    ResultBuilder::addGroup(root, QStringLiteral("单位"),
                            { makeItem("k", "n", "i", "convert/unit") });
    root["mID"] = QStringLiteral("stale");
    const QJsonObject obj =
        QJsonDocument::fromJson(ResultBuilder::toJson(root, QStringLiteral("99")).toUtf8()).object();
    EXPECT_EQ(obj["mID"].toString(), QStringLiteral("99"));
    EXPECT_EQ(obj["cont"].toArray().size(), 1);
}

TEST(ResultBuilderJson, OutputIsCompact)
{
    const QString json = ResultBuilder::buildEmpty(QStringLiteral("1"));
    EXPECT_FALSE(json.contains(QLatin1Char('\n')));
}
