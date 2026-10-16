// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#include "currencyprovider.h"
#include "i18n.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>

// 省略目标币种时全列（与 i18n.cpp currencyTable 对齐，覆盖全部已支持币种）
const QStringList CurrencyProvider::kDefaultTargets = {
    "CNY", "USD", "EUR", "JPY", "GBP", "HKD", "KRW", "RUB",
    "AUD", "CAD", "SGD", "THB", "INR", "TWD", "MOP",
};

static QString currencyDisplay(const QString &code)
{
    return I18n::currencyName(code);
}

// 内置静态汇率（相对 CNY），作为离线降级。数值为示意，联网后会被覆盖。
// 覆盖全部已支持币种，保证「省略目标=全币种列」离线也能出完整结果。
const QHash<QString, double> kStaticRates = {
    {"CNY", 1.0}, {"USD", 7.25}, {"EUR", 7.85}, {"JPY", 0.048},
    {"GBP", 9.20}, {"HKD", 0.93}, {"KRW", 0.0053}, {"RUB", 0.083},
    {"AUD", 4.80}, {"CAD", 5.30}, {"SGD", 5.40}, {"THB", 0.20},
    {"INR", 0.087}, {"TWD", 0.225}, {"MOP", 0.90},
};

CurrencyProvider::CurrencyProvider(QObject *parent)
    : QObject(parent)
{
    loadStaticTable();
    fetchRates(); // 启动即尝试刷新缓存
}

void CurrencyProvider::loadStaticTable()
{
    for (auto it = kStaticRates.begin(); it != kStaticRates.end(); ++it)
        m_rates[it.key()] = it.value();
    m_cacheTime = QDateTime::currentDateTime();
}

bool CurrencyProvider::rate(const QString &code, double &out) const
{
    if (!m_rates.contains(code))
        return false;
    out = m_rates.value(code);
    return true;
}

QList<ResultBuilder::Item> CurrencyProvider::convert(double value, const QString &from,
                                                     const QString &to)
{
    QList<ResultBuilder::Item> items;
    double fromRate = 0.0;
    if (!rate(from.toUpper(), fromRate) || fromRate <= 0.0)
        return items;

    double cny = value * fromRate; // 先折算到 CNY

    QStringList targets;
    if (!to.isEmpty())
        targets << to.toUpper();
    else
        targets = kDefaultTargets;

    for (const QString &tc : targets) {
        if (tc == from.toUpper())
            continue;
        double toRate = 0.0;
        if (!rate(tc, toRate) || toRate <= 0.0)
            continue;
        double out = cny / toRate;
        ResultBuilder::Item it;
        it.key = QString("cur-%1-%2-%3").arg(from.toUpper()).arg(value).arg(tc);
        it.name = QString("%1 %2 = %3 %4")
            .arg(ResultBuilder::formatNumber(value, 2)).arg(currencyDisplay(from.toUpper()))
            .arg(ResultBuilder::formatNumber(out, 2)).arg(currencyDisplay(tc));
        it.icon = "preferences-system"; // 货币图标主题名，依环境
        it.type = "convert/currency";
        items.append(it);
    }
    return items;
}

void CurrencyProvider::fetchRates()
{
    if (m_fetching)
        return;
    m_fetching = true;

    // 使用只读公开 API（frankfurter.app，免 key，EUR 基准，转换为相对 CNY）
    // 失败时保持静态表，不影响功能。
    static QNetworkAccessManager *nam = nullptr;
    if (!nam)
        nam = new QNetworkAccessManager(this);

    QUrl url("https://api.frankfurter.app/latest?from=CNY&to=USD,EUR,JPY,GBP,HKD,KRW");
    QNetworkRequest req(url);
    // 网络超时保护：弱网/无网时避免 reply 长时间挂起、m_fetching 一直为 true 不再重试。
    // convert() 本身只读缓存（静态降级表兜底），超时不影响搜索结果返回。
    req.setTransferTimeout(8000); // 8 秒
    QNetworkReply *reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        m_fetching = false;
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if (doc.isObject()) {
                QJsonObject root = doc.object();
                QJsonObject rates = root.value("rates").toObject();
                // frankfurter 返回「1 CNY = X 目标币」，需取倒数得到「目标币 → CNY」
                for (auto it = rates.begin(); it != rates.end(); ++it) {
                    double v = it.value().toDouble();
                    if (v > 0.0)
                        m_rates[it.key()] = 1.0 / v;
                }
                m_rates["CNY"] = 1.0;
                m_cacheTime = QDateTime::currentDateTime();
            }
        }
        reply->deleteLater();
    });
}
