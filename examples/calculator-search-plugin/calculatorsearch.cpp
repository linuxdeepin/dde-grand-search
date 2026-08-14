// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "calculatorsearch.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QClipboard>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QRegularExpression>

#include "exprtk.hpp"

Q_LOGGING_CATEGORY(logCalcPlugin, "calculator.search.plugin")

CalculatorSearch::CalculatorSearch(QObject *parent)
    : QObject(parent)
{
    qCInfo(logCalcPlugin) << "Calculator search plugin initialized";
}

CalculatorSearch::~CalculatorSearch()
{
}

QString CalculatorSearch::search(const QString &json)
{
    SearchInput input = parseSearchInput(json);
    if (input.mID.isEmpty()) {
        qCWarning(logCalcPlugin) << "Invalid search input: missing mID";
        return buildEmptyResult(input.mID);
    }

    qCDebug(logCalcPlugin) << "Search request - mID:" << input.mID << "content:" << input.cont;

    if (input.cont.isEmpty()) {
        return buildEmptyResult(input.mID);
    }

    if (!isMathExpression(input.cont)) {
        qCDebug(logCalcPlugin) << "Not a math expression:" << input.cont;
        return buildEmptyResult(input.mID);
    }

    double result = 0.0;
    if (!tryEvaluate(input.cont, result)) {
        qCDebug(logCalcPlugin) << "Failed to evaluate:" << input.cont;
        return buildEmptyResult(input.mID);
    }

    QString formatted = formatResult(result);
    qCInfo(logCalcPlugin) << "Expression:" << input.cont << "=" << formatted;

    {
        QMutexLocker locker(&m_mutex);
        m_lastResults.insert(input.mID, {input.cont, result});
    }

    return buildSearchResult(input.mID, input.cont, result);
}

bool CalculatorSearch::stop(const QString &json)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(logCalcPlugin) << "Stop: JSON parse error:" << error.errorString();
        return false;
    }

    QJsonObject root = doc.object();
    QString mID = root.value("mID").toString();

    QMutexLocker locker(&m_mutex);
    m_lastResults.remove(mID);

    qCDebug(logCalcPlugin) << "Stop search task:" << mID;
    return true;
}

bool CalculatorSearch::action(const QString &json)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(logCalcPlugin) << "Action: JSON parse error:" << error.errorString();
        return false;
    }

    QJsonObject root = doc.object();
    QString action = root.value("action").toString();
    QString item = root.value("item").toString();

    qCDebug(logCalcPlugin) << "Action:" << action << "item:" << item;

    if (action == "openitem") {
        QMutexLocker locker(&m_mutex);
        for (const auto &result : m_lastResults) {
            QString resultStr = formatResult(result.result);
            QString itemKey = "calc-" + result.expression;
            if (item == itemKey) {
                locker.unlock();

                QClipboard *clipboard = QGuiApplication::clipboard();
                clipboard->setText(resultStr);

                qCInfo(logCalcPlugin) << "Copied to clipboard:" << resultStr;
                return true;
            }
        }
    }

    return false;
}

CalculatorSearch::SearchInput CalculatorSearch::parseSearchInput(const QString &json) const
{
    SearchInput input;
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(logCalcPlugin) << "JSON parse error:" << error.errorString();
        return input;
    }

    QJsonObject root = doc.object();
    input.ver = root.value("ver").toString();
    input.mID = root.value("mID").toString();
    input.cont = root.value("cont").toString().trimmed();
    return input;
}

QString CalculatorSearch::buildSearchResult(const QString &mID, const QString &expression, double result) const
{
    QString formatted = formatResult(result);
    QString displayText = expression + " = " + formatted;
    QString itemKey = "calc-" + expression;

    QJsonObject root;
    root["ver"] = "1.0";
    root["mID"] = mID;

    QJsonArray contents;
    QJsonObject group;
    group["group"] = QString::fromUtf8("计算器");

    QJsonArray items;
    QJsonObject item;
    item["item"] = itemKey;
    item["name"] = displayText;
    item["icon"] = "accessories-calculator";
    item["type"] = "calculator/result";
    items.append(item);

    group["items"] = items;
    contents.append(group);
    root["cont"] = contents;

    QJsonDocument doc;
    doc.setObject(root);
    return doc.toJson(QJsonDocument::Compact);
}

QString CalculatorSearch::buildEmptyResult(const QString &mID) const
{
    QJsonObject root;
    root["ver"] = "1.0";
    root["mID"] = mID;
    root["cont"] = QJsonArray();

    QJsonDocument doc;
    doc.setObject(root);
    return doc.toJson(QJsonDocument::Compact);
}

bool CalculatorSearch::isMathExpression(const QString &text) const
{
    if (text.isEmpty())
        return false;

    static const QRegularExpression validChars(
        QStringLiteral("^[0-9+\\-*/().\\s^%!,a-zA-Z]+$"));
    if (!validChars.match(text).hasMatch())
        return false;

    static const QRegularExpression hasDigit(QStringLiteral("\\d"));
    if (!hasDigit.match(text).hasMatch())
        return false;

    static const QRegularExpression hasOperator(QStringLiteral("[+\\-*/^%!]"));
    static const QRegularExpression hasFunctionCall(QStringLiteral("[a-zA-Z]\\s*\\("));
    if (!hasOperator.match(text).hasMatch() && !hasFunctionCall.match(text).hasMatch())
        return false;

    return true;
}

bool CalculatorSearch::tryEvaluate(const QString &expression, double &result) const
{
    typedef exprtk::symbol_table<double> symbol_table_t;
    typedef exprtk::expression<double> expression_t;
    typedef exprtk::parser<double> parser_t;

    symbol_table_t symbol_table;
    symbol_table.add_constants();

    expression_t expr;
    expr.register_symbol_table(symbol_table);

    parser_t parser;
    if (!parser.compile(expression.toStdString(), expr)) {
        qCDebug(logCalcPlugin) << "Parse error:" << parser.error().c_str();
        return false;
    }

    result = expr.value();
    return std::isfinite(result);
}

QString CalculatorSearch::formatResult(double value) const
{
    if (value == static_cast<long long>(value) &&
        std::abs(value) < 1e15) {
        return QString::number(static_cast<long long>(value));
    }

    QString str = QString::number(value, 'g', 12);

    if (str.contains('e') || str.contains('E')) {
        str = QString::number(value, 'f', 6);
        while (str.endsWith('0') && str.contains('.'))
            str.chop(1);
        if (str.endsWith('.'))
            str.chop(1);
    }

    return str;
}
