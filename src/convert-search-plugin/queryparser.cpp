// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#include "queryparser.h"

#include <QRegularExpression>
#include <QLocale>

namespace QueryParser {

// 货币符号/代码/中文名 → ISO 代码
static const QHash<QString, QString> kCurrencyAliases = {
    {"$", "USD"}, {"us$", "USD"}, {"美元", "USD"}, {"美金", "USD"},
    {"€", "EUR"}, {"eur", "EUR"}, {"欧元", "EUR"},
    {"¥", "CNY"}, {"￥", "CNY"}, {"cny", "CNY"}, {"rmb", "CNY"}, {"人民币", "CNY"}, {"元", "CNY"},
    {"£", "GBP"}, {"gbp", "GBP"}, {"英镑", "GBP"},
    {"jpy", "JPY"}, {"日元", "JPY"}, {"円", "JPY"},
    {"hkd", "HKD"}, {"港币", "HKD"}, {"港币", "HKD"},
    {"krw", "KRW"}, {"韩元", "KRW"},
    {"usd", "USD"},
    // 静态汇率表支持但需显式别名的币种（其中 RUB/TWD/MOP 无在线源，走静态表）
    {"rub", "RUB"}, {"卢布", "RUB"},
    {"aud", "AUD"}, {"澳元", "AUD"}, {"澳币", "AUD"},
    {"cad", "CAD"}, {"加元", "CAD"},
    {"sgd", "SGD"}, {"新币", "SGD"}, {"新加坡元", "SGD"},
    {"thb", "THB"}, {"泰铢", "THB"},
    {"inr", "INR"}, {"卢比", "INR"},
    {"twd", "TWD"}, {"新台币", "TWD"},
    {"mop", "MOP"}, {"澳门元", "MOP"},
};

// 常见单位词（含中文），用于单位识别
static const QStringList kUnitWords = {
    // 长度
    "inch", "in", "英寸", "cm", "厘米", "mm", "毫米", "km", "千米", "公里",
    "m", "米", "mile", "英里", "feet", "ft", "英尺", "yard", "yd", "码",
    // 重量
    "kg", "千克", "公斤", "g", "克", "lb", "lbs", "磅", "oz", "盎司", "斤", "两",
    // 面积
    "m2", "平方米", "km2", "平方千米", "acre", "英亩", "公顷", "亩",
    // 体积（注意：单字母 l 易子串误判，改用中文/明确词）
    "升", "ml", "毫升", "gal", "加仑", "liter", "litre",
    // 速度
    "kmh", "kph", "km/h", "mph", "英里每小时",
    // 数据
    "kb", "mb", "gb", "tb", "千字节", "兆字节", "吉字节",
};

static bool containsUnitWord(const QString &s)
{
    QString lower = s.toLower();
    for (const QString &u : kUnitWords) {
        if (lower.contains(u))
            return true;
    }
    return false;
}

// 城市/时区关键词（中文名 + 英文名）
static const QStringList kTimeKeywords = {
    "北京", "上海", "东京", "纽约", "伦敦", "巴黎", "洛杉矶", "旧金山", "悉尼",
    "莫斯科", "首尔", "新加坡", "香港", "台北", "柏林", "罗马", "多伦多", "芝加哥",
    "beijing", "shanghai", "tokyo", "newyork", "nyc", "london", "paris", "losangeles",
    "sanfrancisco", "sydney", "moscow", "seoul", "singapore", "hongkong", "taipei",
    "berlin", "rome", "toronto", "chicago", "time",
};

static bool containsTimeKeyword(const QString &s)
{
    QString lower = s.toLower();
    lower.remove(' ');
    for (const QString &t : kTimeKeywords) {
        if (lower.contains(t))
            return true;
    }
    return false;
}

// 程序员工具关键词（中文名 + 英文名）
// 注意：hex/bin/oct 属进制转换（Calc），不在此列，避免与进制转换冲突
static const QStringList kProgramKeywords = {
    "base64", "ascii", "url", "hash", "md5", "sha1", "sha256", "sha",
    "编码", "哈希", "时间戳", "当前时间戳", "毫秒时间戳",
};

static bool containsProgramKeyword(const QString &s)
{
    QString lower = s.toLower();
    lower.remove(' ');
    for (const QString &t : kProgramKeywords) {
        if (lower.contains(t))
            return true;
    }
    return false;
}

// 日期/倒数日关键词
static const QStringList kDateKeywords = {
    "倒数日", "倒计时", "距", "还有几天", "生日", "日期差", "相隔", "相差", "daysuntil",
    "countdown", "days", "距离", "几天后", "date", "today", "今天", "明天", "昨天",
    "星期几", "周几", "weekday", "whatday", "加天",
};

static bool containsDateKeyword(const QString &s)
{
    QString lower = s.toLower();
    lower.remove(' ');
    for (const QString &t : kDateKeywords) {
        if (lower.contains(t))
            return true;
    }
    return false;
}

// 匹配颜色串：#rgb / #rrggbb / rgb(r,g,b) / hsl(h,s%,l%)
static bool matchColor(const QString &s, QString &out)
{
    QString lower = s.toLower();
    lower.remove(' ');
    QRegularExpression colorRe(R"(^(#([0-9a-f]{3}|[0-9a-f]{6})|rgb\(\d{1,3},\d{1,3},\d{1,3}\)|hsl\(\d{1,3},\d{1,3}%,\d{1,3}%\))$)");
    QRegularExpressionMatch m = colorRe.match(lower);
    if (m.hasMatch()) {
        out = m.captured(0);
        return true;
    }
    return false;
}

// 科学/程序员计算器：算术表达式（含进制前缀 0x/0b/0o）或进制转换请求
static bool matchCalc(const QString &s, QString &expr)
{
    QString lower = s.toLower();
    // 进制转换：数值 + to + hex/bin/oct/dec
    QRegularExpression baseRe(R"(([0-9a-fxbo.]+)\s*(to|->|转为|转)\s*(hex|bin|oct|dec|二进制|八进制|十进制|十六进制))");
    QRegularExpressionMatch bm = baseRe.match(lower);
    if (bm.hasMatch()) {
        expr = lower;
        return true;
    }
    // 算术表达式：含运算符且整体为合法字符（含函数名/常量字母）
    QRegularExpression arithRe(R"(^[\s0-9a-zxbo.+\-*/%^()]+$)");
    if (lower.contains('+') || lower.contains('-') || lower.contains('*')
        || lower.contains('/') || lower.contains('%') || lower.contains('^')
        || lower.startsWith("0x") || lower.startsWith("0b") || lower.startsWith("0o")) {
        // 排除纯单位/货币（已被前面分支拦截），且至少含数字
        if (lower.contains(QRegularExpression(R"([0-9])")) && arithRe.match(lower).hasMatch()) {
            expr = lower;
            return true;
        }
    }
    return false;
}

// 触发词（多语言别名）：前缀命中直接短路到对应类型，剩余内容沿用原特征匹配。
// 不新建抽象：仅静态表 + 前缀匹配，各类型仍指向现有 provider 分支。
static const QHash<QueryParser::Type, QStringList> kTriggerWords = {
    {QueryParser::Type::Currency, {"汇率", "换汇", "currency", "exchange"}},
    {QueryParser::Type::Unit,     {"换算", "单位", "convert", "unit"}},
    {QueryParser::Type::Time,     {"时间", "时区", "time", "timezone", "now", "现在", "现在几点", "几点"}},
    {QueryParser::Type::Calc,     {"计算", "算", "calc", "calculate", "进制", "base"}},
    {QueryParser::Type::Program,  {"编码", "哈希", "时间戳", "encode", "hash", "timestamp"}},
    {QueryParser::Type::DateTime, {"日期", "倒数", "date", "countdown",
                                   "转时间戳", "时间戳转", "to-timestamp", "timestamp-of"}},
    {QueryParser::Type::Color,    {"颜色", "color"}},
    {QueryParser::Type::Energy,   {"热量", "卡路里", "energy", "calorie"}},
};

// 若 input 以某触发词 + 分隔符开头，返回对应类型与去掉前缀后的剩余内容；否则返回 None。
static QueryParser::Type matchTrigger(const QString &s, QString &rest)
{
    rest.clear();
    // Latin 触发词整词判定：base64 不应命中触发词 base
    static const QRegularExpression kLatinWord(R"(^[a-zA-Z]+$)");
    // 收集所有前缀命中的触发词，选「最长」者优先，避免 `时间` 误吞 `时间戳` 等前缀包含冲突。
    int bestLen = -1;
    QueryParser::Type bestType = QueryParser::Type::None;
    QString bestRest;
    for (auto it = kTriggerWords.begin(); it != kTriggerWords.end(); ++it) {
        for (const QString &kw : it.value()) {
            if (s.startsWith(kw) && kw.size() > bestLen) {
                // 触发词为 Latin 字母且其后紧连 Latin 字母数字时，视为普通单词的一部分，
                // 不作为触发词（如 "base64 encode" ≠ 触发词 "base"）；中文触发词不受此限，
                // 保留 "算12*8" 类连写用法。
                const int pos = kw.size();
                if (pos < s.size() && kLatinWord.match(kw).hasMatch()
                    && s.at(pos).toLatin1() != 0 && s.at(pos).isLetterOrNumber())
                    continue;
                QString r = s.mid(kw.size()).trimmed();
                // 允许触发词后紧跟分隔符 : ： ? 再接内容
                // 全角冒号 U+FF1A 超出 Latin1，需用 QChar 显式构造
                if (r.startsWith(QLatin1Char(':')) || r.startsWith(QChar(0xFF1A))
                    || r.startsWith(QLatin1Char('?')))
                    r = r.mid(1).trimmed();
                bestLen = kw.size();
                bestType = it.key();
                bestRest = r;
            }
        }
    }
    if (bestType != QueryParser::Type::None) {
        rest = bestRest;
        return bestType;
    }
    return QueryParser::Type::None;
}

Parsed parse(const QString &input)
{
    Parsed p;
    p.raw = input.trimmed();
    if (p.raw.isEmpty())
        return p;

    QString s = p.raw.toLower();

    // 0) 使用说明：help / ? / ？ / 使用说明 / 帮助 / 用法 / 怎么用
    if (s == "help" || s == "?" || s == "？" || s == "使用说明" || s == "帮助"
        || s == "用法" || s == "怎么用" || s == "功能" || s == "menu") {
        p.type = Type::Help;
        return p;
    }

    // 0.5) 触发词前缀短路：命中则剥离前缀，剩余内容交给原特征匹配（递归一次，复用全部规则）。
    // 例：「转时间戳 2026-08-20 15:03:28」→ DateTime；「时间戳 1690000000」→ Program（当前时间戳）。
    {
        QString rest;
        QueryParser::Type t = matchTrigger(s, rest);
        if (t != QueryParser::Type::None) {
            if (rest.isEmpty()) {
                // 仅有触发词无内容：仍按原逻辑（如「时间戳」单独给当前时间戳）
                p.type = t;
                p.text = p.raw;
                return p;
            }
            Parsed sub = parse(rest);
            // 递归未识别（如「颜色 随机」「红色」）时回退到触发词类型，rest 作为内容
            if (sub.type == QueryParser::Type::None) {
                sub.type = t;
                // 交给 provider 完整原文而非 rest：保留触发词里的动作信息
                // （如 "hash hello" 剥掉触发词后剩 "hello"，provider 将无法识别哈希动作）
                sub.text = p.raw;
            }
            sub.raw = p.raw; // 保留原始大小写（ascii 等大小写敏感场景）
            return sub;
        }
    }

    // 1) 货币：符号或代码出现在数字附近
    // 形如 100usd / $50 / 100 usd to cny / 100美元 / 100usd=?cny
    QRegularExpression curRe(R"(([0-9]+(?:\.[0-9]+)?)\s*(usd|eur|cny|gbp|jpy|hkd|krw|rub|aud|cad|sgd|thb|inr|twd|mop|美元|美金|欧元|人民币|元|英镑|日元|港币|韩元|卢布|澳元|澳币|加元|新币|新加坡元|泰铢|卢比|新台币|澳门元|\$|€|¥|￥|£))");
    QRegularExpressionMatch curM = curRe.match(s);
    if (curM.hasMatch()) {
        p.type = Type::Currency;
        p.value = curM.captured(1).toDouble();
        QString sym = curM.captured(2);
        p.from = kCurrencyAliases.value(sym, sym.toUpper());

        // 目标币种：to / =? / ? 之后
        QRegularExpression toRe(R"((?:to|=|\?)\s*(usd|eur|cny|gbp|jpy|hkd|krw|rub|aud|cad|sgd|thb|inr|twd|mop|美元|美金|欧元|人民币|元|英镑|日元|港币|韩元|卢布|澳元|澳币|加元|新币|新加坡元|泰铢|卢比|新台币|澳门元))");
        QRegularExpressionMatch toM = toRe.match(s);
        if (toM.hasMatch())
            p.to = kCurrencyAliases.value(toM.captured(1), toM.captured(1).toUpper());
        return p;
    }
    // 仅货币符号/代码 + 数字（顺序相反：usd100 / 美元100）
    QRegularExpression curRe2(R"((usd|eur|cny|gbp|jpy|hkd|krw|rub|aud|cad|sgd|thb|inr|twd|mop|美元|美金|欧元|人民币|元|英镑|日元|港币|韩元|卢布|澳元|澳币|加元|新币|新加坡元|泰铢|卢比|新台币|澳门元|\$|€|¥|￥|£)\s*([0-9]+(?:\.[0-9]+)?))");
    QRegularExpressionMatch curM2 = curRe2.match(s);
    if (curM2.hasMatch()) {
        p.type = Type::Currency;
        p.value = curM2.captured(2).toDouble();
        p.from = kCurrencyAliases.value(curM2.captured(1), curM2.captured(1).toUpper());
        return p;
    }

    // 2) 温度：数字 + C/F/K（k 需排除热量词 kcal/kj 与 kg/km 等单位词，避免误判）
    // 负号/正号纳入数值捕获；(?<![0-9.]) 防止「-5c」被从子串「5c」命中而丢符号
    QRegularExpression tempRe(R"((?<![0-9.])((?:[+-])?[0-9]+(?:\.[0-9]+)?)\s*(c|f|k(?!cal|j)|摄氏度|华氏度))");
    QRegularExpressionMatch tempM = tempRe.match(s);
    if (tempM.hasMatch() && !containsUnitWord(s)) {
        p.type = Type::Unit;
        p.value = tempM.captured(1).toDouble();
        p.from = tempM.captured(2) == "f" ? "f" : (tempM.captured(2) == "k" ? "k" : "c");
        // 目标温度标度：to / =? 之后
        QRegularExpression tToRe(R"((?:to|=|\?)\s*(c|f|k))");
        QRegularExpressionMatchIterator tIt = tToRe.globalMatch(s);
        if (tIt.hasNext())
            p.to = tIt.next().captured(1);
        return p;
    }

    // 3.5) 热量转换：数字 + 热量单位词（必须在普通单位之前，避免被 k/cal 等拆分）
    QRegularExpression energyRe(R"(([0-9]+(?:\.[0-9]+)?)\s*(kcal|千卡|大卡|kj|千焦|cal|卡|焦|焦耳))");
    QRegularExpressionMatch enM = energyRe.match(s);
    if (enM.hasMatch()) {
        p.type = Type::Energy;
        p.value = enM.captured(1).toDouble();
        p.from = enM.captured(2);
        // 目标热量单位：to / =? / ? 之后
        QRegularExpression eToRe(R"((?:to|=|\?)\s*(kcal|千卡|大卡|kj|千焦|cal|卡|焦|焦耳))");
        QRegularExpressionMatchIterator eIt = eToRe.globalMatch(s);
        if (eIt.hasNext())
            p.to = eIt.next().captured(1);
        return p;
    }

    // 3) 单位换算：数字 + 单位词（可选 目标单位，支持 to/=/?/空格）
    // 单位部分允许「/」：复合单位 km/h、mph 等速度写法不能被截断成 km（长度误判）
    QRegularExpression numUnitRe(R"(([0-9]+(?:\.[0-9]+)?)\s*([a-z/\x{4e00}-\x{9fff}]+))");
    QRegularExpressionMatch nuM = numUnitRe.match(s);
    if (nuM.hasMatch()) {
        p.from = nuM.captured(2);
        if (containsUnitWord(p.from)) {
            p.type = Type::Unit;
            p.value = nuM.captured(1).toDouble();
            // 目标单位：from 之后的部分，支持 to/=/?/空格 分隔
            QString rest = s.mid(nuM.captured(0).length()).trimmed();
            QRegularExpression toRe(R"(^(?:to|=|\?)\s*([a-z/\x{4e00}-\x{9fff}]+))");
            QRegularExpressionMatch toM = toRe.match(rest);
            if (toM.hasMatch()) {
                p.to = toM.captured(1);
            } else {
                // 纯空格分隔：1kg 斤 → 取 rest 首个单位词
                QRegularExpression spRe(R"(([a-z/\x{4e00}-\x{9fff}]+))");
                QRegularExpressionMatch spM = spRe.match(rest);
                if (spM.hasMatch() && containsUnitWord(spM.captured(1)))
                    p.to = spM.captured(1);
            }
            return p;
        }
    }

    // 4) 时间：含城市/时区关键词
    if (containsTimeKeyword(s)) {
        p.type = Type::Time;
        p.city = s;
        return p;
    }

    // 5) 颜色转换：#rgb / #rrggbb / rgb() / hsl() / 色名 / 随机
    {
        QString color;
        if (matchColor(s, color)) {
            p.type = Type::Color;
            p.text = color;
            return p;
        }
        // 色名/随机色：红/蓝/绿色 或 随机
        static const QStringList kColorNames = {
            "红", "橙", "黄", "绿", "青", "蓝", "紫", "黑", "白", "灰",
            "粉", "棕", "red", "orange", "yellow", "green", "cyan", "blue",
            "purple", "black", "white", "gray", "pink", "brown", "随机", "random",
        };
        for (const QString &n : kColorNames) {
            if (s.contains(n)) {
                p.type = Type::Color;
                p.text = p.raw;
                return p;
            }
        }
    }

    // 6) 程序员工具：含 base64/ascii/url/hash/时间戳 等关键词
    // 保留原始大小写（ascii A 与 a 结果不同）
    // 纯数字 10 位（秒级）/13 位（毫秒级）时间戳也归此分支（见 programprovider）
    if ((containsProgramKeyword(s) || QRegularExpression(R"(^\d{10}$|^\d{13}$)").match(s).hasMatch())
        && !containsTimeKeyword(s)) {
        p.type = Type::Program;
        p.text = p.raw;
        return p;
    }

    // 7) 日期/倒数日：提取到日期即命中（无需关键词，避免与计算器/单位冲突）
    {
        // 复用 DateTimeProvider 的日期抽取逻辑过于重，这里做轻量判定：
        // 含 YYYY-MM-DD 等日期形态、中文年月日（含时分秒）或倒数日/距/date 等关键词
        QRegularExpression dateRe(R"(\d{4}[-./]\d{1,2}[-./]\d{1,2})");
        QRegularExpression cnDateRe(R"(\d{4}\s*年\s*\d{1,2}\s*月\s*\d{1,2})");
        if (dateRe.match(s).hasMatch() || cnDateRe.match(s).hasMatch()
            || containsDateKeyword(s)) {
            p.type = Type::DateTime;
            p.text = p.raw;
            return p;
        }
    }

    // 8) 科学/程序员计算器：算术表达式或进制转换
    {
        QString expr;
        if (matchCalc(s, expr)) {
            p.type = Type::Calc;
            p.text = expr;
            return p;
        }
    }

    p.type = Type::None;
    return p;
}

} // namespace QueryParser
