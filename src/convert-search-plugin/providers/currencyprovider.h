// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CURRENCYPROVIDER_H
#define CURRENCYPROVIDER_H

#include <QString>
#include <QList>
#include <QHash>
#include <QDateTime>

#include "resultbuilder.h"

// 汇率换算：静态降级表（离线可用）+ 后台联网刷新缓存（更准）。
// Search 调用必须快速返回，因此不阻塞等待网络：先返回缓存/静态结果，
// 联网结果在后台更新，下次搜索生效。
class CurrencyProvider : public QObject {
    Q_OBJECT
public:
    explicit CurrencyProvider(QObject *parent = nullptr);

    // 换算 value 个 from 币种；to 为空时展示若干常用目标币种。
    QList<ResultBuilder::Item> convert(double value, const QString &from,
                                       const QString &to);

    // 获取某币种对基准(CNY)的汇率；返回 false 表示无数据。
    bool rate(const QString &code, double &out) const;

private:
    void fetchRates();                 // 后台联网刷新
    void loadStaticTable();            // 内置静态汇率（相对 CNY）
    void applyRates(const QJsonObject &rates); // 合并到 m_rates

    QHash<QString, double> m_rates;    // 币种 → 相对 CNY 的汇率
    QDateTime m_cacheTime;
    bool m_fetching = false;

    // 常用展示目标（to 为空时）
    static const QStringList kDefaultTargets;
};

#endif // CURRENCYPROVIDER_H
