// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#include "calcprovider.h"
#include "i18n.h"

#include <QRegularExpression>
#include <QLocale>
#include <cmath>

namespace {

static QString baseLabel(const QString &key)
{
    return I18n::miscName(key);
}



// 解析单个 token：数字（支持 0x/0b/0o 前缀）、标识符（函数名）、运算符。
struct Tok {
    enum Kind { Num, Id, Op, LParen, RParen, End } kind;
    double num = 0.0;
    QString id;
    QChar op;
};

// 简单词法分析：把所有空白去掉后的字符流切分为 token。
QList<Tok> tokenize(const QString &s, bool &ok)
{
    QList<Tok> toks;
    ok = true;
    QString src = s;
    src.remove(' ');
    int i = 0;
    while (i < src.size()) {
        QChar c = src[i];
        if (c == '(') { Tok t; t.kind = Tok::LParen; toks.append(t); ++i; continue; }
        if (c == ')') { Tok t; t.kind = Tok::RParen; toks.append(t); ++i; continue; }
        if (QString("+-*/%^&|~").contains(c)) { toks.append({Tok::Op, 0, {}, c}); ++i; continue; }
        if (c.isDigit()) {
            // 数字（含进制前缀）
            QString num;
            if (i + 1 < src.size() && (c == '0') && (src[i+1] == 'x' || src[i+1] == 'X'
                || src[i+1] == 'b' || src[i+1] == 'B' || src[i+1] == 'o' || src[i+1] == 'O')) {
                num += c; num += src[i+1]; i += 2;
                while (i < src.size() && (src[i].isDigit() || src[i].isUpper() || src[i].isLower()))
                    num += src[i++];
                bool conv = true;
                int base = (num[1].toUpper() == 'X') ? 16 : (num[1].toUpper() == 'B' ? 2 : 8);
                if (base == 16)
                    toks.append({Tok::Num, (double)num.mid(2).toUInt(&conv, 16)});
                else
                    toks.append({Tok::Num, (double)num.mid(2).toUInt(&conv, base)});
                if (!conv) { ok = false; return toks; }
                continue;
            }
            while (i < src.size() && (src[i].isDigit() || src[i] == '.'))
                num += src[i++];
            toks.append({Tok::Num, num.toDouble()});
            continue;
        }
        if (c.isLetter()) {
            QString id;
            while (i < src.size() && (src[i].isLetter() || src[i].isDigit()))
                id += src[i++];
            toks.append({Tok::Id, 0, id});
            continue;
        }
        ok = false; // 非法字符
        return toks;
    }
    Tok endTok; endTok.kind = Tok::End; toks.append(endTok);
    return toks;
}

// 递归下降求值。
class Evaluator {
public:
    explicit Evaluator(const QString &expr) : m_toks(tokenize(expr, m_ok)) {}

    bool ok() const { return m_ok; }

    double run()
    {
        if (!m_ok) return 0.0;
        double v = parseExpr();
        return v;
    }

private:
    QList<Tok> m_toks;
    int m_pos = 0;
    bool m_ok = true;

    const Tok &cur() const { return m_toks[m_pos]; }
    void adv() { if (m_pos < m_toks.size() - 1) ++m_pos; }

    double parseExpr()
    {
        double v = parseTerm();
        while (cur().kind == Tok::Op && (cur().op == '+' || cur().op == '-')) {
            QChar op = cur().op; adv();
            double r = parseTerm();
            v = (op == '+') ? v + r : v - r;
        }
        return v;
    }

    double parseTerm()
    {
        double v = parseFactor();
        while (cur().kind == Tok::Op && QString("*/%&|").contains(cur().op)) {
            QChar op = cur().op; adv();
            double r = parseFactor();
            if (op == '*') v *= r;
            else if (op == '/') v /= r;
            else if (op == '%') v = std::fmod(v, r);
            else if (op == '&') v = (double)((qint64)v & (qint64)r);
            else if (op == '|') v = (double)((qint64)v | (qint64)r);
        }
        return v;
    }

    double parseFactor()
    {
        // 函数调用：id ( expr ) —— 优先于一元/常量解析
        if (cur().kind == Tok::Id && m_pos + 1 < m_toks.size()
            && m_toks[m_pos + 1].kind == Tok::LParen) {
            QString fn = cur().id; adv();
            adv(); // 跳过 '('
            double arg = parseExpr();
            if (cur().kind != Tok::RParen) { m_ok = false; return 0.0; }
            adv();
            return applyFn(fn, arg);
        }
        if (cur().kind == Tok::Op && cur().op == '~') {
            adv();
            return (double)(~(qint64)parseFactor());
        }
        if (cur().kind == Tok::Op && (cur().op == '+' || cur().op == '-')) {
            QChar op = cur().op; adv();
            double r = parseFactor();
            return (op == '-') ? -r : r;
        }
        double v = parseUnary();
        // 幂运算（右结合）
        if (cur().kind == Tok::Op && cur().op == '^') {
            adv();
            double r = parseFactor();
            v = std::pow(v, r);
        }
        return v;
    }

    double parseUnary()
    {
        if (cur().kind == Tok::Num) { double v = cur().num; adv(); return v; }
        if (cur().kind == Tok::LParen) {
            adv();
            double v = parseExpr();
            if (cur().kind == Tok::RParen) adv();
            else m_ok = false;
            return v;
        }
        if (cur().kind == Tok::Id) {
            // 常量
            QString id = cur().id.toLower();
            adv();
            if (id == "pi") return M_PI;
            if (id == "e") return M_E;
            m_ok = false;
            return 0.0;
        }
        m_ok = false;
        return 0.0;
    }

    double applyFn(const QString &fn, double arg)
    {
        QString f = fn.toLower();
        if (f == "sqrt") return std::sqrt(arg);
        if (f == "sin") return std::sin(arg);
        if (f == "cos") return std::cos(arg);
        if (f == "tan") return std::tan(arg);
        if (f == "log") return std::log10(arg);
        if (f == "ln") return std::log(arg);
        if (f == "abs") return std::fabs(arg);
        if (f == "floor") return std::floor(arg);
        if (f == "ceil") return std::ceil(arg);
        if (f == "round") return std::round(arg);
        if (f == "exp") return std::exp(arg);
        m_ok = false;
        return 0.0;
    }
};

} // namespace

bool CalcProvider::compute(const QString &expr, double &out)
{
    Evaluator ev(expr);
    if (!ev.ok())
        return false;
    double v = ev.run();
    if (!ev.ok())
        return false;
    out = v;
    return true;
}

QList<ResultBuilder::Item> CalcProvider::eval(const QString &exprRaw)
{
    QList<ResultBuilder::Item> items;
    QString s = exprRaw.toLower().remove(' ');

    // 进制转换请求：数值 + to + 目标进制
    QRegularExpression baseRe(R"((0x[0-9a-f]+|0b[01]+|0o[0-7]+|\d+(?:\.\d+)?)\s*(?:to|->|转为|转)\s*(hex|bin|oct|dec|二进制|八进制|十进制|十六进制))");
    QRegularExpressionMatch bm = baseRe.match(s);
    if (bm.hasMatch()) {
        QString valStr = bm.captured(1);
        QString target = bm.captured(2);
        bool conv = true;
        qint64 val = 0;
        if (valStr.startsWith("0x")) val = valStr.mid(2).toLongLong(&conv, 16);
        else if (valStr.startsWith("0b")) val = valStr.mid(2).toLongLong(&conv, 2);
        else if (valStr.startsWith("0o")) val = valStr.mid(2).toLongLong(&conv, 8);
        else val = valStr.toLongLong(&conv);
        if (conv) {
            QStringList outs;
            if (target.startsWith("hex") || target.contains("十六"))
                outs << baseLabel("HEX") + QString(": 0x%1").arg(val, 0, 16);
            else if (target.startsWith("bin") || target.contains("二"))
                outs << baseLabel("BIN") + QString(": 0b%1").arg(QString(val ? QString::number(val, 2) : "0"));
            else if (target.startsWith("oct") || target.contains("八"))
                outs << baseLabel("OCT") + QString(": 0o%1").arg(val, 0, 8);
            else
                outs << baseLabel("DEC") + QString(": %1").arg(val);
            for (const QString &o : outs) {
                ResultBuilder::Item it;
                it.key = QString("calc-base-%1").arg(o);
                it.name = o;
                it.icon = "accessories-calculator";
                it.type = "convert/calc-base";
                items.append(it);
            }
            return items;
        }
    }

    // 进制一次全给：数值 + 进制/base → DEC/HEX/BIN/OCT 全列（不舍上限）
    {
        QRegularExpression allBaseRe(R"((0x[0-9a-f]+|0b[01]+|0o[0-7]+|\d+(?:\.\d+)?)\s*(?:进制|base))");
        QRegularExpressionMatch ab = allBaseRe.match(s);
        if (ab.hasMatch()) {
            QString valStr = ab.captured(1);
            bool conv = true;
            qint64 val = 0;
            if (valStr.startsWith("0x")) val = valStr.mid(2).toLongLong(&conv, 16);
            else if (valStr.startsWith("0b")) val = valStr.mid(2).toLongLong(&conv, 2);
            else if (valStr.startsWith("0o")) val = valStr.mid(2).toLongLong(&conv, 8);
            else val = valStr.toLongLong(&conv);
            if (conv) {
                ResultBuilder::Item d, h, b, o;
                d.key = "calc-base-dec"; d.name = baseLabel("DEC") + QString(": %1").arg(val);
                d.icon = "accessories-calculator"; d.type = "convert/calc-base";
                h.key = "calc-base-hex"; h.name = baseLabel("HEX") + QString(": 0x%1").arg(val, 0, 16);
                h.icon = "accessories-calculator"; h.type = "convert/calc-base";
                b.key = "calc-base-bin"; b.name = baseLabel("BIN") + QString(": 0b%1").arg(QString(val ? QString::number(val, 2) : "0"));
                b.icon = "accessories-calculator"; b.type = "convert/calc-base";
                o.key = "calc-base-oct"; o.name = baseLabel("OCT") + QString(": 0o%1").arg(val, 0, 8);
                o.icon = "accessories-calculator"; o.type = "convert/calc-base";
                items << d << h << b << o;
                return items;
            }
        }
    }

    // 普通算术/函数求值
    double out = 0.0;
    if (compute(s, out)) {
        // 除零/域错误等产生非有限结果 → 友好错误卡
        if (!std::isfinite(out)) {
            ResultBuilder::Item err;
            err.key = "calc-error";
            err.icon = "accessories-calculator";
            err.type = "convert/calc-error";
            if (s.contains('/')) {
                err.name = "无法计算：除数不能为 0";
                if (!I18n::isChinese()) err.name = "Cannot compute: division by zero";
            } else {
                err.name = "无法计算：结果超出范围";
                if (!I18n::isChinese()) err.name = "Cannot compute: result out of range";
            }
            items.append(err);
            return items;
        }
        bool isInt = std::fabs(out - std::round(out)) < 1e-9;
        qint64 iv = (qint64)std::round(out);
        // 主结果
        ResultBuilder::Item main;
        main.key = "calc-result";
        main.name = isInt
            ? QString("%1 = %2").arg(exprRaw).arg(QLocale().toString(iv))
            : QString("%1 = %2").arg(exprRaw).arg(QLocale().toString(out, 'g', 12));
        main.icon = "accessories-calculator";
        main.type = "convert/calc";
        items.append(main);

        // 程序员友好：同时给出不同进制（仅整数结果）
        if (isInt) {
            ResultBuilder::Item h, b, o;
            h.key = "calc-hex"; h.name = baseLabel("HEX") + QString(": 0x%1").arg(iv, 0, 16);
            h.icon = "accessories-calculator"; h.type = "convert/calc-hex";
            b.key = "calc-bin"; b.name = baseLabel("BIN") + QString(": 0b%1").arg(QString::number(iv ? iv : 0, 2));
            b.icon = "accessories-calculator"; b.type = "convert/calc-bin";
            o.key = "calc-oct"; o.name = baseLabel("OCT") + QString(": 0o%1").arg(iv, 0, 8);
            o.icon = "accessories-calculator"; o.type = "convert/calc-oct";
            items.append(h); items.append(b); items.append(o);
        } else {
            // 小数增强：附科学计数法 + 百分比（易用性）
            ResultBuilder::Item sci, pct;
            sci.key = "calc-sci"; sci.name = QString("科学计数: %1").arg(QLocale().toString(out, 'g', 12).replace('e', "e"));
            sci.icon = "accessories-calculator"; sci.type = "convert/calc-sci";
            pct.key = "calc-pct"; pct.name = QString("百分比: %1%").arg(ResultBuilder::formatNumber(out * 100.0, 2));
            pct.icon = "accessories-calculator"; pct.type = "convert/calc-pct";
            items << sci << pct;
        }
        return items;
    }

    // 求值失败 → 友好提示卡（而非静默），区分常见错误
    ResultBuilder::Item err;
    err.key = "calc-error";
    err.icon = "accessories-calculator";
    err.type = "convert/calc-error";
    if (s.contains('/')) {
        err.name = "无法计算：除数不能为 0";
        if (!I18n::isChinese()) err.name = "Cannot compute: division by zero";
    } else if (s.contains("sqrt")) {
        err.name = "无法计算：负数没有实数平方根";
        if (!I18n::isChinese()) err.name = "Cannot compute: sqrt of a negative number";
    } else {
        err.name = QString("不支持的表达式：%1（可用函数 sqrt/sin/cos/log/ln/abs 等，输入 help 查看）").arg(exprRaw);
        if (!I18n::isChinese())
            err.name = QString("Unsupported expression: %1 (functions: sqrt/sin/cos/log/ln/abs; type help)").arg(exprRaw);
    }
    items.append(err);
    return items;
}
