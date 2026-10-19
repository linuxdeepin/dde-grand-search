// SPDX-FileCopyrightText: 2026 testerxydw
// SPDX-License-Identifier: GPL-3.0-or-later

#include "programprovider.h"
#include "i18n.h"

#include <QRegularExpression>
#include <QByteArray>
#include <QCryptographicHash>
#include <cctype>
#include <QDateTime>

namespace {

// 提取「动作 + 内容」：如 base64 decode xxx / md5 xxx / url encode xxx
struct Req {
    QString action;  // encode/decode/hash/ascii/time
    QString algo;    // base64/url/md5/sha1/sha256
    QString content; // 待处理文本
    bool hashAll = false; // 裸 hash/哈希（未指定算法）→ 同时给 MD5/SHA1/SHA256
};

Req parseReq(const QString &s)
{
    Req r;
    QString lower = s.toLower().remove(' ');
    // 时间戳（当前）
    if (lower.contains("时间戳") || lower.contains("timestamp")) {
        r.action = "time";
        return r;
    }
    // 数字时间戳：纯数字 10 位（秒级）或 13 位（毫秒级）
    // 需排在日期/计算器之前，避免被误判；含短横线/运算符的不会命中
    if (QRegularExpression(R"(^\d{10}$)").match(lower).hasMatch()) {
        r.action = "time";
        r.content = lower;   // 秒级时间戳
        return r;
    }
    if (QRegularExpression(R"(^\d{13}$)").match(lower).hasMatch()) {
        r.action = "time";
        r.content = lower;   // 毫秒级时间戳
        return r;
    }
    // base64 / url 编解码：提取 algo，剥离其后动词，剩余为内容
    QString rest = s.trimmed();
    QString algo;
    if (lower.contains("base64")) algo = "base64";
    else if (lower.contains("url")) algo = "url";
    if (!algo.isEmpty()) {
        // 去掉 algo 标识词，再去掉动词词，得到纯内容
        QString content = rest;
        content.remove(QRegularExpression("(?i)base64|url"));
        content.remove(QRegularExpression("(?i)encode|decode|编码|解码"));
        content = content.trimmed();
        r.algo = algo;
        r.action = (lower.contains("decode") || lower.contains("解码")) ? "decode" : "encode";
        r.content = content;
        return r;
    }
    // 哈希：md5 / sha1 / sha256 / sha + 内容；裸 hash/哈希 + 内容 → 三种全给
    // 显式算法词：md5 / sha1 / sha256 / sha（sha 视作 sha1）；(?i) 支持大写命令（捕获内容保留大小写）
    QRegularExpression hashSpecRe(R"((?i)(md5|sha1|sha256)\s+(.+))");
    QRegularExpressionMatch hsm = hashSpecRe.match(s.trimmed());
    if (hsm.hasMatch()) {
        r.action = "hash";
        r.algo = hsm.captured(1).toLower();
        r.content = hsm.captured(2).trimmed();
        return r;
    }
    // 裸 hash/哈希 + 内容 → 全给三种
    QRegularExpression hashAllRe(R"((?i)(?:hash|哈希)\s+(.+))");
    QRegularExpressionMatch ham = hashAllRe.match(s.trimmed());
    if (ham.hasMatch()) {
        r.action = "hash";
        r.hashAll = true;
        r.content = ham.captured(1).trimmed();
        return r;
    }
    // ascii：单字符或字符 + ascii 关键词（(?i) 支持大写 ASCII 命令）
    QRegularExpression asciiRe(R"((?i)ascii\s*(.+))");
    QRegularExpressionMatch am = asciiRe.match(s.trimmed());
    if (am.hasMatch()) {
        r.action = "ascii";
        r.content = am.captured(1).trimmed();
        return r;
    }
    return r;
}

QString hashOf(const QString &algo, const QByteArray &data)
{
    if (algo == "sha256")
        return QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();
    if (algo == "sha1")
        return QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex();
    return QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
}

} // namespace

QList<ResultBuilder::Item> ProgramProvider::run(const QString &text)
{
    QList<ResultBuilder::Item> items;
    Req r = parseReq(text);

    if (r.action == "time") {
        // 数字时间戳：反查对应日期（支持 10 位秒级 / 13 位毫秒级）
        if (!r.content.isEmpty()) {
            bool ok = false;
            qint64 raw = r.content.toLongLong(&ok);
            if (ok) {
                qint64 secs = (r.content.size() == 13) ? raw / 1000 : raw;
                QDateTime dt = QDateTime::fromSecsSinceEpoch(secs);
                if (dt.isValid()) {
                    QString local = dt.toString("yyyy-MM-dd HH:mm:ss");
                    QString utc = dt.toUTC().toString("yyyy-MM-dd HH:mm:ss");
                    ResultBuilder::Item it1, it2;
                    it1.key = "prog-ts-date";
                    it1.name = QString("%1 → 本地 %2 / UTC %3")
                                  .arg(r.content).arg(local).arg(utc);
                    it1.icon = "utilities-terminal"; it1.type = "convert/prog-time";
                    it2.key = "prog-ts-date-sec";
                    it2.name = QString("对应秒级时间戳: %1").arg(secs);
                    it2.icon = "utilities-terminal"; it2.type = "convert/prog-time";
                    items.append(it1); items.append(it2);
                    return items;
                }
            }
        }
        // 当前时间戳
        qint64 secs = QDateTime::currentSecsSinceEpoch();
        qint64 ms = QDateTime::currentMSecsSinceEpoch();
        ResultBuilder::Item it1, it2;
        it1.key = "prog-ts-sec"; it1.name = QString("Unix 秒: %1").arg(secs);
        it1.icon = "utilities-terminal"; it1.type = "convert/prog-time";
        it2.key = "prog-ts-ms"; it2.name = QString("毫秒: %1").arg(ms);
        it2.icon = "utilities-terminal"; it2.type = "convert/prog-time";
        items.append(it1); items.append(it2);
        return items;
    }

    if (r.action == "ascii") {
        // 数字 → 码值查字符
        bool isNum = false;
        int code = r.content.toInt(&isNum);
        if (isNum && code >= 0 && code <= 127) {
            ResultBuilder::Item it;
            it.key = "prog-ascii";
            it.name = QString("ASCII %1 = '%2'").arg(code).arg(QChar(code));
            it.icon = "utilities-terminal"; it.type = "convert/prog-ascii";
            items.append(it);
        } else if (!r.content.isEmpty()) {
            if (r.content.size() == 1) {
                // 单字符 → 码值（非 Latin1 字符如实给出 Unicode 码点，不假报 ASCII 码）
                QChar ch = r.content[0];
                ResultBuilder::Item it;
                it.key = "prog-ascii";
                if (ch.unicode() <= 0xFF) {
                    it.name = QString("'%1' = ASCII %2 (0x%3)")
                        .arg(ch).arg((int)ch.toLatin1()).arg((int)ch.toLatin1(), 0, 16);
                } else {
                    // U+ 码点十六进制部分规范大写，不整串 toUpper（避免改动文案）
                    it.name = QString("'%1' 非 ASCII 字符 (Unicode U+%2)")
                        .arg(ch).arg(QString("%1").arg(ch.unicode(), 4, 16, QChar('0')).toUpper());
                }
                it.icon = "utilities-terminal"; it.type = "convert/prog-ascii";
                items.append(it);
            } else {
                // 多字符 → 逐字符紧凑映射（易用性；非 Latin1 字符标 Unicode 码点）
                QStringList maps;
                for (QChar ch : r.content) {
                    if (ch.unicode() <= 0xFF)
                        maps << QString("'%1'=%2").arg(ch).arg((int)ch.toLatin1());
                    else
                        maps << QString("'%1'=U+%2").arg(ch).arg(QString("%1").arg(ch.unicode(), 4, 16, QChar('0')).toUpper());
                }
                ResultBuilder::Item it;
                it.key = "prog-ascii-multi";
                it.name = QString("ASCII: %1").arg(maps.join("  "));
                it.icon = "utilities-terminal"; it.type = "convert/prog-ascii";
                items.append(it);
            }
        }
        return items;
    }

    if (r.action == "hash") {
        QByteArray data = r.content.toUtf8();
        if (r.hashAll) {
            // 裸 hash/哈希 → MD5/SHA1/SHA256 全给（不舍上限）
            for (const QString &algo : {"md5", "sha1", "sha256"}) {
                ResultBuilder::Item it;
                it.key = QString("prog-hash-%1").arg(algo);
                it.name = QString("%1: %2").arg(algo.toUpper()).arg(hashOf(algo, data));
                it.icon = "utilities-terminal"; it.type = "convert/prog-hash";
                items.append(it);
            }
        } else {
            ResultBuilder::Item it;
            it.key = "prog-hash";
            it.name = QString("%1: %2").arg(r.algo.toUpper()).arg(hashOf(r.algo, data));
            it.icon = "utilities-terminal"; it.type = "convert/prog-hash";
            items.append(it);
        }
        return items;
    }

    if (r.action == "encode" || r.action == "decode") {
        QByteArray data = r.content.toUtf8();
        QString result;
        if (r.algo == "base64") {
            result = (r.action == "encode")
                ? data.toBase64()
                : QByteArray::fromBase64(data);
        } else { // url
            if (r.action == "encode") {
                QString e;
                const QByteArray raw = data;
                for (char c : raw) {
                    uchar u = (uchar)c;
                    if (std::isalnum(u) || u == '-' || u == '_' || u == '.' || u == '~')
                        e += QChar(u);
                    else
                        e += QString("%%1").arg(u, 2, 16, QChar('0')).toUpper();
                }
                result = e;
            } else {
                QString in = r.content;
                QByteArray out;
                for (int i = 0; i < in.size(); ++i) {
                    // 非 Latin1 字符（如中文）按 UTF-8 原样保留，不可 toLatin1() 截断损坏
                    if (in[i].unicode() > 0xFF) {
                        out += QString(in[i]).toUtf8();
                        continue;
                    }
                    if (in[i] == '%' && i + 2 < in.size()) {
                        bool ok = false;
                        int v = in.mid(i + 1, 2).toInt(&ok, 16);
                        if (ok) { out.append((char)v); i += 2; continue; }
                    }
                    out.append(in[i].toLatin1());
                }
                result = QString::fromUtf8(out);
            }
        }
        QString verb = I18n::isChinese()
            ? (r.action == "encode" ? "编码" : "解码")
            : (r.action == "encode" ? "encode" : "decode");
        ResultBuilder::Item it;
        it.key = QString("prog-%1-%2").arg(r.algo).arg(r.action);
        it.name = QString("%1 %2: %3").arg(r.algo.toUpper()).arg(verb).arg(result);
        it.icon = "utilities-terminal";
        it.type = QString("convert/prog-%1").arg(r.algo);
        items.append(it);

        // 编码结果附解码验证行（base64/url 均可逆，便于核对）
        if (r.action == "encode") {
            QString verify;
            if (r.algo == "base64")
                verify = QString::fromUtf8(QByteArray::fromBase64(result.toUtf8()));
            else {
                QString in = result;
                QByteArray out;
                for (int i = 0; i < in.size(); ++i) {
                    // 非 Latin1 字符（如中文）按 UTF-8 原样保留，不可 toLatin1() 截断损坏
                    if (in[i].unicode() > 0xFF) {
                        out += QString(in[i]).toUtf8();
                        continue;
                    }
                    if (in[i] == '%' && i + 2 < in.size()) {
                        bool ok = false;
                        int v = in.mid(i + 1, 2).toInt(&ok, 16);
                        if (ok) { out.append((char)v); i += 2; continue; }
                    }
                    out.append(in[i].toLatin1());
                }
                verify = QString::fromUtf8(out);
            }
            ResultBuilder::Item vit;
            vit.key = QString("prog-%1-verify").arg(r.algo);
            vit.name = QString("解码验证: %1").arg(verify);
            vit.icon = "utilities-terminal"; vit.type = QString("convert/prog-%1").arg(r.algo);
            items.append(vit);
        }
        return items;
    }

    return items;
}
