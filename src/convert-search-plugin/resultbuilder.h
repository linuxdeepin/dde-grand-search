// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESULTBUILDER_H
#define RESULTBUILDER_H

#include <QString>
#include <QJsonObject>

// 组装符合 dde-grand-search V1.0 协议的搜索结果 JSON。
class ResultBuilder {
public:
    // 单条结果项
    struct Item {
        QString key;    // item：组内唯一标识（后续 Action 用）
        QString name;   // name：界面展示文本
        QString icon;   // icon：图标名或路径
        QString type;   // type：MIME 风格类型
    };

    // 添加一个分组（group 为分组名，如「汇率」「单位」「时间」）
    static void addGroup(QJsonObject &root, const QString &group,
                         const QList<Item> &items);

    // 构建带空结果的返回（cont 为空数组）
    static QString buildEmpty(const QString &mID);

    // 把 root 序列化为紧凑 JSON 字符串
    static QString toJson(QJsonObject &root, const QString &mID);

    // 数值格式化：最多保留 maxDecimals 位小数，并去掉无意义尾零。
    // 例：formatNumber(200, 4) -> "200"；formatNumber(0.333333,4) -> "0.3333"。
    // 用于消除固定 'f' 格式产生的过多尾零（如 200.0000）。
    static QString formatNumber(double value, int maxDecimals);

private:
    static QJsonObject makeRoot(const QString &mID);
};

#endif // RESULTBUILDER_H
