// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#include "datetimeprovider.h"

#include <QRegularExpression>
#include <QDate>
#include <QDateTime>
#include <QLocale>

namespace {

// 提取文本中所有 YYYY-MM-DD / YYYY.M.D / YYYY/MM/DD 形式的日期
QList<QDate> extractDates(const QString &s)
{
    QList<QDate> out;
    QRegularExpression re(R"((\d{4})[-./](\d{1,2})[-./](\d{1,2}))");
    QRegularExpressionMatchIterator it = re.globalMatch(s);
    while (it.hasNext()) {
        QRegularExpressionMatch m = it.next();
        QDate d = QDate(m.captured(1).toInt(), m.captured(2).toInt(), m.captured(3).toInt());
        if (d.isValid())
            out.append(d);
    }
    // 中文：X月X日（默认今年）
    QRegularExpression cnRe(R"((\d{1,2})月(\d{1,2})[日号])");
    QRegularExpressionMatchIterator cnIt = cnRe.globalMatch(s);
    int year = QDate::currentDate().year();
    while (cnIt.hasNext()) {
        QRegularExpressionMatch m = cnIt.next();
        QDate d = QDate(year, m.captured(1).toInt(), m.captured(2).toInt());
        if (d.isValid())
            out.append(d);
    }
    return out;
}

// 提取完整日期时间（含时分秒），支持「2026年08月20日 15:03:28」「2026-08-20 15:03:28」等。
// 返回本地时区的 QDateTime；无法解析返回无效对象。
QDateTime extractDateTime(const QString &s)
{
    // 中文形态：2026年08月20日 15:03:28（星期几/CST 等冗余词忽略）
    QRegularExpression cnRe(R"((\d{4})年\s*(\d{1,2})月\s*(\d{1,2})[日号]?\s*(\d{1,2})[:：](\d{1,2})(?:[:：](\d{1,2}))?)");
    QRegularExpressionMatch cm = cnRe.match(s);
    if (cm.hasMatch()) {
        QDateTime dt(QDate(cm.captured(1).toInt(), cm.captured(2).toInt(), cm.captured(3).toInt()),
                     QTime(cm.captured(4).toInt(), cm.captured(5).toInt(),
                           cm.captured(6).isEmpty() ? 0 : cm.captured(6).toInt()));
        if (dt.isValid())
            return dt;
    }
    // 标准形态：2026-08-20 15:03:28（或 / . 分隔）
    QRegularExpression stdRe(R"((\d{4})[-./](\d{1,2})[-./](\d{1,2})\s+(\d{1,2})[:：](\d{1,2})(?:[:：](\d{1,2}))?)");
    QRegularExpressionMatch sm = stdRe.match(s);
    if (sm.hasMatch()) {
        QDateTime dt(QDate(sm.captured(1).toInt(), sm.captured(2).toInt(), sm.captured(3).toInt()),
                     QTime(sm.captured(4).toInt(), sm.captured(5).toInt(),
                           sm.captured(6).isEmpty() ? 0 : sm.captured(6).toInt()));
        if (dt.isValid())
            return dt;
    }
    return QDateTime();
}

QString weekdayText(const QDate &d)
{
    return QLocale(QLocale::Chinese).toString(d, "yyyy-MM-dd dddd");
}

} // namespace

QList<ResultBuilder::Item> DateTimeProvider::run(const QString &text)
{
    QList<ResultBuilder::Item> items;
    QList<QDate> dates = extractDates(text);
    QDate today = QDate::currentDate();
    QString lower = text.toLower();

    // 情况A：日期偏移（+N天 / 加N天 / -N天），如「今天加30天」「2026-08-20 +100天」
    {
        QRegularExpression offsetRe(R"((今天|明天|昨天|\d{4}[-./]\d{1,2}[-./]\d{1,2})\s*(?:加|[+-])\s*(\d+)\s*(?:天|日))");
        QRegularExpressionMatch om = offsetRe.match(lower);
        if (om.hasMatch()) {
            QDate base = today;
            QString baseStr = om.captured(1);
            if (baseStr == "明天") base = today.addDays(1);
            else if (baseStr == "昨天") base = today.addDays(-1);
            else if (baseStr != "今天") {
                QRegularExpression dRe(R"((\d{4})[-./](\d{1,2})[-./](\d{1,2}))");
                QRegularExpressionMatch dm = dRe.match(baseStr);
                if (dm.hasMatch()) base = QDate(dm.captured(1).toInt(), dm.captured(2).toInt(), dm.captured(3).toInt());
            }
            int delta = om.captured(2).toInt();
            // 判断方向：+/加 = 加，-/减 = 减
            int op = lower.contains("加") || om.captured(0).contains('+') ? 1 : -1;
            QDate target = base.addDays(delta * op);
            ResultBuilder::Item it;
            it.key = "date-offset";
            it.name = QString("%1 %2 %3天 = %4")
                .arg(weekdayText(base))
                .arg(om.captured(0).contains('-') ? "减" : "加")
                .arg(delta)
                .arg(weekdayText(target));
            it.icon = "office-calendar"; it.type = "convert/date-offset";
            items.append(it);
            return items;
        }
    }

    // 情况B：星期几查询，如「2026-08-20 星期几」「2026-08-20 weekday」
    {
        bool askWeekday = lower.contains("星期几") || lower.contains("周几")
                          || lower.contains("weekday") || lower.contains("whatday");
        if (askWeekday && dates.size() >= 1) {
            ResultBuilder::Item it;
            it.key = "date-weekday";
            it.name = weekdayText(dates[0]);
            it.icon = "office-calendar"; it.type = "convert/date-weekday";
            items.append(it);
            return items;
        }
        // 单独「星期几」不带日期 → 今天星期几
        if (askWeekday && dates.isEmpty()) {
            ResultBuilder::Item it;
            it.key = "date-weekday";
            it.name = weekdayText(today);
            it.icon = "office-calendar"; it.type = "convert/date-weekday";
            items.append(it);
            return items;
        }
    }

    // 情况0：完整日期时间 → 时间戳（秒级 / 毫秒级）
    // 例如「2026年08月20日 15:03:28」「2026-08-20 15:03:28」。
    QDateTime dt = extractDateTime(text);
    if (dt.isValid()) {
        qint64 secs = dt.toSecsSinceEpoch();
        qint64 ms = dt.toMSecsSinceEpoch();
        ResultBuilder::Item it;
        it.key = "date-to-ts";
        it.name = QString("%1\n时间戳(秒): %2\n时间戳(毫秒): %3")
            .arg(QLocale(QLocale::Chinese).toString(dt, "yyyy-MM-dd dddd HH:mm:ss"))
            .arg(secs).arg(ms);
        it.icon = "office-calendar"; it.type = "convert/date-to-timestamp";
        items.append(it);
        return items;
    }

    // 情况1：两个日期 → 日期差
    if (dates.size() >= 2) {
        qint64 days = qAbs(dates[0].daysTo(dates[1]));
        ResultBuilder::Item it;
        it.key = "date-diff";
        it.name = QString("%1 至 %2 相差 %3 天")
            .arg(weekdayText(dates[0])).arg(weekdayText(dates[1])).arg(days);
        it.icon = "office-calendar"; it.type = "convert/date-diff";
        items.append(it);
        return items;
    }

    // 情况2：含「倒数/倒计时/距/还有几天/距离」→ 距目标日倒数
    if (dates.size() == 1
        && (lower.contains("倒数") || lower.contains("倒计时") || lower.contains("距")
            || lower.contains("还有几天") || lower.contains("距离") || lower.contains("daysuntil")
            || lower.contains("countdown"))) {
        qint64 days = today.daysTo(dates[0]);
        QString rel = (days >= 0) ? QString("还有 %1 天").arg(days)
                                  : QString("已过去 %1 天").arg(-days);
        ResultBuilder::Item it;
        it.key = "date-countdown";
        it.name = QString("%1 · %2").arg(weekdayText(dates[0])).arg(rel);
        it.icon = "office-calendar"; it.type = "convert/date-countdown";
        items.append(it);
        return items;
    }

    // 情况3：单个日期 → 展示星期与距今天数
    if (dates.size() == 1) {
        qint64 days = today.daysTo(dates[0]);
        ResultBuilder::Item it;
        it.key = "date-info";
        it.name = QString("%1 · 距今天 %2 天")
            .arg(weekdayText(dates[0])).arg(days);
        it.icon = "office-calendar"; it.type = "convert/date-info";
        items.append(it);
        return items;
    }

    // 情况4：仅「今天/明天/昨天/today」→ 展示今天信息
    if (lower.contains("今天") || lower.contains("today")) {
        ResultBuilder::Item it;
        it.key = "date-today";
        it.name = weekdayText(today);
        it.icon = "office-calendar"; it.type = "convert/date-today";
        items.append(it);
        return items;
    }

    return items;
}
