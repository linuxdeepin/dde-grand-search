// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#include "timeprovider.h"

#include <QHash>
#include <QDateTime>
#include <QTimeZone>
#include <QLocale>

namespace {
// 城市/关键词 → IANA 时区（覆盖主要城市，可扩展）
const QHash<QString, QString> kCityToZone = {
    {"北京", "Asia/Shanghai"}, {"上海", "Asia/Shanghai"}, {"深圳", "Asia/Shanghai"},
    {"香港", "Asia/Hong_Kong"}, {"台北", "Asia/Taipei"}, {"东京", "Asia/Tokyo"},
    {"首尔", "Asia/Seoul"}, {"新加坡", "Asia/Singapore"}, {"曼谷", "Asia/Bangkok"},
    {"莫斯科", "Europe/Moscow"}, {"柏林", "Europe/Berlin"}, {"巴黎", "Europe/Paris"},
    {"罗马", "Europe/Rome"}, {"伦敦", "Europe/London"}, {"纽约", "America/New_York"},
    {"华盛顿", "America/New_York"}, {"芝加哥", "America/Chicago"},
    {"洛杉矶", "America/Los_Angeles"}, {"旧金山", "America/Los_Angeles"},
    {"多伦多", "America/Toronto"}, {"悉尼", "Australia/Sydney"}, {"墨尔本", "Australia/Melbourne"},
    {"迪拜", "Asia/Dubai"}, {"孟买", "Asia/Kolkata"}, {"伊斯坦布尔", "Europe/Istanbul"},
    {"beijing", "Asia/Shanghai"}, {"shanghai", "Asia/Shanghai"}, {"hongkong", "Asia/Hong_Kong"},
    {"taipei", "Asia/Taipei"}, {"tokyo", "Asia/Tokyo"}, {"seoul", "Asia/Seoul"},
    {"singapore", "Asia/Singapore"}, {"moscow", "Europe/Moscow"}, {"berlin", "Europe/Berlin"},
    {"paris", "Europe/Paris"}, {"rome", "Europe/Rome"}, {"london", "Europe/London"},
    {"newyork", "America/New_York"}, {"nyc", "America/New_York"}, {"chicago", "America/Chicago"},
    {"losangeles", "America/Los_Angeles"}, {"toronto", "America/Toronto"},
    {"sydney", "Australia/Sydney"}, {"dubai", "Asia/Dubai"},
};

QString findZone(const QString &kw)
{
    QString s = kw.toLower();
    s.remove(' ');
    // 直接命中
    if (kCityToZone.contains(s))
        return kCityToZone.value(s);
    // 包含命中（如「北京时间」「tokyo now」）
    for (auto it = kCityToZone.begin(); it != kCityToZone.end(); ++it) {
        if (s.contains(it.key()))
            return it.value();
    }
    return QString();
}

QString zoneOffsetText(const QTimeZone &tz, const QDateTime &utcNow)
{
    int offsetSecs = tz.offsetFromUtc(utcNow);
    int diffH = offsetSecs / 3600;
    int diffM = (offsetSecs % 3600) / 60;
    QString sign = diffH >= 0 ? "+" : "-";
    return QString("UTC%1%2:%3")
        .arg(sign).arg(qAbs(diffH), 2, 10, QChar('0'))
        .arg(qAbs(diffM), 2, 10, QChar('0'));
}
} // namespace

// 单城市结果卡：目标城市时间 + 与本地时差；空 zoneId 返回空列表。
static QList<ResultBuilder::Item> oneCity(const QString &zoneId, bool withHere = false)
{
    QList<ResultBuilder::Item> items;
    QTimeZone tz(zoneId.toUtf8());
    if (!tz.isValid())
        return items;

    QDateTime utcNow = QDateTime::currentDateTimeUtc();
    QDateTime local = utcNow.toTimeZone(tz);
    QDateTime here = QDateTime::currentDateTime();

    // 与本地时差
    qint64 diffSecs = tz.offsetFromUtc(utcNow) - here.offsetFromUtc();
    QString diffText;
    if (diffSecs == 0)
        diffText = "本地时差 +0h";
    else {
        int h = int(diffSecs / 3600);
        int m = int(qAbs(diffSecs % 3600) / 60);
        diffText = QString("本地时差 %1%2h%3")
            .arg(h >= 0 ? "+" : "-").arg(qAbs(h)).arg(m ? QString("%1m").arg(m) : "");
    }

    QString weekday = QLocale(QLocale::Chinese).toString(local.date(), "ddd");

    ResultBuilder::Item it;
    it.key = QString("time-%1").arg(zoneId);
    it.name = QString("%1 %2 (%3) · %4 · %5")
        .arg(QLocale(QLocale::Chinese).toString(local.time(), "HH:mm"))
        .arg(zoneId)                       // 时区 ID 作为可读补充
        .arg(zoneOffsetText(tz, utcNow))   // 若需更友好可替换为城市中文名
        .arg(weekday)
        .arg(diffText);
    if (withHere) {
        it.name += QString(" · 本地此刻 %1")
            .arg(QLocale(QLocale::Chinese).toString(here.time(), "HH:mm"));
    }
    it.icon = "preferences-system-time";
    it.type = "convert/time";
    items.append(it);
    return items;
}

// 世界时钟常用城市（无城市词时全列，可展开）
static const QStringList kWorldCities = {
    "Asia/Shanghai", "Asia/Tokyo", "America/New_York", "Europe/London",
    "Europe/Paris", "Australia/Sydney",
};

QList<ResultBuilder::Item> TimeProvider::query(const QString &cityKeyword)
{
    QList<ResultBuilder::Item> items;

    // 无城市词 → 世界时钟全列（本地 + 常用参照城市），并排显示本地此刻
    if (cityKeyword.trimmed().isEmpty()) {
        for (const QString &zoneId : kWorldCities)
            items += oneCity(zoneId, /*withHere=*/true);
        return items;
    }

    // 多城市：按空格分隔，逐个返回（支持「东京 纽约 伦敦」）
    QStringList cities = cityKeyword.split(' ', Qt::SkipEmptyParts);
    for (const QString &c : cities) {
        QString zoneId = findZone(c);
        if (zoneId.isEmpty())
            continue;
        items += oneCity(zoneId, /*withHere=*/true);
    }
    // 若拆分成多段都没匹配，退回整串匹配一次（如「北京时间」）
    if (items.isEmpty()) {
        QString zoneId = findZone(cityKeyword);
        if (!zoneId.isEmpty())
            items += oneCity(zoneId, /*withHere=*/true);
    }
    return items;
}
