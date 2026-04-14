// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "timeextractor.h"
#include "searcher/semantic/intentparser.h"

#include <QDebug>
#include <QLoggingCategory>
#include <QDate>

Q_DECLARE_LOGGING_CATEGORY(logDaemon)

/**
 * @brief 中文数字转换为阿拉伯数字
 *
 * 支持的格式：
 * - 单个数字：一、二、三...十
 * - 组合数字：十一、二十、二十三、三十一
 *
 * @param chinese 中文数字字符串
 * @return int 转换后的阿拉伯数字，失败返回 -1
 */
static int chineseToNumber(const QString &chinese)
{
    static const QHash<QChar, int> digitMap = {
        { u'零', 0 }, { u'一', 1 }, { u'二', 2 }, { u'三', 3 }, { u'四', 4 },
        { u'五', 5 }, { u'六', 6 }, { u'七', 7 }, { u'八', 8 }, { u'九', 9 },
        { u'十', 10 }
    };

    if (chinese.isEmpty())
        return -1;

    // 单个字符
    if (chinese.length() == 1) {
        auto it = digitMap.find(chinese[0]);
        return it != digitMap.end() ? it.value() : -1;
    }

    // 处理组合数字
    int result = 0;

    // 特殊处理 "十X" (11-19)
    if (chinese.startsWith(u'十')) {
        result = 10;
        if (chinese.length() > 1) {
            auto it = digitMap.find(chinese[1]);
            if (it != digitMap.end())
                result += it.value();
        }
        return result;
    }

    // 处理 "X十" 或 "X十Y" (20-99)
    for (int i = 0; i < chinese.length(); ++i) {
        QChar c = chinese[i];
        if (c == u'十') {
            // 十位，前面应该有个位数
            if (result == 0)
                result = 10;   // 单独的"十"
            else
                result *= 10;
        } else {
            auto it = digitMap.find(c);
            if (it != digitMap.end()) {
                if (result >= 10) {
                    // 已经有十位数，加个位
                    result += it.value();
                } else {
                    result = it.value();
                }
            }
        }
    }

    return result > 0 ? result : -1;
}

/**
 * @brief 根据时间单位和偏移量计算时间
 *
 * @param base 基准时间
 * @param offset 偏移量（可为负数）
 * @param unit 时间单位
 * @return QDateTime 计算后的时间
 */
static QDateTime addTimeWithUnit(const QDateTime &base, int offset, DFMSEARCH::TimeUnit unit)
{
    switch (unit) {
    case DFMSEARCH::TimeUnit::Minutes:
        return base.addSecs(offset * 60);
    case DFMSEARCH::TimeUnit::Hours:
        return base.addSecs(offset * 3600);
    case DFMSEARCH::TimeUnit::Days:
        return base.addDays(offset);
    case DFMSEARCH::TimeUnit::Weeks:
        return base.addDays(offset * 7);
    case DFMSEARCH::TimeUnit::Months:
        return base.addMonths(offset);
    case DFMSEARCH::TimeUnit::Years:
        return base.addYears(offset);
    default:
        return base;
    }
}

/**
 * @brief 从捕获组获取数字（支持中文和阿拉伯数字）
 */
static int getNumberFromCapture(const QRegularExpressionMatch &match, int group, bool isChinese)
{
    QString text = match.captured(group);
    if (text.isEmpty())
        return -1;

    if (isChinese) {
        return chineseToNumber(text);
    }

    return text.toInt();
}

/**
 * @brief 解析相对星期
 *
 * @param direction 方向："上"/"下"/"这"
 * @param weekday 星期几（中文）
 * @return TimeConstraint
 */
static TimeConstraint parseRelativeWeekday(const QString &direction, const QString &weekday)
{
    static const QHash<QString, int> weekdayMap = {
        { "一", 1 }, { "二", 2 }, { "三", 3 }, { "四", 4 },
        { "五", 5 }, { "六", 6 }, { "七", 7 }, { "天", 7 }, { "日", 7 }
    };

    int targetDay = weekdayMap.value(weekday, -1);
    if (targetDay == -1)
        return TimeConstraint();

    QDate today = QDate::currentDate();
    int currentDayOfWeek = today.dayOfWeek();   // 1=周一, 7=周日

    // Qt 中周日是 7，但我们用 7 表示周日
    if (targetDay == 7)
        targetDay = 7;

    QDate targetDate;

    if (direction == "这") {
        // 本周的某一天
        int diff = targetDay - currentDayOfWeek;
        targetDate = today.addDays(diff);
    } else if (direction == "上") {
        // 上周的某一天
        int diff = targetDay - currentDayOfWeek - 7;
        targetDate = today.addDays(diff);
    } else if (direction == "下") {
        // 下周的某一天
        int diff = targetDay - currentDayOfWeek + 7;
        targetDate = today.addDays(diff);
    } else {
        return TimeConstraint();
    }

    // 返回单日范围（当天 00:00:00 到 23:59:59）
    return TimeConstraint::makeCustom(
        QDateTime(targetDate, QTime(0, 0, 0)),
        QDateTime(targetDate, QTime(23, 59, 59))
    );
}

void TimeExtractor::extract(const QString &input, ParsedIntent &intent)
{
    QString timeId;
    QRegularExpressionMatch match;

    if (m_engine->match("time", input, &timeId, &match)) {
        RegexRule rule = m_engine->getRule(timeId);
        intent.timeConstraint = parseTimeFromMetadata(rule, match);

        // 记录已消费区间
        intent.consumedSpans.append(MatchSpan(match.capturedStart(), match.capturedEnd(), timeId));
        qCDebug(logDaemon) << "Detected time:" << timeId
                 << "kind:" << static_cast<int>(intent.timeConstraint.kind);
    }
}

/**
 * @brief 从规则元数据解析时间约束
 *
 * 规则 metadata 中的 "type" 字段决定约束类型：
 * - "preset"  : 预设时间段，metadata["preset"] 指定具体预设名
 * - "relative": 相对时间（最近 N 单位），从正则捕获组获取数量，
 *               metadata["unit"] 指定单位（days/weeks/months/years）
 * - "custom"  : 自定义时间范围（保留扩展）
 */
TimeConstraint TimeExtractor::parseTimeFromMetadata(const RegexRule &rule,
                                                    const QRegularExpressionMatch &match)
{
    if (!rule.metadata.contains("type")) {
        return TimeConstraint();
    }

    const QString type = rule.metadata["type"].toString();

    if (type == "preset") {
        // 预设时间段：直接映射到 TimeConstraint::Preset
        const QString presetName = rule.metadata["preset"].toString();

        static const QHash<QString, TimeConstraint::Preset> presetMap = {
            { "today", TimeConstraint::Preset::Today },
            { "yesterday", TimeConstraint::Preset::Yesterday },
            { "this_week", TimeConstraint::Preset::ThisWeek },
            { "last_week", TimeConstraint::Preset::LastWeek },
            { "this_month", TimeConstraint::Preset::ThisMonth },
            { "last_month", TimeConstraint::Preset::LastMonth },
            { "this_year", TimeConstraint::Preset::ThisYear },
            { "last_year", TimeConstraint::Preset::LastYear },
        };

        auto it = presetMap.find(presetName);
        if (it != presetMap.end()) {
            return TimeConstraint::makePreset(it.value());
        }

        qCWarning(logDaemon) << "TimeExtractor: unknown preset name:" << presetName;
        return TimeConstraint();

    } else if (type == "relative") {
        // 相对时间：最近 N 天/周/月/年
        int captureGroup = rule.metadata.value("capture_group", 1).toInt();
        if (captureGroup == 0) captureGroup = 1;

        const QString capturedText = match.captured(captureGroup);
        int value = capturedText.toInt();
        if (value <= 0) {
            qCWarning(logDaemon) << "TimeExtractor: invalid relative value:" << capturedText;
            return TimeConstraint();
        }

        // 解析单位，默认为 days
        const QString unitStr = rule.metadata.value("unit", "days").toString();

        static const QHash<QString, DFMSEARCH::TimeUnit> unitMap = {
            { "minutes", DFMSEARCH::TimeUnit::Minutes },
            { "hours", DFMSEARCH::TimeUnit::Hours },
            { "days", DFMSEARCH::TimeUnit::Days },
            { "weeks", DFMSEARCH::TimeUnit::Weeks },
            { "months", DFMSEARCH::TimeUnit::Months },
            { "years", DFMSEARCH::TimeUnit::Years },
        };

        DFMSEARCH::TimeUnit unit = DFMSEARCH::TimeUnit::Days;
        auto it = unitMap.find(unitStr);
        if (it != unitMap.end()) {
            unit = it.value();
        }

        return TimeConstraint::makeRelative(value, unit);

    } else if (type == "relative_range") {
        // 相对时间范围：从某个偏移到另一个偏移
        int startOffset = rule.metadata.value("start_offset", 0).toInt();
        int endOffset = rule.metadata.value("end_offset", 0).toInt();
        const QString unitStr = rule.metadata.value("unit", "days").toString();

        static const QHash<QString, DFMSEARCH::TimeUnit> unitMap = {
            { "minutes", DFMSEARCH::TimeUnit::Minutes },
            { "hours", DFMSEARCH::TimeUnit::Hours },
            { "days", DFMSEARCH::TimeUnit::Days },
            { "weeks", DFMSEARCH::TimeUnit::Weeks },
            { "months", DFMSEARCH::TimeUnit::Months },
            { "years", DFMSEARCH::TimeUnit::Years },
        };

        DFMSEARCH::TimeUnit unit = DFMSEARCH::TimeUnit::Days;
        auto it = unitMap.find(unitStr);
        if (it != unitMap.end()) {
            unit = it.value();
        }

        QDateTime now = QDateTime::currentDateTime();
        QDateTime startTime, endTime;

        // 计算起始时间
        if (startOffset == -999999) {
            // 特殊值：表示系统初始时间（Unix 纪元）
            startTime = QDateTime(QDate(1970, 1, 1), QTime(0, 0, 0));
        } else {
            startTime = addTimeWithUnit(now, startOffset, unit);
        }

        // 计算结束时间
        endTime = addTimeWithUnit(now, endOffset, unit);

        // 确保开始时间不晚于结束时间
        if (startTime > endTime) {
            qCWarning(logDaemon) << "TimeExtractor: start time is after end time, swapping";
            std::swap(startTime, endTime);
        }

        return TimeConstraint::makeCustom(startTime, endTime);

    } else if (type == "custom") {
        // 自定义时间范围
        const QString format = rule.metadata.value("format").toString();
        bool isChinese = rule.metadata.value("chinese_numbers", false).toBool();
        bool useCurrentYear = rule.metadata.value("default_year").toString() == "current";

        int currentYear = QDate::currentDate().year();

        // 单日期格式
        if (format == "full_date" || format == "iso_date") {
            int yearGroup = rule.metadata.value("year_group", 1).toInt();
            int monthGroup = rule.metadata.value("month_group", 2).toInt();
            int dayGroup = rule.metadata.value("day_group", 3).toInt();

            int year = getNumberFromCapture(match, yearGroup, isChinese);
            int month = getNumberFromCapture(match, monthGroup, isChinese);
            int day = getNumberFromCapture(match, dayGroup, isChinese);

            if (year <= 0 || month <= 0 || month > 12 || day <= 0 || day > 31) {
                qCWarning(logDaemon) << "TimeExtractor: invalid date values:" << year << month << day;
                return TimeConstraint();
            }

            QDate date(year, month, day);
            if (!date.isValid()) {
                qCWarning(logDaemon) << "TimeExtractor: invalid date:" << year << month << day;
                return TimeConstraint();
            }

            // 单日范围：00:00:00 - 23:59:59
            return TimeConstraint::makeCustom(
                QDateTime(date, QTime(0, 0, 0)),
                QDateTime(date, QTime(23, 59, 59))
            );

        } else if (format == "month_day" || format == "chinese_date_month_day") {
            int monthGroup = rule.metadata.value("month_group", 1).toInt();
            int dayGroup = rule.metadata.value("day_group", 2).toInt();

            int month = getNumberFromCapture(match, monthGroup, isChinese);
            int day = getNumberFromCapture(match, dayGroup, isChinese);

            if (month <= 0 || month > 12 || day <= 0 || day > 31) {
                qCWarning(logDaemon) << "TimeExtractor: invalid month/day:" << month << day;
                return TimeConstraint();
            }

            QDate date(currentYear, month, day);
            if (!date.isValid()) {
                qCWarning(logDaemon) << "TimeExtractor: invalid date:" << currentYear << month << day;
                return TimeConstraint();
            }

            return TimeConstraint::makeCustom(
                QDateTime(date, QTime(0, 0, 0)),
                QDateTime(date, QTime(23, 59, 59))
            );

        } else if (format == "chinese_date_full") {
            int yearGroup = rule.metadata.value("year_group", 1).toInt();
            int monthGroup = rule.metadata.value("month_group", 2).toInt();
            int dayGroup = rule.metadata.value("day_group", 3).toInt();

            // 年份是阿拉伯数字
            int year = match.captured(yearGroup).toInt();
            int month = getNumberFromCapture(match, monthGroup, true);
            int day = getNumberFromCapture(match, dayGroup, true);

            if (year <= 0 || month <= 0 || month > 12 || day <= 0 || day > 31) {
                qCWarning(logDaemon) << "TimeExtractor: invalid chinese date:" << year << month << day;
                return TimeConstraint();
            }

            QDate date(year, month, day);
            if (!date.isValid()) {
                qCWarning(logDaemon) << "TimeExtractor: invalid date:" << year << month << day;
                return TimeConstraint();
            }

            return TimeConstraint::makeCustom(
                QDateTime(date, QTime(0, 0, 0)),
                QDateTime(date, QTime(23, 59, 59))
            );

        }
        // 日期范围格式
        else if (format == "date_range_same_year") {
            int yearGroup = rule.metadata.value("year_group", 1).toInt();
            int startMonthGroup = rule.metadata.value("start_month_group", 2).toInt();
            int startDayGroup = rule.metadata.value("start_day_group", 3).toInt();
            int endMonthGroup = rule.metadata.value("end_month_group", 4).toInt();
            int endDayGroup = rule.metadata.value("end_day_group", 5).toInt();

            int year = getNumberFromCapture(match, yearGroup, isChinese);
            int startMonth = getNumberFromCapture(match, startMonthGroup, isChinese);
            int startDay = getNumberFromCapture(match, startDayGroup, isChinese);
            int endMonth = getNumberFromCapture(match, endMonthGroup, isChinese);
            int endDay = getNumberFromCapture(match, endDayGroup, isChinese);

            QDate startDate(year, startMonth, startDay);
            QDate endDate(year, endMonth, endDay);

            if (!startDate.isValid() || !endDate.isValid()) {
                qCWarning(logDaemon) << "TimeExtractor: invalid date range:" << year << startMonth << startDay << endMonth << endDay;
                return TimeConstraint();
            }

            return TimeConstraint::makeCustom(
                QDateTime(startDate, QTime(0, 0, 0)),
                QDateTime(endDate, QTime(23, 59, 59))
            );

        } else if (format == "date_range_cross_year" || format == "date_range_iso") {
            int startYearGroup = rule.metadata.value("start_year_group", 1).toInt();
            int startMonthGroup = rule.metadata.value("start_month_group", 2).toInt();
            int startDayGroup = rule.metadata.value("start_day_group", 3).toInt();
            int endYearGroup = rule.metadata.value("end_year_group", 4).toInt();
            int endMonthGroup = rule.metadata.value("end_month_group", 5).toInt();
            int endDayGroup = rule.metadata.value("end_day_group", 6).toInt();

            int startYear = getNumberFromCapture(match, startYearGroup, isChinese);
            int startMonth = getNumberFromCapture(match, startMonthGroup, isChinese);
            int startDay = getNumberFromCapture(match, startDayGroup, isChinese);
            int endYear = getNumberFromCapture(match, endYearGroup, isChinese);
            int endMonth = getNumberFromCapture(match, endMonthGroup, isChinese);
            int endDay = getNumberFromCapture(match, endDayGroup, isChinese);

            QDate startDate(startYear, startMonth, startDay);
            QDate endDate(endYear, endMonth, endDay);

            if (!startDate.isValid() || !endDate.isValid()) {
                qCWarning(logDaemon) << "TimeExtractor: invalid cross-year date range";
                return TimeConstraint();
            }

            return TimeConstraint::makeCustom(
                QDateTime(startDate, QTime(0, 0, 0)),
                QDateTime(endDate, QTime(23, 59, 59))
            );

        } else if (format == "date_range_month_day") {
            int startMonthGroup = rule.metadata.value("start_month_group", 1).toInt();
            int startDayGroup = rule.metadata.value("start_day_group", 2).toInt();
            int endMonthGroup = rule.metadata.value("end_month_group", 3).toInt();
            int endDayGroup = rule.metadata.value("end_day_group", 4).toInt();

            int startMonth = getNumberFromCapture(match, startMonthGroup, isChinese);
            int startDay = getNumberFromCapture(match, startDayGroup, isChinese);
            int endMonth = getNumberFromCapture(match, endMonthGroup, isChinese);
            int endDay = getNumberFromCapture(match, endDayGroup, isChinese);

            QDate startDate(currentYear, startMonth, startDay);
            QDate endDate(currentYear, endMonth, endDay);

            if (!startDate.isValid() || !endDate.isValid()) {
                qCWarning(logDaemon) << "TimeExtractor: invalid month-day range";
                return TimeConstraint();
            }

            return TimeConstraint::makeCustom(
                QDateTime(startDate, QTime(0, 0, 0)),
                QDateTime(endDate, QTime(23, 59, 59))
            );
        }

        qCWarning(logDaemon) << "TimeExtractor: unknown custom format:" << format;
        return TimeConstraint();

    } else if (type == "relative_weekday") {
        // 相对星期：上周一、下周五等
        int directionGroup = rule.metadata.value("direction_group", 1).toInt();
        int weekdayGroup = rule.metadata.value("weekday_group", 2).toInt();

        QString direction = match.captured(directionGroup);
        QString weekday = match.captured(weekdayGroup);

        return parseRelativeWeekday(direction, weekday);
    }

    qCWarning(logDaemon) << "TimeExtractor: unknown time type:" << type;
    return TimeConstraint();
}
