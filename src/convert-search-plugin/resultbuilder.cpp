// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resultbuilder.h"

#include <QJsonArray>
#include <QJsonDocument>

QJsonObject ResultBuilder::makeRoot(const QString &mID)
{
    QJsonObject root;
    root["ver"] = "1.0";
    root["mID"] = mID;
    root["cont"] = QJsonArray();
    return root;
}

void ResultBuilder::addGroup(QJsonObject &root, const QString &group,
                             const QList<Item> &items)
{
    if (items.isEmpty())
        return;

    QJsonArray contents = root["cont"].toArray();
    QJsonObject groupObj;
    groupObj["group"] = group;

    QJsonArray itemArr;
    for (const Item &it : items) {
        if (it.key.isEmpty() || it.name.isEmpty() || it.type.isEmpty())
            continue; // 协议要求三个字段非空，否则被跳过
        QJsonObject obj;
        obj["item"] = it.key;
        obj["name"] = it.name;
        if (!it.icon.isEmpty())
            obj["icon"] = it.icon;
        obj["type"] = it.type;
        itemArr.append(obj);
    }

    if (itemArr.isEmpty())
        return;

    groupObj["items"] = itemArr;
    contents.append(groupObj);
    root["cont"] = contents;
}

QString ResultBuilder::buildEmpty(const QString &mID)
{
    QJsonObject root = makeRoot(mID);
    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Compact);
}

QString ResultBuilder::toJson(QJsonObject &root, const QString &mID)
{
    if (root.isEmpty())
        root = makeRoot(mID);
    else
        root["mID"] = mID;
    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Compact);
}

QString ResultBuilder::formatNumber(double value, int maxDecimals)
{
    // 先按固定精度格式化为字符串（如 "200.0000" / "0.3333"），再去掉尾零。
    QString s = QLocale().toString(value, 'f', qMax(0, maxDecimals));
    if (s.contains(QLocale().decimalPoint())) {
        // 先去掉尾零，再去掉孤立的小数点。
        while (s.endsWith('0'))
            s.chop(1);
        if (s.endsWith(QLocale().decimalPoint()))
            s.chop(1);
    }
    return s;
}
